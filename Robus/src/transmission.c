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
    LuosHAL_ComTransmit((unsigned char *)&ctx.rx.status.unmap, 1);
    LuosHAL_ComTxComplete();
    LuosHAL_SetRxState(true);
    LuosHAL_SetTxState(false);
    ctx.rx.status.unmap = 0x0F;
}
/******************************************************************************
 * @brief transmission process
 * @param pointer data to send
 * @param size of data to send
 * @return Error
 ******************************************************************************/
error_return_t Transmit_Process(uint8_t *data, uint16_t size)
{
    uint16_t crc_val = 0xFFFF;
    // wait tx unlock
    Transmit_WaitUnlockTx();
    // compute the CRC
    for (uint16_t i = 0; i < size - 2; i++)
    {
        LuosHAL_ComputeCRC(&data[i], (uint8_t *)&crc_val);
    }
    data[size - 2] = (uint8_t)(crc_val);
    data[size - 1] = (uint8_t)(crc_val >> 8);

    // lock the transmission
    if (Transmit_GetLockStatus())
    {
        LuosHAL_SetTxLockDetecState(false);
        return FAILED;
    }
    LuosHAL_SetTxLockDetecState(false);

    // Enable TX
    LuosHAL_SetTxState(true);
    ctx.tx.lock = true;

    // switch reception in collision detection mode
    ctx.tx.collision = FALSE;
    LuosHAL_SetIrqState(false);
    ctx.rx.callback = Recep_GetCollision;
    ctx.tx.data = data;
    LuosHAL_SetIrqState(true);

    if (LuosHAL_ComTransmit(data, size))
    {
        //collision detected
        ctx.tx.collision = FALSE;
        return FAILED;
    }
    LuosHAL_ComTxComplete();
    LuosHAL_SetRxState(true);
    LuosHAL_SetTxState(false);
    return SUCCEED;
}
/******************************************************************************
 * @brief wait end of a transmission to be free
 * @param  None
 * @return None
 ******************************************************************************/
void Transmit_WaitUnlockTx(void) // TODO : This function could be in HAL and replace HAL_is_tx_lock. By the way timeout management here is shity
{
    while (Transmit_GetLockStatus())
        ;
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
