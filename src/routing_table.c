/******************************************************************************
 * @file routingTable
 * @brief routing table descrption function
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <routing_table.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "luos_hal.h"
#include "context.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ALIAS_SIZE 15
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
static bool RoutingTB_WaitRoutingTable(service_t *service, msg_t *intro_msg);

static void RoutingTB_Generate(service_t *service, uint16_t nb_node);
static void RoutingTB_Share(service_t *service, uint16_t nb_node);
static void RoutingTB_SendEndDetection(service_t *service);

// ************************ routing_table search tools ***************************

/******************************************************************************
 * @brief  Return an id from alias
 * @param pointer to alias
 * @return ID or Error
 ******************************************************************************/
uint16_t RoutingTB_IDFromAlias(char *alias)
{
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
 * @param id of service
 * @return NODEID or Error
 ******************************************************************************/
uint16_t RoutingTB_NodeIDFromID(uint16_t id)
{
    for (uint16_t i = RoutingTB_GetServiceIndex(id); i >= 0; i--)
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
 * @param id service look at
 * @return pointer to string or Error
 ******************************************************************************/
char *RoutingTB_AliasFromId(uint16_t id)
{
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
 * @brief  return bigest service ID in list
 * @param None
 * @return ID
 ******************************************************************************/
uint16_t RoutingTB_BigestID(void)
{
    uint16_t max_id = 0;
    for (uint16_t i = 0; i < last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == SERVICE)
        {
            if (routing_table[i].id > max_id)
            {
                max_id = routing_table[i].id;
            }
        }
    }
    return max_id;
}
/******************************************************************************
 * @brief  return bigest node ID in list
 * @param None
 * @return ID
 ******************************************************************************/
static uint16_t RoutingTB_BigestNodeID(void)
{
    uint16_t max_id = 0;
    for (uint16_t i = 0; i < last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == NODE)
        {
            if (routing_table[i].node_id > max_id)
            {
                max_id = routing_table[i].node_id;
            }
        }
    }
    return max_id;
}

/******************************************************************************
 * @brief  get Index of service on the routing table
 * @param routing table id
 * @return index
 ******************************************************************************/
uint16_t RoutingTB_GetServiceIndex(uint16_t id)
{
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
 * @brief compute entry number
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
 * @brief manage service name increment to never have same alias
 * @param alias to change
 * @param nb to add
 * @return None
 ******************************************************************************/
static void RoutingTB_AddNumToAlias(char *alias, uint8_t num)
{
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
    sprintf(alias, "%s%d", alias, num);
}
/******************************************************************************
 * @brief time out to receive en route table from
 * @param service receive
 * @param intro msg in route table
 * @return None
 ******************************************************************************/
static bool RoutingTB_WaitRoutingTable(service_t *service, msg_t *intro_msg)
{
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
 * @param service in node
 * @param node number on network
 * @return None
 ******************************************************************************/
static void RoutingTB_Generate(service_t *service, uint16_t nb_node)
{
    // Asks for introduction for every found node (even the one detecting).
    uint16_t try_nb       = 0;
    uint16_t last_node_id = RoutingTB_BigestNodeID();
    uint16_t last_cont_id = 0;
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
        last_cont_id          = RoutingTB_BigestID() + 1;
        memcpy(intro_msg.data, &last_cont_id, sizeof(uint16_t));
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
        uint16_t found_id = RoutingTB_IDFromAlias(RoutingTB_AliasFromId(id));
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
 * @param service who send
 * @param node number on network
 * @return None
 ******************************************************************************/
static void RoutingTB_Share(service_t *service, uint16_t nb_node)
{
    // send route table to each nodes. Routing tables are commonly usable for each services of a node.
    msg_t intro_msg;
    intro_msg.header.cmd         = RTB;
    intro_msg.header.target_mode = NODEIDACK;

    for (uint16_t i = 2; i <= nb_node; i++) // don't send to ourself
    {
        intro_msg.header.target = i;
        Luos_SendData(service, &intro_msg, routing_table, (last_routing_table_entry * sizeof(routing_table_t)));
    }
}

/******************************************************************************
 * @brief Send a message to indicate the end of the detection
 * @param service who send
 * @return None
 ******************************************************************************/
void RoutingTB_SendEndDetection(service_t *service)
{
    // send end detection message to each nodes
    msg_t msg;
    msg.header.target      = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd         = END_DETECTION;
    msg.header.size        = 0;
    Luos_SendMsg(service, &msg);
}

/******************************************************************************
 * @brief Detect all services and create a route table with it.
 * If multiple services have the same name it will be changed with a number in it
 * Automatically at the end this function create a list of sensors id
 * @param service who send
 * @return None
 ******************************************************************************/
void RoutingTB_DetectServices(service_t *service)
{
    // Desactivate verbose mode
    Luos_SetVerboseMode(false);
    // Starts the topology detection.
    uint16_t nb_node = Robus_TopologyDetection(service->ll_service);
    // Clear data reception state
    Luos_ReceiveData(NULL, NULL, NULL);
    // clear the routing table.
    RoutingTB_Erase();
    // Generate the routing_table
    RoutingTB_Generate(service, nb_node);
    // We have a complete routing table now share it with others.
    RoutingTB_Share(service, nb_node);
    // send a message to indicate the end of the detection
    RoutingTB_SendEndDetection(service);
    // clear statistic of node who start the detction
    Luos_ResetStatistic();
}
/******************************************************************************
 * @brief entry in routable node with associate service
 * @param route table
 * @param node structure
 * @return None
 ******************************************************************************/
void RoutingTB_ConvertNodeToRoutingTable(routing_table_t *entry, node_t *node)
{
    // Check if the NBR_PORT config is too high to fit into routing table.
    LUOS_ASSERT(sizeof(node_t) <= (sizeof(routing_table_t) - 1));
    memset(entry, 0, sizeof(routing_table_t));
    entry->mode = NODE;
    memcpy(entry->unmap_data, node->unmap, sizeof(node_t));
}
/******************************************************************************
 * @brief entry in routable service associate to a node
 * @param route table
 * @param service in node
 * @return None
 ******************************************************************************/
void RoutingTB_ConvertServiceToRoutingTable(routing_table_t *entry, service_t *service)
{
    entry->type = service->ll_service->type;
    entry->id   = service->ll_service->id;
    entry->mode = SERVICE;
    for (uint8_t i = 0; i < MAX_ALIAS_SIZE; i++)
    {
        entry->alias[i] = service->alias[i];
    }
}
/******************************************************************************
 * @brief remove an entire node
 * @param route table
 * @return None
 ******************************************************************************/
void RoutingTB_RemoveNode(uint16_t nodeid)
{
    // instead of removing a node just remove all the service in it to make it unusable
    // We could add a param (CONTROL for example) to declare the node as STOP
    // find the node
    for (uint16_t i = 0; i < last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == NODE)
        {
            if (routing_table[i].node_id == nodeid)
            {
                i++;
                // We find our node remove all services
                while (routing_table[i].mode == SERVICE)
                {
                    RoutingTB_RemoveOnRoutingTable(routing_table[i].id);
                }
                return;
            }
        }
    }
}
/******************************************************************************
 * @brief remove an entry from routing_table
 * @param id of service
 * @return None
 ******************************************************************************/
void RoutingTB_RemoveOnRoutingTable(uint16_t id)
{
    // find the service
    for (uint16_t i = 0; i < last_routing_table_entry; i++)
    {
        if ((routing_table[i].mode == SERVICE) && (routing_table[i].id == id))
        {
            LUOS_ASSERT(i < last_routing_table_entry);
            memcpy(&routing_table[i], &routing_table[i + 1], sizeof(routing_table_t) * (last_routing_table_entry - (i + 1)));
            last_routing_table_entry--;
            memset(&routing_table[last_routing_table_entry], 0, sizeof(routing_table_t));
            return;
        }
    }
}
/******************************************************************************
 * @brief eras erouting_table
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
 * @brief get routing_table
 * @param None
 * @return route table
 ******************************************************************************/
routing_table_t *RoutingTB_Get(void)
{
    return routing_table;
}
/******************************************************************************
 * @brief return the last ID registered into the routing_table
 * @param None
 * @return last service ID
 ******************************************************************************/
uint16_t RoutingTB_GetLastService(void)
{
    return (uint16_t)last_service;
}
/******************************************************************************
 * @brief return the last ID registered into the routing_table
 * @param index of service
 * @return Last entry
 ******************************************************************************/
uint16_t RoutingTB_GetLastEntry(void)
{
    return (uint16_t)last_routing_table_entry;
}

/******************************** Result Table ********************************/
uint8_t RTFilter_InitCheck(search_result_t *result)
{
    // check if we fund the address of the result in routing table
    if ((result->result_table[0] >= &routing_table[0]) && (result->result_table[0] <= &routing_table[last_routing_table_entry - 1]))
    {
        return true;
    }
    return false;
}
/******************************************************************************
 * @brief Initialize the Result table pointers
 * @param index of service
 * @return Last entry
 ******************************************************************************/
search_result_t *RTFilter_Reset(search_result_t *result)
{
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
 * @brief find the service with a specific id
 * @param previous result research structure
 * @param id that we want to find
 * @return new result research structure with the entry of the demanded id
 ******************************************************************************/
search_result_t *RTFilter_ID(search_result_t *result, uint16_t id)
{
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == 0)
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
 * @brief search all the services with the same type
 * @param previous result research structure
 * @param type that we want to find
 * @return new result research structure
 ******************************************************************************/
search_result_t *RTFilter_Type(search_result_t *result, luos_type_t type)
{
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == 0)
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
 * @brief search all the services of the same node
 * @param previous result research structure
 * @param node_id of the node that we want to find
 * @return new result research structure
 ******************************************************************************/
search_result_t *RTFilter_Node(search_result_t *result, uint16_t node_id)
{
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == 0)
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

search_result_t *RTFilter_Alias(search_result_t *result, char *alias)
{
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == 0)
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
 * @brief find the service info with a service pointer
 * @param previous result research structure
 * @param service pointer to the service
 * @return new result research structure with the entry of the demanded service
 ******************************************************************************/
search_result_t *RTFilter_Service(search_result_t *result, service_t *service)
{
    uint8_t entry_nbr = 0;
    // Check result pointer
    LUOS_ASSERT(result != 0);
    // if we the result is not initialized return 0
    if (RTFilter_InitCheck(result) == 0)
    {
        result->result_nbr = 0;
    }
    LUOS_ASSERT(service != 0);
    while (entry_nbr < result->result_nbr)
    {
        // find a service with the wanted type
        if (result->result_table[entry_nbr]->id != service->ll_service->id)
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
