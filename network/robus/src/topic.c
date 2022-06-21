/******************************************************************************
 * @file topic
 * @brief multicast protocole description
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "topic.h"
#include "stdbool.h"
#include <string.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief lookink for a topic in multicast list
 * @param service in multicast
 * @param multicast bank
 * @return Error
 ******************************************************************************/
uint8_t Topic_IsTopicSubscribed(ll_service_t *ll_service, uint16_t topic_id)
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
 * @brief add a topic to the list
 * @param service in multicast
 * @param topic_add to add
 * @return Error
 ******************************************************************************/
error_return_t Topic_Subscribe(ll_service_t *ll_service, uint16_t topic_id)
{
    // check if target exists or if we reached the maximum topics number
    if ((Topic_IsTopicSubscribed(ll_service, topic_id) == false) && (ll_service->last_topic_position < LAST_TOPIC))
    {
        ll_service->topic_list[ll_service->last_topic_position] = topic_id;
        ll_service->last_topic_position++;
        return SUCCEED;
    }
    return FAILED;
}

/******************************************************************************
 * @brief remove a topic from the service list
 * @param service in multicast
 * @param topic_id to remove
 * @return Error
 ******************************************************************************/
error_return_t Topic_Unsubscribe(ll_service_t *ll_service, uint16_t topic_id)
{
    unsigned char i;
    for (i = 0; i < ll_service->last_topic_position; i++)
    {
        if (ll_service->topic_list[i] == topic_id)
        {
            if (ll_service->last_topic_position >= LAST_TOPIC)
            {
                return FAILED;
            }
            memcpy(&ll_service->topic_list[i], &ll_service->topic_list[i + 1], ll_service->last_topic_position - i);
            ll_service->last_topic_position--;
            return SUCCEED;
        }
    }
    return FAILED;
}
