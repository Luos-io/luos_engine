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
    volatile uint8_t lock;                // Transmit lock state
    uint8_t *data;                        // data to compare for collision detection
    volatile uint8_t collision;           // collision flag
    volatile uint16_t additionalDelay_us; // Aditional delay due to collision or Nak
    ll_container_t llContainer;           // Container sending the message
    uint8_t transmitComplete;             // A flag to know if a transmission have been completed
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
