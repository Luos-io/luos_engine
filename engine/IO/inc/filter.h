/******************************************************************************
 * @file filter.c
 * @brief Calculate filter for Phy and compare filter
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdbool.h>
#include <stdint.h>
#include "struct_luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

// generic functions
void Filter_IdInit(void);
void Filter_TopicInit(void);
void Filter_AddServiceId(uint16_t service_id, uint16_t service_number);
void Filter_AddTopic(uint16_t topic_id);
void Filter_RmTopic(uint16_t topic_id);
bool Filter_ServiceID(uint16_t service_id);
bool Filter_Topic(uint16_t topic_id);
bool Filter_Type(uint16_t type_id);
uint8_t Filter_GetPhyTarget(header_t *header);

#endif /* _FILTER_H_ */
