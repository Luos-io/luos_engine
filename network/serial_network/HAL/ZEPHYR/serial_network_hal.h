/******************************************************************************
 * @file serial_network_hal (ZEPHYR / radio)
 * @brief Binds the Luos serial network to the 2.4 GHz radio byte PHY.
 ******************************************************************************/
#ifndef _SERIAL_NETWORK_HAL_H_
#define _SERIAL_NETWORK_HAL_H_

#include <stdint.h>

void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size);
void SerialHAL_Loop(void);
void SerialHAL_Send(uint8_t *data, uint16_t size);
uint8_t SerialHAL_GetPort(void);

#endif /* _SERIAL_NETWORK_HAL_H_ */
