/******************************************************************************
 * @file Service
 * @brief Service related functions
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <string.h>
#include "service.h"
#include "filter.h"
#include "luos_list.h"
#include "node.h"
#include "luos_utils.h"
#include "luos_hal.h"
#include "msg_alloc.h"
#include "pub_sub.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    service_t list[MAX_SERVICE_NUMBER];
    uint16_t number;
} service_ctx_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_ctx_t service_ctx;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief API to Init the service table
 * @param None
 * @return None
 ******************************************************************************/
void Service_Init(void)
{
    service_ctx.number = 0;
}

/******************************************************************************
 * @brief API to get the service table
 * @param None
 * @return service table
 ******************************************************************************/
service_t *Service_GetTable(void)
{
    return service_ctx.list;
}

/******************************************************************************
 * @brief API to get the service number
 * @param None
 * @return service number
 ******************************************************************************/
uint16_t Service_GetNumber(void)
{
    return service_ctx.number;
}

/******************************************************************************
 * @brief API to reset all service statistics
 * @param None
 * @return None
 ******************************************************************************/
void Service_ResetStatistics(void)
{
    for (uint16_t i = 0; i < service_ctx.number; i++)
    {
        service_ctx.list[i].statistics.max_retry = 0;
    }
}

/******************************************************************************
 * @brief API to generate a service ID
 * @param baseId : base ID of the services, this value will be increased for eeach services
 * @return None
 ******************************************************************************/
void Service_GenerateId(uint16_t base_id)
{
    LUOS_ASSERT(base_id > 0);
    Filter_AddServiceId(base_id, service_ctx.number);
    if (base_id == 1)
    {
        // If base_id is 1, it means that the node is a detector.
        // Id 1 would be reserved for the already setted detector service.
        // So we need to avoid the service with id 1 and start with id 2.
        base_id = 2;
    }
    for (uint16_t i = 0; i < service_ctx.number; i++)
    {
        if (service_ctx.list[i].ll_service->id != 1)
        {
            service_ctx.list[i].ll_service->id = base_id++;
        }
    }
}

/******************************************************************************
 * @brief Clear all service ID
 * @return None
 ******************************************************************************/
void Service_ClearId(void)
{
    for (uint16_t i = 0; i < service_ctx.number; i++)
    {
        service_ctx.list[i].ll_service->id = DEFAULTID;
    }
}

/******************************************************************************
 * @brief Get pointer to a service based on ll_service
 * @param ll_service : low level service
 * @return Service from list
 ******************************************************************************/
service_t *Service_GetService(ll_service_t *ll_service)
{
    for (uint16_t i = 0; i < service_ctx.number; i++)
    {
        if (ll_service == service_ctx.list[i].ll_service)
        {
            return &service_ctx.list[i];
        }
    }
    return 0;
}

/******************************************************************************
 * @brief Get this service index in the service table
 * @param Service
 * @return Service id the service list
 ******************************************************************************/
uint16_t Service_GetIndex(service_t *service)
{
    for (uint16_t i = 0; i < service_ctx.number; i++)
    {
        if (service == &service_ctx.list[i])
        {
            return i;
        }
    }
    return 0xFFFF;
}

/******************************************************************************
 * @brief Auto update call for services
 * @param none
 * @return none
 ******************************************************************************/
void Service_AutoUpdateManager(void)
{
    // check all services timed_update_t contexts
    for (uint16_t i = 0; i < service_ctx.number; i++)
    {
        // check if services have an actual ID. If not, we are in detection mode and should reset the auto refresh
        if (service_ctx.list[i].ll_service->id == DEFAULTID)
        {
            // this service have not been detected or is in detection mode. remove auto_refresh parameters
            service_ctx.list[i].auto_refresh.target      = 0;
            service_ctx.list[i].auto_refresh.time_ms     = 0;
            service_ctx.list[i].auto_refresh.last_update = 0;
        }
        else
        {
            // check if there is a timed update setted and if it's time to update it.
            if (service_ctx.list[i].auto_refresh.time_ms)
            {
                if (service_ctx.list[i].ll_service->dead_service_spotted == service_ctx.list[i].auto_refresh.target)
                {
                    service_ctx.list[i].auto_refresh.target      = 0;
                    service_ctx.list[i].auto_refresh.time_ms     = 0;
                    service_ctx.list[i].auto_refresh.last_update = 0;
                    continue;
                }
                if ((LuosHAL_GetSystick() - service_ctx.list[i].auto_refresh.last_update) >= service_ctx.list[i].auto_refresh.time_ms)
                {
                    // This service need to send an update
                    // Create a fake message for it from the service asking for update
                    msg_t updt_msg;
                    updt_msg.header.config      = BASE_PROTOCOL;
                    updt_msg.header.target      = service_ctx.list[i].ll_service->id;
                    updt_msg.header.source      = service_ctx.list[i].auto_refresh.target;
                    updt_msg.header.target_mode = SERVICEIDACK;
                    updt_msg.header.cmd         = GET_CMD;
                    updt_msg.header.size        = 0;
                    if ((service_ctx.list[i].service_cb != 0))
                    {
                        service_ctx.list[i].service_cb(&service_ctx.list[i], &updt_msg);
                    }
                    else
                    {
                        if (Node_GetState() == DETECTION_OK)
                        {
                            // directly transmit the message in Localhost
                            Robus_SetTxTask(service_ctx.list[i].ll_service, &updt_msg);
                        }
                    }
                    service_ctx.list[i].auto_refresh.last_update = LuosHAL_GetSystick();
                }
            }
        }
    }
}

/******************************************************************************
 * @brief Parse msg to find a service concerned
 * @param header of message
 * @return ll_service pointer
 ******************************************************************************/
ll_service_t *Service_GetConcerned(header_t *header)
{
    uint16_t i = 0;
    LUOS_ASSERT(header);
    // Find if we are concerned by this message.
    switch (header->target_mode)
    {
        case SERVICEIDACK:
        case SERVICEID:
            // Check all ll_service id
            for (i = 0; i < service_ctx.number; i++)
            {
                if (header->target == service_ctx.list[i].ll_service->id)
                {
                    return service_ctx.list[i].ll_service;
                }
            }
            break;
        case TYPE:
            // Check all ll_service type
            for (i = 0; i < service_ctx.number; i++)
            {
                if (header->target == service_ctx.list[i].ll_service->type)
                {
                    return service_ctx.list[i].ll_service;
                }
            }
            break;
        case BROADCAST:
        case NODEIDACK:
        case NODEID:
            return service_ctx.list[0].ll_service;
            break;
        case TOPIC:
        default:
            return NULL;
            break;
    }
    return NULL;
}

/******************************************************************************
 * @brief Parse msg to find all services concerned and allocate them
 * @param msg pointer
 * @return None
 ******************************************************************************/
void Service_AllocMsg(msg_t *msg)
{
    uint16_t i = 0;

    // Find if we are concerned by this message.
    switch (msg->header.target_mode)
    {
        case SERVICEIDACK:
        case SERVICEID:
            // Check all ll_service id
            for (i = 0; i < service_ctx.number; i++)
            {
                if (msg->header.target == service_ctx.list[i].ll_service->id)
                {
                    MsgAlloc_LuosTaskAlloc(service_ctx.list[i].ll_service, msg);
                    break;
                }
            }
            return;
            break;
        case TYPE:
            // Check all ll_service type
            for (i = 0; i < service_ctx.number; i++)
            {
                if (msg->header.target == service_ctx.list[i].ll_service->type)
                {
                    MsgAlloc_LuosTaskAlloc(service_ctx.list[i].ll_service, msg);
                }
            }
            return;
            break;
        case BROADCAST:
            for (i = 0; i < service_ctx.number; i++)
            {
                MsgAlloc_LuosTaskAlloc(service_ctx.list[i].ll_service, msg);
            }
            return;
            break;
        case TOPIC:
            for (i = 0; i < service_ctx.number; i++)
            {
                if (PubSub_IsTopicSubscribed(service_ctx.list[i].ll_service, msg->header.target))
                {
                    // TODO manage multiple slave concerned
                    MsgAlloc_LuosTaskAlloc(service_ctx.list[i].ll_service, msg);
                }
            }
            return;
            break;
        case NODEIDACK:
        case NODEID:
            if (msg->header.target == DEFAULTID) // on default ID it's always a luos command create only one task
            {
                MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[0], msg);
                return;
            }
            // check if the message is really for the node or it is a service that has no filter
            if (msg->header.target == Node_Get()->node_id)
            {
                for (i = 0; i < service_ctx.number; i++)
                {
                    MsgAlloc_LuosTaskAlloc(service_ctx.list[i].ll_service, msg);
                }
            }
            return;
            break;
        default:
            break;
    }
}

/******************************************************************************
 * @brief API to Create a service
 * @param service_cb : Callback msg handler for the service
 * @param type of service corresponding to object dictionnary
 * @param alias for the service string (15 caracters max).
 * @param version FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return Service object pointer.
 ******************************************************************************/
service_t *Luos_CreateService(SERVICE_CB service_cb, uint8_t type, const char *alias, revision_t revision)
{
    uint8_t i           = 0;
    service_t *service  = &service_ctx.list[service_ctx.number];
    service->ll_service = &Robus_GetLlServiceList()[service_ctx.number];

    // Set the service type
    service->ll_service->type = type;
    // Initialise the service id, TODO the ID could be stored in EEprom, the default ID could be set in factory...
    service->ll_service->id = DEFAULTID;
    // Initialize dead service detection
    service->ll_service->dead_service_spotted = 0;
    // Clear stats
    service->ll_service->ll_stat.max_retry = 0;
    // Clear topic number
    service->ll_service->last_topic_position = 0;
    for (uint16_t i = 0; i < LAST_TOPIC; i++)
    {
        service->ll_service->topic_list[i] = 0;
    }

    // Link the service to his callback
    service->service_cb = service_cb;

    // Initialise the service aliases to 0
    memset((void *)service->default_alias, 0, MAX_ALIAS_SIZE);
    memset((void *)service->alias, 0, MAX_ALIAS_SIZE);
    // Save aliases
    for (i = 0; i < MAX_ALIAS_SIZE - 1; i++)
    {
        service->default_alias[i] = alias[i];
        service->alias[i]         = alias[i];
        if (service->default_alias[i] == '\0')
            break;
    }
    service->default_alias[i] = '\0';
    service->alias[i]         = '\0';

    // Initialise the service revision to 0
    memset((void *)service->revision.unmap, 0, sizeof(revision_t));
    // Save firmware version
    for (i = 0; i < sizeof(revision_t); i++)
    {
        service->revision.unmap[i] = revision.unmap[i];
    }

    // initiate service statistics
    service->ll_service->ll_stat.max_retry = &service->statistics.max_retry;

    service_ctx.number++;
    LUOS_ASSERT(service_ctx.number <= MAX_SERVICE_NUMBER);
    return service;
}

/******************************************************************************
 * @brief Store alias name service in flash
 * @param service : Service to store
 * @param alias : Alias to store
 * @return SUCCEED : If the alias is correctly updated
 ******************************************************************************/
error_return_t Luos_UpdateAlias(service_t *service, const char *alias, uint16_t size)
{

    if ((size == 0) || (alias[0] == '\0'))
    {
        // This is a void alias just replace it with the default alias, write it
        memcpy(service->alias, service->default_alias, MAX_ALIAS_SIZE);
        return SUCCEED;
    }
    // Be sure to have a size including \0
    if (alias[size - 1] != '\0')
    {
        size++;
    }
    // Clip size
    if (size > MAX_ALIAS_SIZE)
    {
        size = MAX_ALIAS_SIZE;
    }
    char clean_alias[MAX_ALIAS_SIZE] = {0};
    // Replace any ' '' character by a '_' character, FAIL at any special character.
    for (uint8_t i = 0; i < size - 1; i++)
    {
        switch (alias[i])
        {
            case 'A' ... 'Z':
            case 'a' ... 'z':
            case '0' ... '9':
            case '_':
                // This is good
                clean_alias[i] = alias[i];
                break;
            case ' ':
                clean_alias[i] = '_';
                break;
            default:
                // This is a wrong character, don't do anything and return FAILED
                return FAILED;
                break;
        }
    }
    // We are ready to save this new alias, write it
    memcpy(service->alias, clean_alias, MAX_ALIAS_SIZE);
    return SUCCEED;
}
/******************************************************************************
 * @brief Clear list of service
 * @param none
 * @return none
 ******************************************************************************/
void Luos_ServicesClear(void)
{
    service_ctx.number = 0;
    Robus_ServicesClear();
}