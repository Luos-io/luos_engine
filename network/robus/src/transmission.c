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
    if ((job != NULL) && (Transmit_GetLockStatus() == false) && (job->phy_data != NULL))
    {
        LUOS_ASSERT((job->phy_data != NULL) && (job->size != 0) && (job->size < sizeof(msg_t)));
        robus_encaps_t *jobEncaps = (robus_encaps_t *)job->phy_data;
        // We have something to send
        // Check if we already try to send it multiple times and save it on stats if it is
        if (nbrRetry >= NBR_RETRY)
        {
            // We failed to transmit this message. We can't allow it, there is an issue on this target.
            Phy_DeadTargetSpotted(robus_phy, job);
            nbrRetry         = 0;
            ctx.tx.collision = false;
            // Try to get a new job
            job = Phy_GetJob(robus_phy);
            if (job == NULL)
            {
                // Nothing to transmit anymore, just exit.
                return;
            }
        }
        // Check if we will need an ACK for this message and compute the transmit status we will need to manage
        transmitStatus_t initial_transmit_status = TX_OK;
        if (job->ack == true)
        {
            // We will need to validate the good reception with a ack.
            // Switch the tx status as TX_NOK allowing to detect a default at the next Timeout if no ACK have been received.
            initial_transmit_status = TX_NOK;
        }
        // Now we can try to send something
        if (Transmit_GetLockStatus() == false)
        {
            // We are free to transmit
            // We will prepare to transmit something enable tx status with precomputed value of the initial_transmit_status
            ctx.tx.status = initial_transmit_status;
            // Lock the bus
            ctx.tx.lock = true;
            RobusHAL_SetRxDetecPin(false);
            // Switch reception in collision detection mode
            LuosHAL_SetIrqState(false);
            ctx.rx.callback = Recep_GetCollision;
            LuosHAL_SetIrqState(true);
            ctx.tx.data = tx_data;

            if (!nbrRetry)
            {
                // This is the first time we try to send this message, we need to backup the original crc value and the job data to the TX_data buffer
                crc_val = jobEncaps->crc;
                memcpy(tx_data, job->data_pt, job->size);
                // Add the end of the message in the end of the buffer
                memcpy(&tx_data[job->size], jobEncaps->unmaped, jobEncaps->size);
            }

            // Put timestamping on data here
            if (job->timestamp)
            {

                // Convert date to a sendable timestamp and put it on the encapsulation
                jobEncaps->timestamp = Phy_ComputeTimestamp(job);

                jobEncaps->timestamped_crc = ll_crc_compute(jobEncaps->unmaped, sizeof(time_luos_t), crc_val);
                jobEncaps->size            = sizeof(time_luos_t) + CRC_SIZE;
                // Add the end of the message in the end of the buffer
                memcpy(&tx_data[job->size], jobEncaps->unmaped, jobEncaps->size);
            }

            // Transmit data
            if (Phy_GetJobNbr(robus_phy) != 0)
            {
                LUOS_ASSERT((job->size + jobEncaps->size) >= 9);
                // We still have something to send, no reset occured
                RobusHAL_ComTransmit(tx_data, (job->size + jobEncaps->size));
            }
        }
    }
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
        if (robus_phy->job_nb > 0)
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
