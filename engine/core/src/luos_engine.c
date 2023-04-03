/******************************************************************************
 * @file luos
 * @brief User functionalities of the Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_engine.h"
#include <stdio.h>
#include <stdbool.h>
#include "msg_alloc.h"
#include "robus.h"
#include "luos_hal.h"
#include "bootloader_core.h"
#include "_timestamp.h"
#include "filter.h"
#include "context.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
revision_t luos_version = {.major = 3, .minor = 0, .build = 0};
package_t package_table[MAX_SERVICE_NUMBER];
uint16_t package_number = 0;
service_t service_table[MAX_SERVICE_NUMBER];
uint16_t service_number = 0;
service_t *detection_service;
uint8_t Flag_DetectServices = 0;

luos_stats_t luos_stats;
general_stats_t general_stats;

/*******************************************************************************
 * Function
 ******************************************************************************/
static error_return_t Luos_MsgHandler(service_t *service, msg_t *input);
static service_t *Luos_GetService(ll_service_t *ll_service);
static uint16_t Luos_GetServiceIndex(service_t *service);
static void Luos_TransmitLocalRoutingTable(service_t *service, msg_t *routeTB_msg);
static void Luos_AutoUpdateManager(void);
static error_return_t Luos_IsALuosCmd(service_t *service, uint8_t cmd, uint16_t size);
static inline void Luos_EmptyNode(void);
static inline void Luos_PackageInit(void);
static inline void Luos_PackageLoop(void);

/******************************************************************************
 * @brief Luos init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Init(void)
{
    service_number = 0;
    memset(&luos_stats.unmap[0], 0, sizeof(luos_stats_t));
    LuosHAL_Init();
    Robus_Init(&luos_stats.memory);

#ifdef WITH_BOOTLOADER
    if (APP_START_ADDRESS == (uint32_t)FLASH_BASE)
    {
        if (LuosHAL_GetMode() != JUMP_TO_APP_MODE)
        {
            LuosHAL_JumpToAddress(BOOT_START_ADDRESS);
        }
    }
#endif
}
/******************************************************************************
 * @brief Luos Loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Loop(void)
{
    static uint32_t last_loop_date;
    uint16_t remaining_msg_number   = 0;
    ll_service_t *oldest_ll_service = NULL;
    msg_t *returned_msg             = NULL;

#ifdef WITH_BOOTLOADER
    // After 3 Luos_Loop, consider this application as safe and write a flag to let the booloader know it can jump to the application safely.
    static uint8_t loop_count = 0;
    if (loop_count < 3)
    {
        loop_count++;
        if (loop_count == 3)
        {
            LuosHAL_SetMode((uint8_t)JUMP_TO_APP_MODE);
        }
    }
#endif

    // check loop call time stat
    if ((LuosHAL_GetSystick() - last_loop_date) > luos_stats.max_loop_time_ms)
    {
        luos_stats.max_loop_time_ms = LuosHAL_GetSystick() - last_loop_date;
    }
    if (MsgAlloc_IsReseted() == SUCCEED)
    {
        // We receive a reset detection
        // Reset services ID
        // Reinit ll_service id
        for (uint8_t i = 0; i < ctx.ll_service_number; i++)
        {
            ctx.ll_service_table[i].id = DEFAULTID;
        }
        // Reset the data reception context
        Luos_ReceiveData(NULL, NULL, NULL);
    }
    Robus_Loop();
    // look at all received messages
    LUOS_MUTEX_LOCK
    while (MsgAlloc_LookAtLuosTask(remaining_msg_number, &oldest_ll_service) != FAILED)
    {
        // There is a message available find the service linked to it
        service_t *service = Luos_GetService(oldest_ll_service);
        // check if this is a Luos Command
        uint8_t cmd   = 0;
        uint16_t size = 0;
        // There is a possibility to receive in IT a START_DETECTION so check task before doing any treatement
        if ((MsgAlloc_GetLuosTaskCmd(remaining_msg_number, &cmd) != SUCCEED) || (MsgAlloc_GetLuosTaskSize(remaining_msg_number, &size) != SUCCEED))
        {
            break;
        }
        // check if this msg cmd should be consumed by Luos_MsgHandler
        if (Luos_IsALuosCmd(service, cmd, size) == SUCCEED)
        {
            if (MsgAlloc_PullMsgFromLuosTask(remaining_msg_number, &returned_msg) == SUCCEED)
            {
                // be sure the content of this message need to be managed by Luos and do it if it is.
                if (Luos_MsgHandler((service_t *)service, returned_msg) == SUCCEED)
                {
                    // Luos CMD are generic for all services and have to be executed only once
                    // Clear all luos tasks related to this message (in case of multicast message)
                    MsgAlloc_ClearMsgFromLuosTasks(returned_msg);
                }
                else
                {
                    // Here we should not have polling services.
                    LUOS_ASSERT(service->service_cb != 0);
                    // This message is for the user, pass it to the user.
                    service->service_cb(service, returned_msg);
                }
            }
        }
        else
        {
            // This message is for a service
            // check if this service have a callback?
            if (service->service_cb != 0)
            {
                // This service have a callback pull the message
                if (MsgAlloc_PullMsgFromLuosTask(remaining_msg_number, &returned_msg) == SUCCEED)
                {
                    // This message is for the user, pass it to the user.
                    service->service_cb(service, returned_msg);
                }
            }
            else
            {
#ifdef BOOTLOADER
                if (MsgAlloc_PullMsgFromLuosTask(remaining_msg_number, &returned_msg) == SUCCEED)
                {
                    LuosBootloader_MsgHandler(returned_msg);
                }
#else
                remaining_msg_number++;
#endif
            }
        }
    }
    LUOS_MUTEX_UNLOCK
    // finish msg used
    MsgAlloc_UsedMsgEnd();
    // manage timed auto update
    Luos_AutoUpdateManager();
    // save loop date
    last_loop_date = LuosHAL_GetSystick();

    if (Flag_DetectServices == 1)
    {
        Flag_DetectServices++;
        RoutingTB_DetectServices(detection_service);
        Flag_DetectServices = 0;
    }
}
/******************************************************************************
 * @brief Luos clear statistic
 * @param None
 * @return None
 ******************************************************************************/
void Luos_ResetStatistic(void)
{
    memset(&luos_stats.unmap[0], 0, sizeof(luos_stats_t));
    for (uint16_t i = 0; i < service_number; i++)
    {
        service_table[i].statistics.max_retry = 0;
    }
}
/******************************************************************************
 * @brief Check if this command concern luos
 * @param service : Pointer to the service
 * @param cmd : The command value
 * @param size : The size of the received message
 * @return SUCCEED : If the command if for Luos, else FAILED
 ******************************************************************************/
static error_return_t Luos_IsALuosCmd(service_t *service, uint8_t cmd, uint16_t size)
{
    switch (cmd)
    {
        case WRITE_NODE_ID:
        case START_DETECTION:
            // ERROR
            LUOS_ASSERT(0);
            break;
        case ASSERT:
            if (service->service_cb != 0)
            {
                return SUCCEED;
            }
            else
            {
                return FAILED;
            }
            break;
        case LOCAL_RTB:
        case RTB:
        case WRITE_ALIAS:
        case UPDATE_PUB:
        case ASK_DETECTION:
            return SUCCEED;
            break;

        case REVISION:
        case LUOS_REVISION:
        case LUOS_STATISTICS:
            if (size == 0)
            {
                return SUCCEED;
            }
            break;
        case VERBOSE:
            if (size == 1)
            {
                return SUCCEED;
            }
            break;
        case BOOTLOADER_CMD:
            return SUCCEED;
            break;
        default:
            return FAILED;
            break;
    }
    return FAILED;
}
/******************************************************************************
 * @brief Handling message for Luos library
 * @param service
 * @param input : Message received
 * @return SUCCEED : If the messaged is consumed, else FAILED
 ******************************************************************************/
static error_return_t Luos_MsgHandler(service_t *service, msg_t *input)
{
    error_return_t consume = FAILED;
    msg_t output_msg;
    routing_table_t *route_tab = &RoutingTB_Get()[RoutingTB_GetLastEntry()];
    time_luos_t time;
    uint16_t base_id = 0;

    if (((input->header.target_mode == SERVICEIDACK) || (input->header.target_mode == SERVICEID)) && (input->header.target != service->ll_service->id))
    {
        return FAILED;
    }

    switch (input->header.cmd)
    {
        case ASSERT:
            // a service assert remove all services of the asserted node in routing table
            RoutingTB_RemoveNode(input->header.source);
            // This assert information could be usefull for services, do not remove it.
            consume = FAILED;
            break;
        case LOCAL_RTB:
            // Depending on the size of this message we have to make different operations
            // If size is 0 someone ask to get local_route table back
            // If size is 2 someone ask us to generate a local route table based on the given service ID then send local route table back.
            switch (input->header.size)
            {
                case 2:
                    // generate local ID
                    RoutingTB_Erase();
                    memcpy(&base_id, &input->data[0], sizeof(uint16_t));
                    if (base_id == 1)
                    {
                        // set service Id based on received data except for the detector one.
                        base_id   = 2;
                        int index = 0;
                        for (uint16_t i = 0; i < service_number; i++)
                        {
                            if (service_table[i].ll_service->id != 1)
                            {
                                service_table[i].ll_service->id = base_id + index;
                                index++;
                            }
                        }
                        Filter_AddServiceId(1, service_number);
                    }
                    else
                    {
                        // set service Id based on received data
                        for (uint16_t i = 0; i < service_number; i++)
                        {
                            service_table[i].ll_service->id = base_id + i;
                        }
                        Filter_AddServiceId(base_id, service_number);
                    }
                case 0:
                    // send back a local routing table
                    output_msg.header.cmd         = RTB;
                    output_msg.header.target_mode = SERVICEIDACK;
                    output_msg.header.target      = input->header.source;
                    Luos_TransmitLocalRoutingTable(service, &output_msg);
                    break;
            }
            consume = SUCCEED;
            break;
        case RTB:
            // Check routing table overflow
            LUOS_ASSERT(((uintptr_t)route_tab + input->header.size) <= ((uintptr_t)RoutingTB_Get() + (sizeof(routing_table_t) * MAX_RTB_ENTRY)));
            if (Luos_ReceiveData(service, input, (void *)route_tab) > 0)
            {
                // route table section reception complete
                RoutingTB_ComputeRoutingTableEntryNB();
                Luos_ResetStatistic();
            }
            consume = SUCCEED;
            break;

        case REVISION:
            if (input->header.size == 0)
            {
                msg_t output;
                output.header.cmd         = REVISION;
                output.header.target_mode = SERVICEID;
                memcpy(output.data, service->revision.unmap, sizeof(revision_t));
                output.header.size   = sizeof(revision_t);
                output.header.target = input->header.source;
                Luos_SendMsg(service, &output);
                consume = SUCCEED;
            }
            break;
        case LUOS_REVISION:
            if (input->header.size == 0)
            {
                msg_t output;
                output.header.cmd         = LUOS_REVISION;
                output.header.target_mode = SERVICEID;
                memcpy(output.data, &luos_version.unmap, sizeof(revision_t));
                output.header.size   = sizeof(revision_t);
                output.header.target = input->header.source;
                Luos_SendMsg(service, &output);
                consume = SUCCEED;
            }
            break;
        case ASK_DETECTION:
            if (input->header.size == 0)
            {
                if (Node_GetState() < LOCAL_DETECTION)
                {
                    Flag_DetectServices = 1;
                }
            }
            consume = SUCCEED;
            break;
        case LUOS_STATISTICS:
            if (input->header.size == 0)
            {
                msg_t output;
                output.header.cmd         = LUOS_STATISTICS;
                output.header.target_mode = SERVICEID;
                output.header.size        = sizeof(general_stats_t);
                output.header.target      = input->header.source;
                memcpy(&general_stats.node_stat, &luos_stats.unmap, sizeof(luos_stats_t));
                memcpy(&general_stats.service_stat, service->statistics.unmap, sizeof(service_stats_t));
                memcpy(output.data, &general_stats.unmap, sizeof(general_stats_t));
                Luos_SendMsg(service, &output);
                consume = SUCCEED;
            }
            break;
        case WRITE_ALIAS:
            // Save this alias into the service
            Luos_UpdateAlias(service, (const char *)input->data, input->header.size);
            // Send this message to user
            consume = FAILED;
            break;
        case UPDATE_PUB:
            // this service need to be auto updated
            TimeOD_TimeFromMsg(&time, input);
            service->auto_refresh.target      = input->header.source;
            service->auto_refresh.time_ms     = (uint16_t)TimeOD_TimeTo_ms(time);
            service->auto_refresh.last_update = LuosHAL_GetSystick();
            consume                           = SUCCEED;
            break;
        case VERBOSE:
            // this node should send messages to all the network
            Luos_SetVerboseMode(input->data[0]);
            consume = SUCCEED;
            break;
        case BOOTLOADER_CMD:
            // send data to the bootloader
            LuosBootloader_MsgHandler(input);
            consume = SUCCEED;
            break;
        default:
            break;
    }
    return consume;
}
/******************************************************************************
 * @brief Get pointer to a service in route table
 * @param ll_service : low level service
 * @return Service from list
 ******************************************************************************/
static service_t *Luos_GetService(ll_service_t *ll_service)
{
    for (uint16_t i = 0; i < service_number; i++)
    {
        if (ll_service == service_table[i].ll_service)
        {
            return &service_table[i];
        }
    }
    return 0;
}
/******************************************************************************
 * @brief Get this index of the service
 * @param Service
 * @return Service from list
 ******************************************************************************/
static uint16_t Luos_GetServiceIndex(service_t *service)
{
    for (uint16_t i = 0; i < service_number; i++)
    {
        if (service == &service_table[i])
        {
            return i;
        }
    }
    return 0xFFFF;
}
/******************************************************************************
 * @brief Transmit local RTB to network
 * @param service
 * @param routeTB_msg : Loca RTB message to transmit
 * @return none
 ******************************************************************************/
static void Luos_TransmitLocalRoutingTable(service_t *service, msg_t *routeTB_msg)
{
    uint16_t entry_nb = 0;
    routing_table_t local_routing_table[service_number + 1];

    // start by saving node entry
    RoutingTB_ConvertNodeToRoutingTable(&local_routing_table[entry_nb], Node_Get());
    entry_nb++;
    // save services entry
    for (uint16_t i = 0; i < service_number; i++)
    {
        RoutingTB_ConvertServiceToRoutingTable((routing_table_t *)&local_routing_table[entry_nb++], &service_table[i]);
    }
    Luos_SendData(service, routeTB_msg, (void *)local_routing_table, (entry_nb * sizeof(routing_table_t)));
}
/******************************************************************************
 * @brief Auto update publication for service
 * @param none
 * @return none
 ******************************************************************************/
static void Luos_AutoUpdateManager(void)
{
    // check all services timed_update_t contexts
    for (uint16_t i = 0; i < service_number; i++)
    {
        // check if services have an actual ID. If not, we are in detection mode and should reset the auto refresh
        if (service_table[i].ll_service->id == DEFAULTID)
        {
            // this service have not been detected or is in detection mode. remove auto_refresh parameters
            service_table[i].auto_refresh.target      = 0;
            service_table[i].auto_refresh.time_ms     = 0;
            service_table[i].auto_refresh.last_update = 0;
        }
        else
        {
            // check if there is a timed update setted and if it's time to update it.
            if (service_table[i].auto_refresh.time_ms)
            {
                if (service_table[i].ll_service->dead_service_spotted == service_table[i].auto_refresh.target)
                {
                    service_table[i].auto_refresh.target      = 0;
                    service_table[i].auto_refresh.time_ms     = 0;
                    service_table[i].auto_refresh.last_update = 0;
                    continue;
                }
                if ((LuosHAL_GetSystick() - service_table[i].auto_refresh.last_update) >= service_table[i].auto_refresh.time_ms)
                {
                    // This service need to send an update
                    // Create a fake message for it from the service asking for update
                    msg_t updt_msg;
                    updt_msg.header.config      = BASE_PROTOCOL;
                    updt_msg.header.target      = service_table[i].ll_service->id;
                    updt_msg.header.source      = service_table[i].auto_refresh.target;
                    updt_msg.header.target_mode = SERVICEIDACK;
                    updt_msg.header.cmd         = GET_CMD;
                    updt_msg.header.size        = 0;
                    if ((service_table[i].service_cb != 0))
                    {
                        service_table[i].service_cb(&service_table[i], &updt_msg);
                    }
                    else
                    {
                        if (Node_GetState() == DETECTION_OK)
                        {
                            // directly transmit the message in Localhost
                            Robus_SetTxTask(service_table[i].ll_service, &updt_msg);
                        }
                    }
                    service_table[i].auto_refresh.last_update = LuosHAL_GetSystick();
                }
            }
        }
    }
}
/******************************************************************************
 * @brief Clear list of service
 * @param none
 * @return none
 ******************************************************************************/
void Luos_ServicesClear(void)
{
    service_number = 0;
    Robus_ServicesClear();
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
    service_t *service  = &service_table[service_number];
    service->ll_service = Robus_ServiceCreate(type);

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
    service->node_statistics               = &luos_stats;
    service->ll_service->ll_stat.max_retry = &service->statistics.max_retry;

    service_number++;
    LUOS_ASSERT(service_number <= MAX_SERVICE_NUMBER);
    return service;
}
/******************************************************************************
 * @brief Send msg through network
 * @param Service : Who send
 * @param Message : To send
 * @return None
 ******************************************************************************/
error_return_t Luos_SendMsg(service_t *service, msg_t *msg)
{
    // set protocol version
    msg->header.config = BASE_PROTOCOL;

    if (service == 0)
    {
        // There is no service specified here, take the first one
        service = &service_table[0];
    }
    if ((service->ll_service->id == 0) && (msg->header.cmd >= LUOS_LAST_RESERVED_CMD))
    {
        // We are in detection mode and this command come from user
        // We can't send it
        return PROHIBITED;
    }
    return Robus_SendMsg(service->ll_service, msg);
}

/******************************************************************************
 * @brief Send msg through network
 * @param service : Who send
 * @param msg : Message to send
 * @param timestamp
 * @return SUCCEED : If the message is sent, else FAILED or PROHIBITED
 ******************************************************************************/
error_return_t Luos_SendTimestampMsg(service_t *service, msg_t *msg, time_luos_t timestamp)
{
    // set timestamp in message
    Timestamp_EncodeMsg(msg, timestamp);
    if (service == 0)
    {
        // There is no service specified here, take the first one
        service = &service_table[0];
    }
    if ((service->ll_service->id == 0) && (msg->header.cmd >= LUOS_LAST_RESERVED_CMD))
    {
        // We are in detection mode and this command come from user
        // We can't send it
        return PROHIBITED;
    }
    if (Robus_SendMsg(service->ll_service, msg) == FAILED)
    {
        return FAILED;
    }
    return SUCCEED;
}

/******************************************************************************
 * @brief Read last message from buffer for a specifig service
 * @param service : Who receives the message we are looking for
 * @param returned_msg : Oldest message of the service
 * @return SUCCEED : If the message is passed to the user, else FAILED
 ******************************************************************************/
error_return_t Luos_ReadMsg(service_t *service, msg_t **returned_msg)
{
    error_return_t error = SUCCEED;
    LUOS_MUTEX_LOCK
    while (error == SUCCEED)
    {
        error = MsgAlloc_PullMsg(service->ll_service, returned_msg);
        // check if the content of this message need to be managed by Luos and do it if it is.
        if (error == SUCCEED)
        {
            if (Luos_MsgHandler(service, *returned_msg) == FAILED)
            {
                LUOS_MUTEX_UNLOCK
                // This message is for the user, pass it to the user.
                return SUCCEED;
            }
        }
        MsgAlloc_ClearMsgFromLuosTasks(*returned_msg);
    }
    LUOS_MUTEX_UNLOCK
    return FAILED;
}
/******************************************************************************
 * @brief Read last msg from buffer from a specific id service
 * @param service : Who receive the message we are looking for
 * @param id : Who sent the message we are looking for
 * @param returned_msg : Oldest message of the service
 * @return FAILED if no message available
 ******************************************************************************/
error_return_t Luos_ReadFromService(service_t *service, short id, msg_t **returned_msg)
{
    uint16_t remaining_msg_number   = 0;
    ll_service_t *oldest_ll_service = NULL;
    error_return_t error            = SUCCEED;
    LUOS_MUTEX_LOCK
    while (MsgAlloc_LookAtLuosTask(remaining_msg_number, &oldest_ll_service) != FAILED)
    {
        // Check if this message is for us
        if (oldest_ll_service == service->ll_service)
        {
            // Check the source id
            uint16_t source = 0;
#ifdef LUOS_ASSERTION
            LUOS_ASSERT(MsgAlloc_GetLuosTaskSourceId(remaining_msg_number, &source) == SUCCEED);
#else
            MsgAlloc_GetLuosTaskSourceId(remaining_msg_number, &source);
#endif
            if (source == id)
            {
                // Source id of this message match, get it and treat it.
                error = MsgAlloc_PullMsgFromLuosTask(remaining_msg_number, returned_msg);
                // check if the content of this message need to be managed by Luos and do it if it is.
                if ((Luos_MsgHandler(service, *returned_msg) == FAILED) & (error == SUCCEED))
                {
                    // This message is for the user, pass it to the user.
                    LUOS_MUTEX_UNLOCK
                    return SUCCEED;
                }
                MsgAlloc_ClearMsgFromLuosTasks(*returned_msg);
            }
            else
            {
                remaining_msg_number++;
            }
        }
        else
        {
            remaining_msg_number++;
        }
    }
    LUOS_MUTEX_UNLOCK
    return FAILED;
}
/******************************************************************************
 * @brief Send large among of data and formating to send into multiple msg
 * @param service : Who send
 * @param message : Message to send
 * @param bin_data : Pointer to the message data table
 * @param size : Size of the data to transmit
 * @return None
 ******************************************************************************/
void Luos_SendData(service_t *service, msg_t *msg, void *bin_data, uint16_t size)
{
    // Compute number of message needed to send this data
    uint16_t msg_number = 1;
    uint16_t sent_size  = 0;
    if (size > MAX_DATA_MSG_SIZE)
    {
        msg_number = (size / MAX_DATA_MSG_SIZE);
        msg_number += (msg_number * MAX_DATA_MSG_SIZE < size);
    }

    // Send messages one by one
    for (uint16_t chunk = 0; chunk < msg_number; chunk++)
    {
        // Compute chunk size
        uint16_t chunk_size = 0;
        if ((size - sent_size) > MAX_DATA_MSG_SIZE)
        {
            chunk_size = MAX_DATA_MSG_SIZE;
        }
        else
        {
            chunk_size = size - sent_size;
        }

        // Copy data into message
        memcpy(msg->data, (uint8_t *)bin_data + sent_size, chunk_size);
        msg->header.size = size - sent_size;

        // Send message
        uint32_t tickstart = Luos_GetSystick();
        while (Luos_SendMsg(service, msg) == FAILED)
        {
            // No more memory space available
            // 500ms of timeout after start trying to load our data in memory. Perhaps the buffer is full of RX messages try to increate the buffer size.
            LUOS_ASSERT(((volatile uint32_t)Luos_GetSystick() - tickstart) < 500);
        }

        // Save current state
        sent_size = sent_size + chunk_size;
    }
}
/******************************************************************************
 * @brief Receive a multi msg data
 * @param service : who receive
 * @param msg : Message chunk received
 * @param bin_data : Pointer to data
 * @return Valid data received (negative values are errors)
 ******************************************************************************/
int Luos_ReceiveData(service_t *service, msg_t *msg, void *bin_data)
{
    // Manage buffer session (one per service)
    static uint32_t data_size[MAX_SERVICE_NUMBER]       = {0};
    static uint32_t total_data_size[MAX_SERVICE_NUMBER] = {0};
    static uint16_t last_msg_size                       = 0;

    // When this function receive a data from a NULL service it is an error and we should reinit the reception state
    if (service == NULL)
    {
        memset(data_size, 0, sizeof(data_size));
        memset(total_data_size, 0, sizeof(total_data_size));
        last_msg_size = 0;
        return -1;
    }

    LUOS_ASSERT(msg != 0);
    LUOS_ASSERT(bin_data != 0);

    uint16_t id = Luos_GetServiceIndex(service);
    // check good service index
    if (id == 0xFFFF)
    {
        return -1;
    }

    // store total size of a msg
    if (total_data_size[id] == 0)
    {
        total_data_size[id] = msg->header.size;
    }

    LUOS_ASSERT(msg->header.size <= total_data_size[id]);

    // check message integrity
    if ((last_msg_size > 0) && (last_msg_size - MAX_DATA_MSG_SIZE > msg->header.size))
    {
        // we miss a message (a part of the data),
        // reset session and return an error.
        data_size[id] = 0;
        last_msg_size = 0;
        return -1;
    }

    // Get chunk size
    uint16_t chunk_size = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
    {
        chunk_size = MAX_DATA_MSG_SIZE;
    }
    else
    {
        chunk_size = msg->header.size;
    }

    // Copy data into buffer
    memcpy(bin_data + data_size[id], msg->data, chunk_size);

    // Save buffer session
    data_size[id] = data_size[id] + chunk_size;
    last_msg_size = msg->header.size;

    // check
    LUOS_ASSERT(data_size[id] <= total_data_size[id]);

    // Check end of data
    if (msg->header.size <= MAX_DATA_MSG_SIZE)
    {
        // Data collection finished, reset buffer session state
        data_size[id]       = 0;
        last_msg_size       = 0;
        uint32_t backup     = total_data_size[id];
        total_data_size[id] = 0;
        return backup;
    }
    return 0;
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
 * @brief Return the number of messages available
 * @param None
 * @return The number of messages
 ******************************************************************************/
uint16_t Luos_NbrAvailableMsg(void)
{
    return MsgAlloc_LuosTasksNbr();
}
/******************************************************************************
 * @brief Check if all Tx message are complete
 * @param None
 * @return SUCCEED : If Tx message are complete
 ******************************************************************************/
error_return_t Luos_TxComplete(void)
{
    return MsgAlloc_TxAllComplete();
}
/******************************************************************************
 * @brief Register a new package
 * @param Init : Init function name
 * @param Loop : Loop function name
 * @return None
 ******************************************************************************/
void Luos_AddPackage(void (*Init)(void), void (*Loop)(void))
{
    package_table[package_number].Init = Init;
    package_table[package_number].Loop = Loop;

    package_number += 1;
}

/******************************************************************************
 * @brief Create a service to signal an empty node
 * @param None
 * @return None
 ******************************************************************************/
void Luos_EmptyNode(void)
{
    Luos_CreateService(0, VOID_TYPE, "empty_node", luos_version);
}

/******************************************************************************
 * @brief Run each package Init()
 * @param None
 * @return None
 ******************************************************************************/
void Luos_PackageInit(void)
{
    uint16_t package_index = 0;
    if (package_number)
    {
        while (package_index < package_number)
        {
            package_table[package_index].Init();
            package_index += 1;
        }
    }
    else
    {
        Luos_EmptyNode();
    }
}

/******************************************************************************
 * @brief Run each package Loop()
 * @param None
 * @return None
 ******************************************************************************/
void Luos_PackageLoop(void)
{
    uint16_t package_index = 0;
    while (package_index < package_number)
    {
        package_table[package_index].Loop();
        package_index += 1;
    }
}

/******************************************************************************
 * @brief Luos high level state machine
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Run(void)
{
    static bool node_run = false;

    if (!node_run)
    {
        Luos_Init();
#ifdef BOOTLOADER
        LuosBootloader_Init();
#else
        Luos_PackageInit();
#endif
        // go to run state after initialization
        node_run = true;
    }
    else
    {
        Luos_Loop();
#ifdef BOOTLOADER
        LuosBootloader_Loop();
#else
        Luos_PackageLoop();
#endif
    }
}
/******************************************************************************
 * @brief Set a local id
 * @param service : Service that we want to set id
 * @param id : Id value
 * @return None
 ******************************************************************************/
void Luos_SetID(service_t *service, uint16_t id)
{
    Filter_IdInit();
    // set id
    service->ll_service->id = 1;
    // change filter mask
    Filter_AddServiceId(id, service_number);
}
/******************************************************************************
 * @brief Demand a detection
 * @param service : Service that launched the detection
 * @return None
 ******************************************************************************/
void Luos_Detect(service_t *service)
{
    msg_t detect_msg;

    if (Node_GetState() < LOCAL_DETECTION)
    {
        // set the detection launcher id to 1
        Luos_SetID(service, 1);
        //  send ask detection message
        detection_service             = service;
        detect_msg.header.target_mode = SERVICEIDACK;
        detect_msg.header.cmd         = ASK_DETECTION;
        detect_msg.header.size        = 0;
        detect_msg.header.target      = 1;
        Luos_SendMsg(service, &detect_msg);
    }
}
