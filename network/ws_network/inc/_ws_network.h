/******************************************************************************
 * @file _serial_network.h
 * @brief Private serial communication driver functions for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#ifndef __WS_H_
#define __WS_H_

#include <stdint.h>
#include "ws_config.h"

void Ws_Reception(uint8_t *data, uint32_t size);

#endif /* __WS_H_ */
