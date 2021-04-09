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
typedef enum
{
    TX_DISABLE, /*!< transmission with ack */
    TX_OK,      /*!< transmission ok */
    TX_NOK      /*!< transmission fail */

} transmitStatus_t;

typedef struct
{
    volatile uint8_t lock;            // Transmit lock state
    uint8_t *data;                    // data to compare for collision detection
    volatile transmitStatus_t status; // data to compare for collision detection
    volatile uint8_t collision;       // true is a collision occure during this transmission.
} TxCom_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Transmit_SendAck(void);
void Transmit_Process(void);
void Transmit_End(void);

#endif /* _TRANSMISSION_H_ */
