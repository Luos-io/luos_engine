/******************************************************************************
 * @file transmission
 * @brief transmission management.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

/******************************* Description of the TX process ************************************************************************
 *
 *                                           |                             Luos_send
 *                                           |                                 |
 *                                           |                                 |
 *                       TX_status=NOK  +----v-----+  TX_status=DISABLE  +-----v-----+
 *                           +----------|  Timeout |-----------+         |    Msg    |         +-----------+     +-----------+
 *                           |          +----------+           |         |Preparation|         | Collision | x4  |  Prepare  |      ^
 *                           |               |TX_status=OK     |         +-----------+         | Reception |---->|  ACK RX   |------+
 *                           |               |                 |               |               +-----------+     +-----------+
 *      +-----------+   +----v-----+    +----v-----+    +------v----+    +-----v-----+               |
 * ^    | TX_status |   |  Delay   |    |   rm     |    | TX_status |    |    Tx     |     ^         |Collision
 * +----| =DISABLE  |<--| compute  |    | TX task  |    | =DISABLE  |    |Allocation |-----+         |
 *      +-----------+   +----------+    +----------+    +-----------+    +-----------+tx_lock  +-----v-----+
 *                           |               |             |                   |               |  Stop TX  |
 *                           |               |             |         +---------+-------+       +-----------+
 *                           |retry=10       +-------------+-------->|Transmit |       |             |
 *                           |               |                       |Process  |       |       +-----v-----+
 *                      +----v-----+         |                       |   +-----v-----+ |       | Pass data |
 *                      | Exclude  |         |                       |   |    Get    | |   ^   |   to RX   |
 *                      | service  |---------+                       |   | TX_tasks  |-+---+   +-----------+
 *                      +----------+                                 |   +-----------+ No task       |
 *                                                                   |         |       |       +-----v-----+
 *                                                                   |   +-----v-----+ |       | TX_status |      ^
 *                                                                   |   |Send + set | |       |   =NOK    |------+
 *                                                                   |   | TX_status | |       +-----------+
 *                                                                   |   +-----------+ |
 *                                                                   |         |       |
 *                                      +----------+                 |   +-----v-----+ |
 *                      +----------+    |TX_status |     ^           |   |   Enable  | |   ^
 *                  --->|   RX[1]  |--->| =DISABLE |-----+           |   | Collision |-+---+
 *                      +----------+    +----------+                 |   +-----------+ |
 *                                                                   +-----------------+
 *
 ************************************************************************************************************************************/

#include <string.h>
#include <stdbool.h>
#include "transmission.h"
#include "robus_hal.h"
#include "context.h"
#include "reception.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t nbrRetry = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
_CRITICAL static uint8_t Transmit_GetLockStatus(void);
_CRITICAL static bool Transmit_SnapJob(phy_job_t *job, phy_job_t *snapshot);
/******************************************************************************
 * @brief Transmit_Init
 * @param None
 * @return None
 ******************************************************************************/
void Transmit_Init(void)
{
    // no transmission lock
    ctx.tx.lock = false;
    // Init collision state
    ctx.tx.collision = false;
    // Init Tx status
    ctx.tx.status = TX_DISABLE;
    // Init the transmission retry counter
    nbrRetry = 0;
}
/******************************************************************************
 * @brief Transmit an ACK
 * @param None
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Transmit_SendAck(void)
{
    // Info : We don't consider this transmission as a complete message transmission but as a complete message reception.
    RobusHAL_SetRxState(false);
    // Transmit Ack data
    RobusHAL_ComTransmit((unsigned char *)&ctx.rx.status.unmap, 1);
    // Reset Ack status
    ctx.rx.status.unmap = 0x0F;
}

/******************************************************************************
 * @brief crc computation
 * @param data
 * @param size of data
 * @param crc initialization value
 * @return crc
 ******************************************************************************/
uint16_t ll_crc_compute(const uint8_t *data, uint16_t size, uint16_t crc_seed)
{
    uint16_t crc_val = crc_seed;
    for (uint16_t i = 0; i < size; i++)
    {
        uint16_t dbyte = data[i];
        crc_val ^= dbyte << 8;
        for (uint8_t j = 0; j < 8; ++j)
        {
            uint16_t mix = crc_val & 0x8000;
            crc_val      = (crc_val << 1);
            if (mix)
                crc_val = crc_val ^ 0x0007;
        }
    }
    return crc_val;
}

/******************************************************************************
 * @brief transmission process
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void Transmit_Process()
{
    static uint16_t crc_val = 0;
    luos_phy_t *robus_phy   = Robus_GetPhy();
    phy_job_t *job          = Phy_GetJob(robus_phy);
    static uint8_t tx_data[sizeof(msg_t) + sizeof(robus_encaps_t)];
    // Get the message encapsulation
    if ((job != NULL) && (Transmit_GetLockStatus() == false))
    {
        // Work from a snapshot, never from *job. Phy_RmJob memsets a job to 0 and
        // reaches us from the USART and timeout interrupts through Transmit_End,
        // so a job can be torn down between two reads of the same field.
        phy_job_t tx_job;
        if (Transmit_SnapJob(job, &tx_job) == false)
        {
            // The job was removed while we were reaching for it. That is a normal
            // race, not a broken invariant, so there is nothing to send and
            // nothing to assert about.
            return;
        }
        robus_encaps_t *jobEncaps = (robus_encaps_t *)tx_job.phy_data;
        // We have something to send
        // Check if we already try to send it multiple times and save it on stats if it is
        if (nbrRetry >= NBR_RETRY)
        {
            // We failed to transmit this message. We can't allow it, there is an issue on this target.
            Phy_FailedJob(robus_phy, job);
            nbrRetry         = 0;
            ctx.tx.collision = false;
            // Try to get a new job
            job = Phy_GetJob(robus_phy);
            if (job == NULL)
            {
                // Nothing to transmit anymore, just exit.
                return;
            }
            // Snapshot the new job too: it carries its own data, size and
            // encapsulation. Keeping the failed job's encapsulation here sent the
            // new message out under the old message's CRC.
            if (Transmit_SnapJob(job, &tx_job) == false)
            {
                return;
            }
            jobEncaps = (robus_encaps_t *)tx_job.phy_data;
        }
        // Check if we will need an ACK for this message and compute the transmit status we will need to manage
        transmitStatus_t initial_transmit_status = TX_OK;
        if (tx_job.ack == true)
        {
            // We will need to validate the good reception with a ack.
            // Switch the tx status as TX_NOK allowing to detect a default at the next Timeout if no ACK have been received.
            initial_transmit_status = TX_NOK;
        }
        // Now we can try to send something
        if (Transmit_GetLockStatus() == false)
        {
            // We are free to transmit
            // Lock the bus
            ctx.tx.lock = true;
            RobusHAL_SetRxDetecPin(false);
            // Switch reception in collision detection mode
            Phy_SetIrqState(false);
            ctx.rx.callback = Recep_GetCollision;
            Phy_SetIrqState(true);
            ctx.tx.data = tx_data;

            if (!nbrRetry)
            {
                // This is the first time we try to send this message, we need to backup the original crc value and the job data to the TX_data buffer
                crc_val = jobEncaps->crc;
                // tx_data holds one message plus one encapsulation and no more.
                // State that bound here instead of trusting the encapsulation: it
                // is the only thing standing between a bad size and the rest of
                // the .bss section.
                LUOS_ASSERT((tx_job.size + jobEncaps->size) <= sizeof(tx_data));
                memcpy(tx_data, tx_job.data_pt, tx_job.size);
                // Add the end of the message in the end of the buffer
                memcpy(&tx_data[tx_job.size], jobEncaps->unmaped, jobEncaps->size);
            }

            // Put timestamping on data here
            if (tx_job.timestamp)
            {

                // Convert date to a sendable timestamp and put it on the encapsulation
                jobEncaps->timestamp = Phy_ComputeMsgTimestamp(robus_phy, &tx_job);

                jobEncaps->timestamped_crc = ll_crc_compute(jobEncaps->unmaped, sizeof(time_luos_t), crc_val);
                jobEncaps->size            = sizeof(time_luos_t) + CRC_SIZE;
                // Add the end of the message in the end of the buffer
                memcpy(&tx_data[tx_job.size], jobEncaps->unmaped, jobEncaps->size);
            }

            // Transmit data
            if (Phy_GetJob(robus_phy) == job)
            {
                LUOS_ASSERT((tx_job.size + jobEncaps->size) >= 9);
                Phy_SetIrqState(false);
                // We will prepare to transmit something enable tx status with precomputed value of the initial_transmit_status
                ctx.tx.status = initial_transmit_status;
                // We still have something to send, no reset occured
                RobusHAL_ComTransmit(tx_data, (tx_job.size + jobEncaps->size));
                Phy_SetIrqState(true);
            }
            else
            {
                nbrRetry = 0;
            }
        }
    }
}

/******************************************************************************
 * @brief Copy a job out of the phy queue so a transmission can rely on it
 * @param job the job to copy, as returned by Phy_GetJob
 * @param snapshot where to store the copy
 * @return true if the copy describes a job we can still transmit
 * _CRITICAL function call in IRQ
 *
 * struct_phy.h guarantees that a job never *moves* while a phy is sending it, so
 * that phys may hold the pointer. It does not guarantee that the job still holds
 * anything: Phy_RmJob memsets it to 0, and it is reached from the USART and
 * timeout interrupts through Transmit_End. A transmission that reads job->size or
 * job->phy_data more than once can therefore see a valid value and then a zeroed
 * one, having already decided the first was good.
 *
 * That is not theoretical. Checking job->phy_data and then re-reading it one line
 * later was enough to hand the transmission a NULL encapsulation, whose ->size
 * read at NULL + 10 does not even fault on targets that alias address 0 to flash:
 * it returns whatever the vector table holds there. On an STM32G4 that is 0x0801,
 * so Transmit_Process copied 2049 bytes over a 147 byte buffer and flattened
 * 1.9 kB of .bss -- taking uwTick and the whole phy context with it, which then
 * crashed somewhere else entirely, through a function pointer that had been
 * overwritten with flash contents.
 *
 * So take one atomic copy, validate that, and never look at *job again.
 ******************************************************************************/
_CRITICAL static bool Transmit_SnapJob(phy_job_t *job, phy_job_t *snapshot)
{
    LUOS_ASSERT((job != NULL) && (snapshot != NULL));
    Phy_SetIrqState(false);
    *snapshot = *job;
    Phy_SetIrqState(true);
    return (snapshot->phy_data != NULL)
           && (snapshot->data_pt != NULL)
           && (snapshot->size != 0)
           && (snapshot->size <= sizeof(msg_t));
}

/******************************************************************************
 * @brief Send ID to others service on network
 * @param None
 * @return lock status
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL static uint8_t Transmit_GetLockStatus(void)
{
    if (ctx.tx.lock != true)
    {
        ctx.tx.lock |= RobusHAL_GetTxLockState();
    }
    return ctx.tx.lock;
}

/******************************************************************************
 * @brief finish transmit and try to launch a new one
 * @param None
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Transmit_End(void)
{
    if (ctx.tx.status == TX_OK)
    {
        // A job have been sucessfully transmitted
        nbrRetry         = 0;
        ctx.tx.collision = false;
        ctx.tx.status    = TX_DISABLE;
        // Remove the job
        luos_phy_t *robus_phy = Robus_GetPhy();
        phy_job_t *job        = Phy_GetJob(robus_phy);
        // We may had a reset during this transmission, so we need to check if we still have something to transmit
        if (Phy_GetJobNumber(robus_phy) > 0)
        {
            job->phy_data = 0;
            Phy_RmJob(robus_phy, job);
        }
    }
    else if (ctx.tx.status == TX_NOK)
    {
        // A tx_task failed
        nbrRetry++;
        // compute a delay before retry
        RobusHAL_ResetTimeout(20 * nbrRetry * (Phy_GetNodeId() + 1));
        // Lock the trasmission to be sure no one can send something from this node until next timeout.
        ctx.tx.lock   = true;
        ctx.tx.status = TX_DISABLE;
        return;
    }
    ctx.tx.lock = false;
    // Try to send something if we need to.
    Transmit_Process();
}
