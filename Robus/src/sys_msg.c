/******************************************************************************
 * @file sys_msg
 * @brief protocol system message management.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <sys_msg.h>

#include "luosHAL.h"
#include <string.h>
#include <stdbool.h>
#include "context.h"
#include "reception.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

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
    LuosHAL_SetTxState(true);
    LuosHAL_SetRxState(false);
    LuosHAL_ComTransmit((unsigned char *)&ctx.status.unmap, 1);
    LuosHAL_ComTxTimeout();
    LuosHAL_SetRxState(true);
    LuosHAL_SetTxState(false);
    ctx.status.unmap = 0x0F;
}
/******************************************************************************
 * @brief transmission process
 * @param pointer data to send
 * @param size of data to send
 * @return Error
 ******************************************************************************/
uint8_t Transmit_Process(uint8_t *data, uint16_t size)
{
    const int col_check_data_num = 5;
    // wait tx unlock
    Transmit_WaitUnlockTx();
    // compute the CRC
    LuosHAL_ComputeCRC(data, size - 2, (unsigned char *)&data[size - 2]);
    ctx.collision = FALSE;
    // Enable TX
    LuosHAL_SetTxState(true);
    LuosHAL_SetIrqState(false);
    // switch reception in collision detection mode
    ctx.data_cb = Recep_GetCollision;
    ctx.tx_data = data;
    LuosHAL_SetIrqState(true);
    // re-lock the transmission
    if (ctx.collision | Transmit_GetLockStatus())
    {
        // We receive something during our configuration, stop this transmission
        LuosHAL_SetTxState(false);
        ctx.collision = FALSE;
        return 1;
    }
    ctx.tx_lock = true;
    LuosHAL_SetTxLockDetecState(false);
    // Try to detect a collision during the "col_check_data_num" first bytes
    if (LuosHAL_ComTransmit(data, col_check_data_num))
    {
        LuosHAL_SetTxState(false);
        ctx.collision = FALSE;
        return 1;
    }
    // No collision occure, stop collision detection mode and continue to transmit
    LuosHAL_SetIrqState(false);
    ctx.data_cb = Recep_GetHeader;
    LuosHAL_SetIrqState(true);
    LuosHAL_SetRxState(false);
    LuosHAL_ComTransmit(data + col_check_data_num, size - col_check_data_num);
    LuosHAL_ComTxTimeout();
    // get ready to receive a ack just in case
    // disable TX and Enable RX
    LuosHAL_SetRxState(true);
    LuosHAL_SetTxState(false);
    // Force Usart Timeout
    Recep_Timeout();
    return 0;
}
/******************************************************************************
 * @brief wait end of a transmission to be free
 * @param  None
 * @return None
 ******************************************************************************/
void Transmit_WaitUnlockTx(void) // TODO : This function could be in HAL and replace HAL_is_tx_lock. By the way timeout management here is shity
{
    volatile int timeout = 0;
    while (Transmit_GetLockStatus() && (timeout < 64000))
    {
        timeout++;
    }
}
/******************************************************************************
 * @brief Send ID to others module on network
 * @param None
 * @return lock status
 ******************************************************************************/
static uint8_t Transmit_GetLockStatus(void)
{
    if (ctx.tx_lock != true)
    {
        ctx.tx_lock |= LuosHAL_GetTxLockState();
    }
    return ctx.tx_lock;
}
