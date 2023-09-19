/******************************************************************************
 * @file _serial_network.h
 * @brief Private serial communication driver functions for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#ifndef __SERIAL_H_
#define __SERIAL_H_

#include <stdint.h>
#include "serial_config.h"

void Serial_TransmissionEnd(void);
void Serial_ReceptionWrite(uint8_t *data, uint32_t size);
void Serial_ReceptionIncrease(uint32_t size);

#endif /* __SERIAL_H_ */
