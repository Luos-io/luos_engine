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
#include "io_struct.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Robus_Init(memory_stats_t *memory_stats);
void Robus_Loop(void);

void Robus_ServicesClear(void);
ll_service_t *Robus_GetLlServiceList(void);

error_return_t Robus_SetTxTask(ll_service_t *ll_service, msg_t *msg);
error_return_t Robus_SendMsg(ll_service_t *ll_service, msg_t *msg);
uint16_t Robus_TopologyDetection(ll_service_t *ll_service);

#endif /* _ROBUS_H_ */
