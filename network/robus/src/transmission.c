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

#include <transmission.h>

#include "robus_hal.h"
#include "luos_hal.h"
#include <string.h>
#include <stdbool.h>
#include "context.h"
#include "reception.h"
#include "msg_alloc.h"
#include "_timestamp.h"
#include "luos_engine.h"

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
uint16_t ll_crc_compute(uint8_t *data, uint16_t size, uint16_t crc_seed)
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
    uint8_t *data = 0;
    uint16_t size;
    service_t *service_pt;
    if ((MsgAlloc_GetTxTask(&service_pt, &data, &size) == SUCCEED) && (Transmit_GetLockStatus() == false))
    {
        // We have something to send
        // Check if we already try to send it multiple times and save it on stats if it is
        if ((service_pt->statistics.max_retry < nbrRetry) || (nbrRetry >= NBR_RETRY))
        {
            service_pt->statistics.max_retry = nbrRetry;
            if (nbrRetry >= NBR_RETRY)
            {
                // We failed to transmit this message. We can't allow it, there is a issue on this target.
                service_pt->dead_service_spotted = (uint16_t)(((msg_t *)data)->header.target);
                nbrRetry                         = 0;
                ctx.tx.collision                 = false;
                // Remove all transmist messages of this specific target
                MsgAlloc_PullServiceFromTxTask((uint16_t)(((msg_t *)data)->header.target));
                // Try to get a tx_task for another service
                if (MsgAlloc_GetTxTask(&service_pt, &data, &size) == FAILED)
                {
                    // Nothing to transmit anymore, just exit.
                    return;
                }
            }
        }
        // Check if we will need an ACK for this message and compute the transmit status we will need to manage it
        transmitStatus_t initial_transmit_status = TX_OK;
        if (((((msg_t *)data)->header.target_mode == SERVICEIDACK) || (((msg_t *)data)->header.target_mode == NODEIDACK)))
        {
            // We will need to validate the good reception with a ack.
            // Switch the tx status as TX_NOK allowing to detect a default at the next Timeout if no ACK have been received.
            initial_transmit_status = TX_NOK;
        }
        // Now we can try to send something
        if (Transmit_GetLockStatus() == false)
        {
            // We are free to transmit
            // We will prepare to transmit something enable tx status with precomputed value if we need ACK
            ctx.tx.status = initial_transmit_status;
            // Lock the bus
            ctx.tx.lock = true;
            RobusHAL_SetRxDetecPin(false);
            // Switch reception in collision detection mode
            LuosHAL_SetIrqState(false);
            ctx.rx.callback = Recep_GetCollision;
            LuosHAL_SetIrqState(true);
            ctx.tx.data = data;

            // Put timestamping on data here
            if (Luos_IsMsgTimstamped((msg_t *)data) && (!nbrRetry))
            {
                // Convert date to latency
                Timestamp_ConvertToLatency((msg_t *)data);

                // Complete the CRC computation with the latency
                msg_t *msg                       = (msg_t *)data;
                uint16_t full_size               = sizeof(header_t) + msg->header.size + sizeof(time_luos_t) + CRC_SIZE;
                uint16_t index_without_timestamp = sizeof(header_t) + msg->header.size;
                uint16_t crc_seed                = 0;
                memcpy(&crc_seed, &msg->stream[full_size - CRC_SIZE], CRC_SIZE);
                uint16_t crc_val = ll_crc_compute(&msg->stream[index_without_timestamp], full_size - CRC_SIZE - index_without_timestamp, crc_seed);

                // copy crc in message
                memcpy(&msg->stream[full_size - CRC_SIZE], &crc_val, CRC_SIZE);
            }

            // Transmit data
            if (MsgAlloc_TxAllComplete() == FAILED)
            {
                // There is no more task to send. the network have been reseted
                RobusHAL_ComTransmit(data, size);
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
        // A tx_task have been sucessfully transmitted
        nbrRetry         = 0;
        ctx.tx.collision = false;
        ctx.tx.status    = TX_DISABLE;
        // Remove the task
        MsgAlloc_PullMsgFromTxTask();
    }
    else if (ctx.tx.status == TX_NOK)
    {
        // A tx_task failed
        nbrRetry++;
        // compute a delay before retry
        RobusHAL_ResetTimeout(20 * nbrRetry * (Node_Get()->node_id + 1));
        // Lock the trasmission to be sure no one can send something from this node.
        ctx.tx.lock   = true;
        ctx.tx.status = TX_DISABLE;
        return;
    }
    ctx.tx.lock = false;
    // Try to send something if we need to.
    Transmit_Process();
}