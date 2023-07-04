/******************************************************************************
 * @file serial_network.h
 * @brief serial communication driver for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOS_SERIAL_H_
#define _LUOS_SERIAL_H_

#include "luos_phy.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef SERIAL_TX_BUFFER_SIZE
    #define SERIAL_RX_BUFFER_SIZE 512
#endif

/*******************************************************************************
 * Function
 ******************************************************************************/
void Serial_Init(void);
void Serial_Loop(void);

#endif /* _LUOS_SERIAL_H_ */
