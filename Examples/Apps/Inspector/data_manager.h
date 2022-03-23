/******************************************************************************
 * @file data manager
 * @brief data manager for inspector
 * @author Luos
 ******************************************************************************/
#include "inspector.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    STOPPED,
    STARTED
} insp_state_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void DataManager_GetPipeMsg(service_t *service, msg_t *data_msg);
uint8_t DataManager_GetInspectorState(void);
void DataManager_SendRoutingTB(service_t *service);
void DataManager_GetServiceMsg(service_t *service);