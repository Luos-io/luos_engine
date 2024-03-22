/******************************************************************************
 * @file ws_hal
 * @brief Websocket Hardware Abstration Layer. Describe Low layer fonctions
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _WSHAL_H_
#define _WSHAL_H_

#include <stdint.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void WsHAL_Init(void);                               // Init the Websocket communication
void WsHAL_Loop(void);                               // Do your loop stuff if needed
void WsHAL_Send(const uint8_t *data, uint16_t size); // Send data

#endif /* _WSHAL_H_ */
