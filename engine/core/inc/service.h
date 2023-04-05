/******************************************************************************
 * @file service.h
 * @brief Service related functions
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "struct_luos.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void Service_Init(void);
service_t *Service_GetTable(void);
uint16_t Service_GetNumber(void);
void Service_ResetStatistics(void);
void Service_GenerateId(uint16_t base_id);
void Service_ClearId(void);
uint16_t Service_GetIndex(service_t *service);
void Service_AutoUpdateManager(void);

// IO related functions
service_t *Service_GetConcerned(header_t *header);
void Service_AllocMsg(msg_t *msg);

#endif /* _SERVICE_H_ */
