/******************************************************************************
 * @file serial_hal
 * @brief hardware abstraction layer of serial communication driver for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#ifndef _SERIAL_HAL_H_
#define _SERIAL_HAL_H_

#include "stdint.h"
#include "serial_hal_config.h"

void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size);
void SerialHAL_Loop(void);
void SerialHAL_Send(uint8_t *data, uint16_t size);
uint8_t SerialHAL_GetPort(void); // Return the port number of the serial communication

#endif /* _SERIAL_HAL_H_ */
