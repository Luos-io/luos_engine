/******************************************************************************
 * @file serial_hal
 * @brief hardware abstraction layer of serial communication driver for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#ifndef _SERIAL_HAL_H_
#define _SERIAL_HAL_H_

#include "stdint.h"
#include "stdbool.h"

void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size); // Init the serial communication
void SerialHAL_Loop(void);                                     // Do your loop stuff if needed
void SerialHAL_Send(uint8_t *data, uint16_t size);             // Send data trough the DMA
uint8_t SerialHAL_GetPort(void);                               // Return the port number of the serial communication

#endif /* _SERIAL_HAL_H_ */
