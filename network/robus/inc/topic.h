/******************************************************************************
 * @file topic
 * @brief multicast protocole description
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _TOPIC_H_
#define _TOPIC_H_

#include "context.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
uint8_t Topic_IsTopicSubscribed(ll_service_t *ll_service, uint16_t topic_id);
error_return_t Topic_Subscribe(ll_service_t *ll_service, uint16_t topic_id);
error_return_t Topic_Unsubscribe(ll_service_t *ll_service, uint16_t topic_id);

#endif /* _TARGET_H_ */
