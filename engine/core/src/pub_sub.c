/******************************************************************************
 * @file topic
 * @brief multicast protocole description
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "pub_sub.h"
#include "luos_engine.h"
#include <stdbool.h>
#include <string.h>
#include "filter.h"
#include "service.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief lookink for a topic in multicast list
 * @param service in multicast
 * @param multicast bank
 * @return Error
 ******************************************************************************/
uint8_t PubSub_IsTopicSubscribed(ll_service_t *ll_service, uint16_t topic_id)
{
    unsigned char i;
    for (i = 0; i < ll_service->last_topic_position; i++)
    {
        if (ll_service->topic_list[i] == topic_id)
            return true;
    }
    return false;
}

/******************************************************************************
 * @brief Subscribe to a new topic
 * @param service
 * @param topic
 * @return None
 ******************************************************************************/
error_return_t Luos_Subscribe(service_t *service, uint16_t topic)
{
    // Assert if we add a topic that is greater than the max topic value
    LUOS_ASSERT(topic <= LAST_TOPIC);

    // Put this topic in the multicast bank
    Filter_AddTopic(topic);

    // add multicast this topic to the service
    ll_service_t *ll_service = (ll_service_t *)&ctx.ll_service_table[0];
    if (service != 0)
    {
        ll_service = service->ll_service;
    }

    // Check if target exists or if we reached the maximum topics number
    if ((PubSub_IsTopicSubscribed(ll_service, topic) == false) && (ll_service->last_topic_position < LAST_TOPIC))
    {
        ll_service->topic_list[ll_service->last_topic_position] = topic;
        ll_service->last_topic_position++;
        return SUCCEED;
    }
    return FAILED;
}

/******************************************************************************
 * @brief Unsubscribe from a topic
 * @param service
 * @param topic
 * @return None
 ******************************************************************************/
error_return_t Luos_Unsubscribe(service_t *service, uint16_t topic)
{
    error_return_t err       = FAILED;
    ll_service_t *ll_service = (ll_service_t *)&ctx.ll_service_table[0];
    unsigned char i;
    if (service != 0)
    {
        ll_service = service->ll_service;
    }
    // Delete topic from service list
    for (i = 0; i < ll_service->last_topic_position; i++)
    {
        if (ll_service->topic_list[i] == topic)
        {
            if (ll_service->last_topic_position >= LAST_TOPIC)
            {
                break;
            }
            memcpy(&ll_service->topic_list[i], &ll_service->topic_list[i + 1], ll_service->last_topic_position - i);
            ll_service->last_topic_position--;
            err = SUCCEED;
            break;
        }
    }

    // Recompute multicast mask if needed
    if (err == SUCCEED)
    {
        for (uint16_t i = 0; i < Service_GetNumber(); i++)
        {
            if (PubSub_IsTopicSubscribed((ll_service_t *)(&ctx.ll_service_table[i]), topic) == true)
            {
                return err;
            }
        }
        // Remove topic from multicast mask
        Filter_RmTopic(topic);
    }
    return err;
}