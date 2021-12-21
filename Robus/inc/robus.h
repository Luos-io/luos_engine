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
#include "robus_struct.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    NETWORK_LINK_DOWN,
    NETWORK_LINK_CONNECTING,
    NETWORK_LINK_UP
} network_state_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Robus_Init(memory_stats_t *memory_stats);
void Robus_Loop(void);
ll_service_t *Robus_ServiceCreate(uint16_t type);
void Robus_ServicesClear(void);
error_return_t Robus_SendMsg(ll_service_t *ll_service, msg_t *msg);
uint16_t Robus_TopologyDetection(ll_service_t *ll_service);
node_t *Robus_GetNode(void);
void Robus_Flush(void);
void Robus_ShiftMaskCalculation(uint16_t ID, uint16_t ServiceNumber);
void Robus_SetNodeDetected(network_state_t);
network_state_t Robus_IsNodeDetected(void);
void Robus_SetFilterState(uint8_t state, ll_service_t *service);
void Robus_SetVerboseMode(uint8_t mode);

#endif /* _ROBUS_H_ */
