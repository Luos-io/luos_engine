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
uint8_t PubSub_IsTopicSubscribed(service_t *service, uint16_t topic_id)
{
    unsigned char i;
    for (i = 0; i < service->last_topic_position; i++)
    {
        if (service->topic_list[i] == topic_id)
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
    LUOS_ASSERT(service != 0);

    // Put this topic in the multicast bank
    Filter_AddTopic(topic);

    // Check if target exists or if we reached the maximum topics number
    if ((PubSub_IsTopicSubscribed(service, topic) == false) && (service->last_topic_position < LAST_TOPIC))
    {
        service->topic_list[service->last_topic_position] = topic;
        service->last_topic_position++;
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
    LUOS_ASSERT(topic <= LAST_TOPIC);
    LUOS_ASSERT(service != 0);

    error_return_t err = FAILED;
    // Delete topic from service list
    for (uint16_t i = 0; i < service->last_topic_position; i++)
    {
        if (service->topic_list[i] == topic)
        {
            if (service->last_topic_position >= LAST_TOPIC)
            {
                break;
            }
            memcpy(&service->topic_list[i], &service->topic_list[i + 1], service->last_topic_position - i);
            service->last_topic_position--;
            err = SUCCEED;
            break;
        }
    }

    // Recompute multicast mask if needed
    if (err == SUCCEED)
    {
        for (uint16_t i = 0; i < Service_GetNumber(); i++)
        {
            if (PubSub_IsTopicSubscribed(&Service_GetTable()[i], topic) == true)
            {
                return err;
            }
        }
        // Remove topic from multicast mask
        Filter_RmTopic(topic);
    }
    return err;
}