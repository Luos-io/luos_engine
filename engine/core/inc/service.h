/******************************************************************************
 * @file service.h
 * @brief Service related functions
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "struct_luos.h"
#include "luos_io.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (MAX_SERVICE_NUMBER <= 8)
typedef uint8_t service_filter_t;
#elif (MAX_SERVICE_NUMBER <= 16)
typedef uint16_t service_filter_t;
#elif (MAX_SERVICE_NUMBER <= 32)
typedef uint32_t service_filter_t;
#elif (MAX_SERVICE_NUMBER <= 64)
typedef uint64_t service_filter_t;
#else
    #error "MAX_SERVICE_NUMBER is too high"
#endif

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
void Service_RmAutoUpdateTarget(uint16_t service_id);
void Service_AutoUpdateManager(void);
error_return_t Service_Deliver(phy_job_t *job);

// IO related functions
service_t *Service_GetConcerned(const header_t *header);
service_filter_t Service_GetFilter(const msg_t *msg);

#endif /* _SERVICE_H_ */
