/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUS_H_
#define _ROBUS_H_

#include <stdint.h>
#include <stdbool.h>
#include "struct_luos.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Robus_Init(memory_stats_t *memory_stats);
void Robus_Loop(void);

error_return_t Robus_SetTxTask(service_t *service, msg_t *msg);
error_return_t Robus_SendMsg(service_t *service, msg_t *msg);
uint16_t Robus_TopologyDetection(service_t *service);

#endif /* _ROBUS_H_ */
