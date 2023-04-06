/******************************************************************************
 * @file service.h
 * @brief Service related functions
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "service_structs.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void Service_Init(void);
service_t *Service_GetTable(void);
uint16_t Service_GetNumber(void);
void Service_ResetStatistics(void);
void Service_GenerateId(uint16_t base_id);
service_t *Service_GetService(ll_service_t *ll_service);
uint16_t Service_GetIndex(service_t *service);
void Service_AutoUpdateManager(void);
#endif /* _SERVICE_H_ */
