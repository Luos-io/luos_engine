/******************************************************************************
 * @file sys_msg
 * @brief protocol system message management.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _TRANSMISSION_H_
#define _TRANSMISSION_H_

#include "robus_struct.h"
#include <stdint.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    uint8_t *data;
    volatile uint8_t lock;
    volatile uint8_t collision;
} TxCom_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Transmit_SendAck(void);
error_return_t Transmit_Process(uint8_t *data, uint16_t size);
void Transmit_WaitUnlockTx(void);

#endif /* _TRANSMISSION_H_ */
