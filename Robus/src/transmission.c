/******************************************************************************
 * @file sys_msg
 * @brief protocol system message management.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <transmission.h>

#include "luos_hal.h"
#include <string.h>
#include <stdbool.h>
#include "context.h"
#include "reception.h"
#include "msg_alloc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t ack_transmission = false;

/*******************************************************************************
 * Function
 ******************************************************************************/
static uint8_t Transmit_GetLockStatus(void);

/******************************************************************************
 * @brief detect network topologie
 * @param None
 * @return None
 ******************************************************************************/
void Transmit_SendAck(void)
{
    LuosHAL_SetRxState(false);
    // Transmit Ack data
    LuosHAL_ComTransmit((unsigned char *)&ctx.rx.status.unmap, 1);
    // Reset Ack status
    ctx.rx.status.unmap = 0x0F;
    ack_transmission = true;
}
/******************************************************************************
 * @brief transmission process
 * @param pointer data to send
 * @param size of data to send
 * @return None
 ******************************************************************************/
void Transmit_Process()
{
    uint8_t *data = 0;
    uint16_t size;
    uint8_t localhost;
    ll_container_t *ll_container_pt;

    if ((MsgAlloc_GetTxTask(&ll_container_pt, &data, &size, &localhost) == SUCCEED) && (Transmit_GetLockStatus() == false))
    {
        // Check if ACK needed
        if ((((msg_t *)data)->header.target_mode == IDACK) || (((msg_t *)data)->header.target_mode == NODEIDACK))
        {
            // Check if it is a localhost message
            if (localhost && (((msg_t *)data)->header.target != DEFAULTID))
            {
                // We don't need to validate the good reception af the ack
                ctx.ack_needed = false;
            }
            else
            {
                // We need ta validate the good reception af the ack, change the state of state machine after the end of collision detection to wait a ACK
                ctx.ack_needed = true;
            }
        }
        ctx.tx.lock = true;
        // switch reception in collision detection mode
        ctx.tx.collision = false;
        LuosHAL_SetIrqState(false);
        ctx.rx.callback = Recep_GetCollision;
        ctx.tx.data = data;
        LuosHAL_SetIrqState(true);
        LuosHAL_ComTransmit(data, size);
    }
}
/******************************************************************************
 * @brief Send ID to others container on network
 * @param None
 * @return lock status
 ******************************************************************************/
static uint8_t Transmit_GetLockStatus(void)
{
    if (ctx.tx.lock != true)
    {
        ctx.tx.lock |= LuosHAL_GetTxLockState();
    }
    return ctx.tx.lock;
}
/******************************************************************************
 * @brief finish transmit and try to launch a new one
 * @param None
 * @return None
 ******************************************************************************/
void Transmit_End(void)
{
    if ((ctx.tx.transmitComplete) && (ack_transmission == false))
    {
        // We previously complete a transmission and it was not a ACK
        // Check the Ack status
        if (!ctx.ack_needed)
        {
            // we don't needed Ack or we received one
            // we can remove the task
            MsgAlloc_PullMsgFromTxTask();
        }
    }
    ctx.tx.transmitComplete = false;
    ack_transmission = false;
    Transmit_Process();
}
