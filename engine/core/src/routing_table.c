/******************************************************************************
 * @file routingTable
 * @brief routing table description function
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "routing_table.h"
#include "luos_hal.h"
#include "luos_engine.h"
#include "struct_engine.h"
#include "luos_io.h"
#include "service.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
routing_table_t routing_table[MAX_RTB_ENTRY];
volatile uint16_t last_service             = 0;
volatile uint16_t last_routing_table_entry = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void RoutingTB_AddNumToAlias(char *alias, uint8_t num);
uint16_t RoutingTB_IDFromAlias(char *alias);
char *RoutingTB_AliasFromId(uint16_t id);
static uint16_t RoutingTB_BigestNodeID(void);
uint16_t RoutingTB_GetServiceIndex(uint16_t id);
bool RoutingTB_WaitRoutingTable(service_t *service, msg_t *intro_msg);

static void RoutingTB_Generate(service_t *service, uint16_t nb_node);
static void RoutingTB_Share(service_t *service, uint16_t nb_node);
static void RoutingTB_SendEndDetection(service_t *service);

// ************************ routing_table search tools ***************************

/******************************************************************************
 * @brief  Return an id from alias
 * @param alias : Pointer to alias
 * @return ID, or 0 if error
 ******************************************************************************/
uint16_t RoutingTB_IDFromAlias(char *alias)
{
    LUOS_ASSERT(alias);
    if (*alias != -1)
    {
        for (int i = 0; i <= last_routing_table_entry; i++)
        {
            if (routing_table[i].mode == SERVICE)
            {
                if (strcmp(routing_table[i].alias, alias) == 0)
                {
                    return routing_table[i].id;
                }
            }
        }
    }
    return 0;
}

/******************************************************************************
 * @brief  Return a Nodeid from service id
 * @param id : Id of service
 * @return NODEID, or 0 if error
 ******************************************************************************/
uint16_t RoutingTB_NodeIDFromID(uint16_t id)
{
    LUOS_ASSERT(id != 0); // Services can't have id 0.
    // In RTB, node are always before service, so we can search backward and start just before the service index.
    for (int16_t i = RoutingTB_GetServiceIndex(id) - 1; i >= 0; i--)
    {
        if (routing_table[i].mode == NODE)
        {
            return routing_table[i].node_id;
        }
    }
    return 0;
}

/******************************************************************************
 * @brief  Return service Alias from ID
 * @param id : Id service look at
 * @return Pointer to string, , or 0 if error
 ******************************************************************************/
char *RoutingTB_AliasFromId(uint16_t id)
{
    LUOS_ASSERT(id != 0); // Services can't have id 0.
    for (int i = 0; i <= last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == SERVICE)
        {
            if (id == routing_table[i].id)
            {
                return routing_table[i].alias;
            }
        }
    }
    return (char *)0;
}

/******************************************************************************
 * @brief  Return bigest service ID in list
 * @param None
 * @return Bigest service ID
 ******************************************************************************/
uint16_t RoutingTB_BigestID(void)
{
    return (uint16_t)last_service;
}

/******************************************************************************
 * @brief  Return bigest node ID in list
 * @param None
 * @return Bigest node ID
 ******************************************************************************/
static uint16_t RoutingTB_BigestNodeID(void)
{
    for (int32_t i = last_routing_table_entry; i >= 0; i--)
    {
        if (routing_table[i].mode == NODE)
        {
            return routing_table[i].node_id;
        }
    }
    return 0;
}

/******************************************************************************
 * @brief  Get Index of service on the routing table
 * @param id : service id
 * @return Routing table Index
 ******************************************************************************/
uint16_t RoutingTB_GetServiceIndex(uint16_t id)
{
    LUOS_ASSERT(id != 0); // Services can't have id 0.
    for (uint8_t i = 0; i < last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == SERVICE && routing_table[i].id == id)
        {
            return i;
        }
    }
    return 0;
}

// ********************* routing_table management tools ************************

/******************************************************************************
 * @brief Compute RTB entry number
 * @param None
 * @return None
 ******************************************************************************/
void RoutingTB_ComputeRoutingTableEntryNB(void)
{
    for (uint16_t i = 0; i < MAX_RTB_ENTRY; i++)
    {
        if (routing_table[i].mode == SERVICE)
        {
            last_service = routing_table[i].id;
        }
        if (routing_table[i].mode == CLEAR)
        {
            last_routing_table_entry = i;
            return;
        }
    }
    // Routing table space is full.
    last_routing_table_entry = MAX_RTB_ENTRY - 1;
}

/******************************************************************************
 * @brief Manage service name increment to never have same alias
 * @param alias : Alias to change
 * @param num : Number to add
 * @return None
 ******************************************************************************/
static void RoutingTB_AddNumToAlias(char *alias, uint8_t num)
{
    LUOS_ASSERT(alias);
    uint8_t intsize = 1;
    if (num > 9)
    {
        // The string size of num is 2
        intsize = 2;
    }
    if (num > 99) // only 2 digit are alowed when add alias number
    {
        // This is probably a mistake, put an error into the alias
        memset(alias, 0, ALIAS_SIZE);
        memcpy(alias, "error", strlen("error"));
        return;
    }
    // Change size to fit into 15 characters
    if (strlen(alias) > (ALIAS_SIZE - intsize))
    {
        alias[(ALIAS_SIZE - intsize)] = '\0';
    }
    else
    {
        alias[strlen(alias)] = '\0';
    }
    // Add a number at the end of the alias
    char *alias_copy = alias;
    sprintf(alias, "%s%d", alias_copy, num);
}

/******************************************************************************
 * @brief Time out to receive en route table from
 * @param service : Service receive
 * @param intro_msg : into route table message
 * @return None
 ******************************************************************************/
bool RoutingTB_WaitRoutingTable(service_t *service, msg_t *intro_msg)
{
    LUOS_ASSERT((service != 0) && (intro_msg != 0));
    const uint8_t timeout    = 15; // timeout in ms
    const uint16_t entry_bkp = last_routing_table_entry;
    Luos_SendMsg(service, intro_msg);
    uint32_t timestamp = LuosHAL_GetSystick();
    while ((LuosHAL_GetSystick() - timestamp) < timeout)
    {
        // If this request is for a service in this board allow him to respond.
        Luos_Loop();
        if (entry_bkp != last_routing_table_entry)
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * @brief Generate Complete route table with local route table receive
 * @param service : Service in node
 * @param nb_node : Node number on network
 * @return None
 ******************************************************************************/
static void RoutingTB_Generate(service_t *service, uint16_t nb_node)
{
    LUOS_ASSERT(service);
    // Asks for introduction for every found node (even the one detecting).
    uint16_t try_nb          = 0;
    uint16_t last_node_id    = RoutingTB_BigestNodeID();
    uint16_t last_service_id = 0;
    msg_t intro_msg;
    while ((last_node_id < nb_node) && (try_nb < nb_node))
    {
        try_nb++;
        intro_msg.header.cmd         = LOCAL_RTB;
        intro_msg.header.target_mode = NODEIDACK;
        // Target next unknown node
        intro_msg.header.target = last_node_id + 1;
        // set the first service id it can use
        intro_msg.header.size = 2;
        last_service_id       = RoutingTB_BigestID() + 1;
        memcpy(intro_msg.data, &last_service_id, sizeof(uint16_t));
        // Ask to introduce and wait for a reply
        if (!RoutingTB_WaitRoutingTable(service, &intro_msg))
        {
            // We don't get the answer
            nb_node = last_node_id;
            break;
        }
        last_node_id = RoutingTB_BigestNodeID();
    }
    // Check Alias duplication.
    uint16_t nb_service = RoutingTB_BigestID();
    for (uint16_t id = 1; id <= nb_service; id++)
    {
        int32_t found_id = RoutingTB_IDFromAlias(RoutingTB_AliasFromId(id));
        if ((found_id != id) & (found_id != -1))
        {
            // The found_id don't match with the actual ID of the service because the alias already exist
            // Find the new alias to give him
            uint8_t annotation              = 1;
            char base_alias[MAX_ALIAS_SIZE] = {0};
            memcpy(base_alias, RoutingTB_AliasFromId(id), MAX_ALIAS_SIZE);
            // Add a number after alias in routing table
            RoutingTB_AddNumToAlias(RoutingTB_AliasFromId(id), annotation++);
            // check another time if this alias is already used
            while (RoutingTB_IDFromAlias(RoutingTB_AliasFromId(id)) != id)
            {
                // This alias is already used.
                // Remove the number previously setuped by overwriting it with the base_alias
                memcpy(RoutingTB_AliasFromId(id), base_alias, MAX_ALIAS_SIZE);
                RoutingTB_AddNumToAlias(RoutingTB_AliasFromId(id), annotation++);
            }
        }
    }
}

/******************************************************************************
 * @brief Send the complete route table to each node on the network
 * @param service : Service who send
 * @param nb_node : number of nodes on network
 * @return None
 ******************************************************************************/
static void RoutingTB_Share(service_t *service, uint16_t nb_node)
{
    LUOS_ASSERT(service);
    // Make sure that the detection is not interrupted
    if (Node_GetState() == EXTERNAL_DETECTION)
    {
        return;
    }
    // send route table to each nodes. Routing tables are commonly usable for each services of a node.
    msg_t intro_msg;
    intro_msg.header.cmd         = RTB;
    intro_msg.header.target_mode = NODEIDACK;

    for (uint16_t i = 2; i <= nb_node; i++) // don't send to ourself
    {
        intro_msg.header.target = i;

        uint16_t node_idx;

        for (node_idx = i; node_idx < last_routing_table_entry; node_idx++)
        {
            if ((routing_table[node_idx].mode == NODE) && (routing_table[node_idx].node_id == i))
            {
                break;
            }
        }
        if ((routing_table[node_idx].node_info & (1 << 0)) == 0)
        {
            Luos_SendData(service, &intro_msg, routing_table, (last_routing_table_entry * sizeof(routing_table_t)));
        }
    }
}

/******************************************************************************
 * @brief Send a message to indicate the end of the detection
 * @param service : Service who send
 * @return None
 ******************************************************************************/
void RoutingTB_SendEndDetection(service_t *service)
{
    LUOS_ASSERT(service);
    // Make sure that the detection is not interrupted
    if (Node_GetState() == EXTERNAL_DETECTION)
    {
        return;
    }
    // send end detection message to each nodes
    msg_t msg;
    msg.header.target      = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd         = END_DETECTION;
    msg.header.size        = 0;
    while (Luos_SendMsg(service, &msg) != SUCCEED)
        ;
}

/******************************************************************************
 * @brief Detect all services and create a route table with it.
 * If multiple services have the same name it will be changed with a number in it
 * Automatically at the end this function create a list of sensors id
 * @param service : Service who send
 * @return None
 ******************************************************************************/
void RoutingTB_DetectServices(service_t *service)
{
    LUOS_ASSERT(service);
    // Starts the topology detection.
    uint16_t nb_node = LuosIO_TopologyDetection(service);
    // Clear data reception state
    Luos_ReceiveData(NULL, NULL, NULL);
    // Clear the routing table.
    RoutingTB_Erase();
    // Generate the routing_table
    RoutingTB_Generate(service, nb_node);
    // We have a complete routing table now share it with others.
    RoutingTB_Share(service, nb_node);
    // Send a message to indicate the end of the detection
    RoutingTB_SendEndDetection(service);
    // Clear statistic of node who start the detction
    Luos_ResetStatistic();
}

/******************************************************************************
 * @brief Entry in routable node with associate service
 * @param entry : Route table
 * @param node : Node structure
 * @return None
 ******************************************************************************/
void RoutingTB_ConvertNodeToRoutingTable(routing_table_t *entry, node_t *node)
{
    LUOS_ASSERT((node != NULL) && (entry != NULL));
    // Check if the NBR_PORT config is too high to fit into routing table.
    LUOS_ASSERT(sizeof(node_t) <= (sizeof(routing_table_t) - 1));
    memset(entry, 0, sizeof(routing_table_t));
    entry->mode = NODE;
    memcpy(entry->unmap_data, node->unmap, sizeof(node_t));
}

/******************************************************************************
 * @brief Entry in routable service associate to a node
 * @param entry : Route table
 * @param service : Service in node
 * @return None
 ******************************************************************************/
void RoutingTB_ConvertServiceToRoutingTable(routing_table_t *entry, service_t *service)
{
    LUOS_ASSERT((service != NULL) && (entry != NULL));
    entry->type = service->type;
    entry->id   = service->id;
    entry->mode = SERVICE;
    for (uint8_t i = 0; i < MAX_ALIAS_SIZE; i++)
    {
        entry->alias[i] = service->alias[i];
    }
}

/******************************************************************************
 * @brief Remove an entire node
 * @param nodeid : Node id to remove from RTB
 * @return None
 ******************************************************************************/
void RoutingTB_RemoveNode(uint16_t nodeid)
{
    LUOS_ASSERT(nodeid != 0);
    // Instead of removing a node just remove all the service in it to make it unusable
    // We could add a param (CONTROL for example) to declare the node as STOP
    // find the node
    for (uint16_t i = 0; i < last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == NODE)
        {
            if (routing_table[i].node_id == nodeid)
            {
                i++;
                // We foundd our node remove all services in it
                while (routing_table[i].mode == SERVICE)
                {
                    RoutingTB_RemoveService(routing_table[i].id);
                }
                return;
            }
        }
    }
}

/******************************************************************************
 * @brief Remove an entry from routing_table
 * @param id : Id of service
 * @return None
 ******************************************************************************/
void RoutingTB_RemoveService(uint16_t serviceid)
{
    LUOS_ASSERT(serviceid != 0);
    Service_RmAutoUpdateTarget(serviceid);
    // Find the service
    for (uint16_t i = 0; i < last_routing_table_entry; i++)
    {
        if ((routing_table[i].mode == SERVICE) && (routing_table[i].id == serviceid))
        {
            LUOS_ASSERT(i < last_routing_table_entry);
            memcpy(&routing_table[i], &routing_table[i + 1], sizeof(routing_table_t) * (last_routing_table_entry - (i + 1)));
            last_routing_table_entry--;
            memset(&routing_table[last_routing_table_entry], 0, sizeof(routing_table_t));
            if (serviceid == last_service)
            {
                last_service = 0;
                for (uint16_t i = last_routing_table_entry; i > 0; i--)
                {
                    if (routing_table[i].mode == SERVICE)
                    {
                        last_service = routing_table[i].id;
                        break;
                    }
                }
            }
            return;
        }
    }
}

/******************************************************************************
 * @brief Erase routing_table
 * @param None
 * @return None
 ******************************************************************************/
void RoutingTB_Erase(void)
{
    memset(routing_table, 0, sizeof(routing_table));
    last_service             = 0;
    last_routing_table_entry = 0;
}

/******************************************************************************
 * @brief Get routing_table
 * @param None
 * @return route table
 ******************************************************************************/
routing_table_t *RoutingTB_Get(void)
{
    return routing_table;
}

/******************************************************************************
 * @brief Return the last ID registered into the routing_table
 * @param None
 * @return Last Id
 ******************************************************************************/
uint16_t RoutingTB_GetLastEntry(void)
{
    return (uint16_t)last_routing_table_entry;
}

/******************************** Result Table ********************************/
/******************************************************************************
 * @brief Check if result is in routing table
 * @param result : Pointer to search result structure
 * @return SUCCEED : If the result address is available, else FAILED
 ******************************************************************************/
error_return_t RTFilter_InitCheck(search_result_t *result)
{
    LUOS_ASSERT(result != NULL);
    // check if we fund the address of the result in routing table
    if ((result->result_table[0] >= &routing_table[0]) && (result->result_table[0] <= &routing_table[last_routing_table_entry - 1]))
    {
        return SUCCEED;
    }
    return FAILED;
}

/******************************************************************************
 * @brief Initialize the Result table pointers
 * @param result : Pointer to result table
 * @return Last entry
 ******************************************************************************/
search_result_t *RTFilter_Reset(search_result_t *result)
{
    LUOS_ASSERT(result != NULL);
    // the initialization is to keep a pointer to all the  servicesentries of the routing table
    result->result_nbr = 0;
    for (uint8_t i = 0; i < last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == SERVICE)
        {
            result->result_table[result->result_nbr] = &routing_table[i];
            result->result_nbr++;
        }
    }
    return result;
}

/******************************************************************************
 * @brief Find the service with a specific Id
 * @param result : Pointer to previous result research structure
 * @param id : Id that we want to find
 * @return new result research structure with the entry of the demanded id
 ******************************************************************************/
search_result_t *RTFilter_ID(search_result_t *result, uint16_t id)
{
    LUOS_ASSERT((result != NULL) && (id != 0));
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == FAILED)
    {
        result->result_nbr = 0;
    }
    while (entry_nbr < result->result_nbr)
    {
        // find a service with the wanted type
        if (result->result_table[entry_nbr]->id != id)
        {
            // if we find an other id, erase it from the research table
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr));
            result->result_nbr--;
        }
        else
        {
            entry_nbr++;
        }
    }
    // return a pointer to the search structure
    return (result);
}

/******************************************************************************
 * @brief Search all the services with the same type
 * @param result : Pointer to previous result research structure
 * @param type : Type that we want to find
 * @return New result research structure
 ******************************************************************************/
search_result_t *RTFilter_Type(search_result_t *result, luos_type_t type)
{
    LUOS_ASSERT((result != NULL));
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == FAILED)
    {
        result->result_nbr = 0;
    }
    while (entry_nbr < result->result_nbr)
    {
        // find a service with the wanted type
        if (result->result_table[entry_nbr]->type != type)
        {
            // if we find an other type, erase it from the research table
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr));
            result->result_nbr--;
        }
        else
        {
            entry_nbr++;
        }
    }
    // return a pointer to the search structure
    return (result);
}

/******************************************************************************
 * @brief Search all the services of the same node
 * @param result : Pointer to previous result research structure
 * @param node_id : Node Id of the node that we want to find
 * @return New result research structure
 ******************************************************************************/
search_result_t *RTFilter_Node(search_result_t *result, uint16_t node_id)
{
    LUOS_ASSERT((result != NULL) && (node_id != 0));
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == FAILED)
    {
        result->result_nbr = 0;
    }
    // search all the entries of the research table
    while (entry_nbr < result->result_nbr)
    {
        // find a service with the wanted node_id
        if (RoutingTB_NodeIDFromID(result->result_table[entry_nbr]->id) != node_id)
        {
            // if we find an other node_id, erase it from the research table
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr));
            result->result_nbr--;
        }
        else
        {
            entry_nbr++;
        }
    }
    // return a pointer to the search structure
    return (result);
}

/******************************************************************************
 * @brief Search all the services with the same alias
 * @param result : Pointer to previous result research structure
 * @param alias : Alias that we want to find
 * @return New result research structure
 ******************************************************************************/
search_result_t *RTFilter_Alias(search_result_t *result, char *alias)
{
    LUOS_ASSERT((result != NULL) && (alias != 0));
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == FAILED)
    {
        result->result_nbr = 0;
    }
    // search all the entries of the research table
    while (entry_nbr < result->result_nbr)
    {
        // find a service with the wanted node_id
        if (strstr(result->result_table[entry_nbr]->alias, alias) == 0)
        {
            // if we find an other node_id, erase it from the research table
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr));
            result->result_nbr--;
        }
        else
        {
            entry_nbr++;
        }
    }
    // return a pointer to the search structure
    return (result);
}

/******************************************************************************
 * @brief Find the service info with a service pointer
 * @param result : Pointer to previous result research structure
 * @param service : Service pointer to the service
 * @return new result research structure with the entry of the demanded service
 ******************************************************************************/
search_result_t *RTFilter_Service(search_result_t *result, service_t *service)
{
    LUOS_ASSERT((result != NULL) && (service != 0));
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == FAILED)
    {
        result->result_nbr = 0;
    }
    LUOS_ASSERT(service != 0);
    while (entry_nbr < result->result_nbr)
    {
        // find a service with the wanted type
        if (result->result_table[entry_nbr]->id != service->id)
        {
            // if we find an other id, erase it from the research table
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr));
            result->result_nbr--;
        }
        else
        {
            entry_nbr++;
        }
    }
    // return a pointer to the search structure
    return (result);
}
