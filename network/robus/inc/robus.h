/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUS_H_
#define _ROBUS_H_

#include "luos_io.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void Robus_Init(void);
void Robus_Loop(void);

error_return_t Robus_SetTxTask(service_t *service, msg_t *msg);
error_return_t Robus_SendMsg(service_t *service, msg_t *msg);
uint16_t Robus_TopologyDetection(service_t *service);
error_return_t Robus_MsgHandler(msg_t *input);

#endif /* _ROBUS_H_ */
