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
    NO_DETECTION,
    DETECTION_OK,
    LOCAL_DETECTION,
    EXTERNAL_DETECTION,
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
error_return_t Robus_SetTxTask(ll_service_t *ll_service, msg_t *msg);
error_return_t Robus_SendMsg(ll_service_t *ll_service, msg_t *msg);
uint16_t Robus_TopologyDetection(ll_service_t *ll_service);
node_t *Robus_GetNode(void);
void Robus_IDMaskCalculation(uint16_t service_id, uint16_t service_number);
void Robus_SetNodeDetected(network_state_t);
network_state_t Robus_IsNodeDetected(void);
void Robus_SetFilterState(uint8_t state, ll_service_t *service);
void Robus_SetVerboseMode(uint8_t mode);
void Robus_MaskInit(void);
error_return_t Robus_TopicSubscribe(ll_service_t *ll_service, uint16_t topic_id);
error_return_t Robus_TopicUnsubscribe(ll_service_t *ll_service, uint16_t topic_id);

#endif /* _ROBUS_H_ */
