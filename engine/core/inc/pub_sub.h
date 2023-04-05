/******************************************************************************
 * @file pub_sub.h
 * @brief multicast referencing description
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _PUB_SUB_H_
#define _PUB_SUB_H_

#include "struct_luos.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
uint8_t PubSub_IsTopicSubscribed(service_t *service, uint16_t topic_id);

#endif /* _PUB_SUB_H_ */
