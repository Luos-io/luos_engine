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
#include "_routing_table.h"
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

static int RoutingTB_Generate(service_t *service, uint16_t nb_node, connection_t *connection_table);
static bool RoutingTB_Share(service_t *service, uint16_t nb_node);
static void RoutingTB_SendEndDetection(service_t *service);
static void RoutingTB_ComputeNodeIndexes(service_t *service, uint16_t node_index, uint16_t nb_node, connection_t *connection_table);
static void RoutingTB_SendNodeIndexes(service_t *service, uint16_t node_id, uint8_t phy_index, uint8_t *node_indexes);
static void RoutingTB_ComputeServiceIndexes(service_t *service, uint16_t rtb_index);
static void RoutingTB_SendServiceIndexes(service_t *service, uint16_t node_id, uint8_t phy_index, uint8_t *service_indexes);

// ************************ routing_table search tools ***************************

/******************************************************************************
 * @brief  Return an id from alias
 * @param alias : Pointer to alias
 * @return ID, or 0 if error
 ******************************************************************************/
uint16_t RoutingTB_IDFromAlias(char *alias)
{
    LUOS_ASSERT(alias);
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
        if ((routing_table[i].mode == SERVICE) && (last_service < routing_table[i].id))
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
        // The string size of num have a size of 2
        intsize = 2;
    }
    LUOS_ASSERT(num < 100); // only 2 digit are alowed when add alias number
    // Change size to fit into 15 characters
    if (strlen(alias) > (MAX_ALIAS_SIZE - 1 - intsize))
    {
        alias[(MAX_ALIAS_SIZE - 1 - intsize)] = '\0';
    }
    // Add a number at the end of the alias
    char *end_pointer = &alias[strlen(alias)];
    sprintf(end_pointer, "%d", num);
}

/******************************************************************************
 * @brief Generate Complete route table with local route table receive
 * @param service : Service in node
 * @param nb_node : Node number on network
 * @return None
 ******************************************************************************/
static int RoutingTB_Generate(service_t *service, uint16_t nb_node, connection_t *connection_table)
{
    LUOS_ASSERT(service);
    // Asks for introduction for every found node (even the one detecting).
    uint16_t try_nb              = 0;
    static uint16_t last_node_id = 0;
    uint16_t last_service_id     = 0;
    msg_t intro_msg;
    uint16_t nb_service;
    static uint16_t rtb_next_node_index;
    static uint8_t detect_state_machine = 0;
    static uint16_t entry_bkp;
    static uint32_t timestamp;

    switch (detect_state_machine)
    {
        case 0:
            if ((last_node_id >= nb_node) || (try_nb >= nb_node))
            {
                // Go to check alias duplication step
                detect_state_machine = 2;
                return 0;
            }
            // First compute the node indexes for this node and send it to it.
            RoutingTB_ComputeNodeIndexes(service, last_node_id, nb_node, connection_table);
            try_nb++;
            intro_msg.header.cmd         = LOCAL_RTB;
            intro_msg.header.target_mode = NODEIDACK;
            // Target next unknown node
            intro_msg.header.target = last_node_id + 1;
            // Set the first service id it can use
            intro_msg.header.size = 2;
            last_service_id       = RoutingTB_BigestID() + 1;
            memcpy(intro_msg.data, &last_service_id, sizeof(uint16_t));
            // Save the current last routing table entry allowing us to easily write the connection informations later
            rtb_next_node_index = RoutingTB_GetLastEntry();
            entry_bkp           = last_routing_table_entry;
            Luos_SendMsg(service, &intro_msg);
            timestamp = LuosHAL_GetSystick();
            detect_state_machine++;
        case 1:
            if ((LuosHAL_GetSystick() - timestamp) >= 2000)
            {
                // Time out is reached
                // We don't get the answer
                nb_node = last_node_id;
                // Go directly to Alias duplication check
                detect_state_machine = 2;
                return 0;
            }
            // If this request is for a service in this board allow him to respond.
            if (entry_bkp == last_routing_table_entry)
            {
                // We don't get the answer yet
                return 0;
            }
            // We get the answer
            // The node answer don't include connection because the node don't know it yet
            // Add this information to the routing table
            LUOS_ASSERT(routing_table[rtb_next_node_index].mode == NODE);
            routing_table[rtb_next_node_index].connection = connection_table[last_node_id];
            last_node_id                                  = RoutingTB_BigestNodeID();
            // Go back to step 0 to ask the routing table of the next node
            detect_state_machine = 0;
            return 0;
            break;
        case 2:
            // Check Alias duplication.
            nb_service = RoutingTB_BigestID();
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
            detect_state_machine = 0;
            last_node_id         = 0;
            return 1;
            break;
        default:
            LUOS_ASSERT(0);
            break;
    }
    LUOS_ASSERT(0);
    return -1;
}

/******************************************************************************
 * @brief Send the complete route table and all indexes to each node on the network
 * @param service : Service who send
 * @param nb_node : number of nodes on network
 * @return None
 ******************************************************************************/
static bool RoutingTB_Share(service_t *service, uint16_t nb_node)
{
    static uint8_t detect_state_machine = 0;
    LUOS_ASSERT(service);
    // Make sure that the detection is not interrupted
    if (Node_GetState() == EXTERNAL_DETECTION)
    {
        return true;
    }

    if (detect_state_machine == 0)
    {
        // Compute local indexes
        RoutingTB_ComputeServiceIndexes(service, 0);
        detect_state_machine++;
        return false;
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
        RoutingTB_ComputeServiceIndexes(service, node_idx);
        // Check if this node need to get the routing table.
        if ((routing_table[node_idx].node_info & (1 << 0)) == 0)
        {
            Luos_SendData(service, &intro_msg, routing_table, (last_routing_table_entry * sizeof(routing_table_t)));
        }
    }
    detect_state_machine = 0;
    return true;
}

/******************************************************************************
 * @brief Send the completed node indexes to a specific node
 * @param service : Service who send
 * @param node_id : node to send indexes
 * @param phy_index : phy index in the node concerned by these indexes
 * @param node_indexes : node indexes to send
 * @return None
 ******************************************************************************/
void RoutingTB_SendNodeIndexes(service_t *service, uint16_t node_id, uint8_t phy_index, uint8_t *node_indexes)
{
    msg_t index_msg;
    index_msg.header.target_mode = NODEIDACK;
    index_msg.header.target      = node_id;
    LUOS_ASSERT(phy_index != 0xFF);
    uint8_t data[MAX_NODE_NUMBER / 8 + 1];

    // First we send the phy index allowing the target to know where to store the indexes
    index_msg.header.cmd = PHY_ID;
    Luos_SendData(service, &index_msg, &phy_index, sizeof(uint8_t));
    // Then we evaluate how many bytes we need to send the node indexes
    volatile uint16_t i = 0;
    for (i = (MAX_NODE_NUMBER / 8) + 1; i > 0; i--)
    {
        if (node_indexes[i - 1] != 0)
        {
            break;
        }
    }
    // Send the node indexes
    index_msg.header.cmd = NODE_INDEXES;
    memcpy(data, node_indexes, i);
    Luos_SendData(service, &index_msg, data, i);
}

/******************************************************************************
 * @brief Compute indexes of nodes on the network and send it to them
 * @param node_id : node id to compute indexes
 * @return None
 ******************************************************************************/
void RoutingTB_ComputeNodeIndexes(service_t *service, uint16_t node_index, uint16_t nb_node, connection_t *connection_table)
{
    /*
     * To get all the nodes indexes of the provided node ID, we have to parse the routing table and use the connection informations to know how to dispatch all the nodes.
     * Each node is connected to all the other nodes, we "just" have to find out trough which phy we can reach them.
     * If we don't have any child all the nodes are accessible from the phy of our parent.
     * Because node ID are contiguous the next node in the routing table (if it exist) will always have a connection information indicating trough which phy we can reach it. Let's say it is phy 1.
     * We will look at all the following nodes connections informations, if those informations doesn't involve our node we can consider that they are accessible trough phy 1.
     * If we encounter a node with parent connection informations that involve our node, this means that we completed the previous phy indexing, or exploring a parralel branch.
     * - If the phy we were indexing is the same as the parent phy, we have to add them to the same index.
     * - If the phy we were indexing is different from the parent phy, we have to send the phy indexing and start a new one dedicated to the phy described by the parent node connection informations we have.
     *  we can send it and start a new one for the phy described by the node connection informations.
     * When we reach the end of the routing table, we have 2 options:
     * - If the phy we were indexing is the same as our parent phy, we have to add all the parents nodes in the same phy indexing then send it.
     * - If the phy we were indexing is different from our parent phy, we have to send the phy indexing and start a new one dedicated to the phy described by the parent node connection informations we have.
     */
    uint16_t node_idx;
    uint8_t nodes_indexes[MAX_NODE_NUMBER / 8 + 1] = {0};
    int node_phy                                   = -1;
    if (nb_node == 1)
    {
        // We are the only node on the network, we don't have to send any indexes.
        return;
    }
    // If we have no child, this means that all the nodes are accessible trough the phy of our parent.
    if (connection_table[node_index + 1].parent.node_id != node_index + 1)
    {
        // The next node is not connected to our node, this means that we have no childs, all the nodes are accessible trough the phy of our parent.
        node_phy = connection_table[node_index].child.phy_id;
        for (node_idx = 0; node_idx < nb_node; node_idx++)
        {
            if (node_idx == node_index)
            {
                // This is our node, we don't care.
                continue;
            }
            // Add the node index to the nodes_indexes table.
            uint8_t bit_index = node_idx;
            nodes_indexes[bit_index / 8] |= 1 << (bit_index % 8);
        }
        RoutingTB_SendNodeIndexes(service, node_index + 1, node_phy, nodes_indexes);
        return;
    }

    // Start from the provided node index and parse the connection table until the end.
    for (node_idx = node_index + 1; node_idx < nb_node; node_idx++)
    {
        // Check if the connection informations involve our node.
        if (connection_table[node_idx].parent.node_id == node_index + 1)
        {
            // This node consider our node as its parent!
            // This means that we completed the previous phy indexing, or we are exploring a parralel branch.
            // If the phy of the new branch is not the same, we can send it and start a new one for the phy described by the node connection informations.
            // If the phy of the new branch is the same, we have to add them to the same index.
            if (node_phy != connection_table[node_idx].parent.phy_id)
            {
                // The parent phy_id is different than the one we were working on, we can consider that we completed the previous phy indexing, we can send it and start a new one for the phy described by the node connection informations.
                if (node_phy != -1)
                {
                    // We have to send the indexes we completed.
                    RoutingTB_SendNodeIndexes(service, node_index + 1, node_phy, nodes_indexes);
                }

                // We have to reset the indexes table and set the node_phy to the index of the concerned phy.
                memset(nodes_indexes, 0, sizeof(nodes_indexes));
                node_phy = connection_table[node_idx].parent.phy_id;
            }
        }
        // Add the node index to the nodes_indexes table.
        uint8_t bit_index = node_idx;
        nodes_indexes[bit_index / 8] |= 1 << (bit_index % 8);
    }

    // We reached the end of the routing table!
    // Check if the node index is the root of the routing table. If it is we just have to send the indexes and we are done.
    if (node_index == 0)
    {
        if (node_phy != -1)
        {
            // We have to send the indexes we completed.
            RoutingTB_SendNodeIndexes(service, node_index + 1, node_phy, nodes_indexes);
        }
        return;
    }
    // Compare the current node_phy to the current node index connection informations.
    if (node_phy != connection_table[node_index].child.phy_id)
    {
        if (node_phy != -1)
        {
            // We were indexing another phy, we can consider it as done and send it. Then we have to start a new for connection_table[node_index].child.phy_id.
            RoutingTB_SendNodeIndexes(service, node_index + 1, node_phy, nodes_indexes);
        }
        // We have to reset the indexes table and set the node_phy to the index of the concerned phy.
        memset(nodes_indexes, 0, sizeof(nodes_indexes));
        node_phy = connection_table[node_index].child.phy_id;
    }

    // Now we can put all the parent slots in the current indexes table.
    for (node_idx = 0; node_idx < node_index; node_idx++)
    {
        // Add the node index to the nodes_indexes table.
        uint8_t bit_index = node_idx;
        nodes_indexes[bit_index / 8] |= 1 << (bit_index % 8);
    }

    if (node_phy != -1)
    {
        // Send the last phy indexes.
        RoutingTB_SendNodeIndexes(service, node_index + 1, node_phy, nodes_indexes);
    }
}

/******************************************************************************
 * @brief Send the completed service indexes to a specific node
 * @param service : Service who send
 * @param node_id : node to send indexes
 * @param phy_index : phy index in the node concerned by these indexes
 * @param node_indexes : node indexes to send
 * @param service_indexes : service indexes to send
 * @return None
 ******************************************************************************/
void RoutingTB_SendServiceIndexes(service_t *service, uint16_t node_id, uint8_t phy_index, uint8_t *service_indexes)
{
    msg_t index_msg;
    index_msg.header.target_mode = NODEIDACK;
    index_msg.header.target      = node_id;
    LUOS_ASSERT(phy_index != 0xFF);
    uint8_t data[MAX_SERVICE_NUMBER / 8 + 1];

    // First we send the phy index allowing the target to know where to store the indexes
    index_msg.header.cmd = PHY_ID;
    Luos_SendData(service, &index_msg, &phy_index, sizeof(uint8_t));
    // Then we evaluate how many bytes we need to send for the service indexes
    volatile uint16_t i = 0;
    for (i = (MAX_SERVICE_NUMBER / 8) + 1; i > 0; i--)
    {
        if (service_indexes[i - 1] != 0)
        {
            break;
        }
    }
    if (i != 0)
    {
        // Send the service indexes
        index_msg.header.cmd = SERVICE_INDEXES;
        memcpy(data, service_indexes, i);
        Luos_SendData(service, &index_msg, data, i);
    }
}

/******************************************************************************
 * @brief Compute indexes of nodes on the network and send it to them
 * @param node_id : node id to compute indexes
 * @return None
 ******************************************************************************/
void RoutingTB_ComputeServiceIndexes(service_t *service, uint16_t rtb_index)
{
    /*
     * To get all the nodes indexes of the provided node ID, we have to parse the routing table and use the connection informations to know how to dispatch all the nodes.
     * Each node is connected to all the other nodes, we "just" have to find out trough which phy we can reach them.
     * Because node ID are contiguous the next node in the routing table (if it exist) will always have a connection information indicating trough which phy we can reach it. Let's say it phy 1.
     * Then we will look at all the following nodes connections informations, if those informations doesn't involve our node we can consider taht they are accessible trough phy 1.
     * If we encounter a node with connection informations that involve our node, this means that we completed the previous phy indexing, or we are exploring a new branch.
     * - If the phy we were indexing is the same as the parent phy, we have to add them to the same index.
     * - If the phy we were indexing is different from the parent phy, we have to send the phy indexing and start a new one dedicated to the phy described by the parent node connection informations we have.
     * When we reach the end of the routing table, we have 2 options:
     * - If the phy we were indexing is the same as our parent phy, we have to add all the parents nodes in the same phy indexing then send it.
     * - If the phy we were indexing is different from our parent phy, we have to send the phy indexing and start a new one dedicated to the phy described by the parent node connection informations we have.
     */
    LUOS_ASSERT((rtb_index < last_routing_table_entry) && (routing_table[rtb_index].mode == NODE));
    uint16_t rtb_idx;
    uint8_t services_indexes[MAX_SERVICE_NUMBER / 8 + 1] = {0};
    int node_phy                                         = -1;
    // Start from the provided node index and parse the routing table until the end.
    for (rtb_idx = rtb_index + 1; rtb_idx < last_routing_table_entry; rtb_idx++)
    {
        switch (routing_table[rtb_idx].mode)
        {
            case NODE:
                // This is a node slot, check if the connection informations involve our node.
                if (routing_table[rtb_idx].connection.parent.node_id == routing_table[rtb_index].node_id)
                {
                    // This node consider our node as its parent!
                    // This means that we completed the previous phy indexing, or we are exploring a parallel branch.
                    // If the phy of the new branch is not the same, we can send it and start a new one for the phy described by the node connection informations.
                    // If the phy of the new branch is the same, we have to add them to the same index.
                    if (node_phy != routing_table[rtb_idx].connection.parent.phy_id)
                    {
                        if (node_phy != -1)
                        {
                            // We have to send the indexes we completed.
                            RoutingTB_SendServiceIndexes(service, routing_table[rtb_index].node_id, node_phy, services_indexes);
                        }

                        // We have to reset the indexes table and set the node_phy to the index of the concerned phy.
                        memset(services_indexes, 0, sizeof(services_indexes));
                        node_phy = routing_table[rtb_idx].connection.parent.phy_id;
                    }
                }
                else
                {
                    if (node_phy == -1)
                    {
                        // The first node we encounter is not our child, this means that we have no childs, all the nodes are accessible trough the phy of our parent.
                        node_phy = routing_table[rtb_idx].connection.child.phy_id;
                    }
                }
                break;
            case SERVICE:
                // This is a service slot.
                // if a node phy is set, this service is accessible trough this phy, we have to add it to the services_indexes table.
                // else this service is a local one (already referenced by Luos_engine phy), we don't care.
                if (node_phy != -1)
                {
                    // Add the service index to the services_indexes table.
                    uint8_t bit_index = routing_table[rtb_idx].id - 1; // Because 1 represent bit index 0.
                    services_indexes[bit_index / 8] |= 1 << (bit_index % 8);
                }
                break;
            case CLEAR:
            default:
                LUOS_ASSERT(0);
                break;
        }
    }

    // We reached the end of the routing table!
    // Check if the node index is the root of the routing table. If it is we just have to send the indexes and we are done.
    if (rtb_index == 0)
    {
        if (node_phy != -1)
        {
            // We have to send the indexes we completed.
            RoutingTB_SendServiceIndexes(service, routing_table[rtb_index].node_id, node_phy, services_indexes);
        }
        return;
    }
    // Compare the current node_phy to the current node index connection informations.
    if (node_phy != routing_table[rtb_index].connection.child.phy_id)
    {
        if (node_phy != -1)
        {
            // We were indexing another phy, we can consider it as done and send it. Then we have to start a new one for routing_table[node_index].connection.child.phy_id.
            RoutingTB_SendServiceIndexes(service, routing_table[rtb_index].node_id, node_phy, services_indexes);
        }
        // We have to reset the indexes table and set the node_phy to the index of the concerned phy.
        memset(services_indexes, 0, sizeof(services_indexes));
        node_phy = routing_table[rtb_index].connection.child.phy_id;
    }

    // Now we can put all the parent slots in the current indexes table.
    for (rtb_idx = 0; rtb_idx < rtb_index; rtb_idx++)
    {
        if (routing_table[rtb_idx].mode == SERVICE)
        {
            // Add the service index to the services_indexes table.
            uint8_t bit_index = routing_table[rtb_idx].id - 1; // Because 1 represent bit index 0.
            services_indexes[bit_index / 8] |= 1 << (bit_index % 8);
        }
    }

    if (node_phy != -1)
    {
        // Send the last phy indexes.
        RoutingTB_SendServiceIndexes(service, routing_table[rtb_index].node_id, node_phy, services_indexes);
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
 * @return return true if the detection is complete
 ******************************************************************************/
bool RoutingTB_DetectServices(service_t *service)
{
    LUOS_ASSERT(service);
    static uint8_t detect_state_machine = 0;
    static uint16_t nb_node             = 0;
    static connection_t connection_table[MAX_NODE_NUMBER];
    int result;
    switch (detect_state_machine)
    {
        case 0:
            // Create a connetion list to store all the connection describing the topology
            memset(connection_table, 0xFF, sizeof(connection_table));
            detect_state_machine++;
        case 1:
            result = LuosIO_TopologyDetection(service, connection_table);
            if (result == 0)
            {
                // No node detected meaning that the detection is not finished
                return false;
            }
            if (result < 0)
            {
                // another detection is in progress, stop this one
                detect_state_machine = 0;
                return true;
            }
            nb_node = (uint16_t)result;
            // Clear data reception state
            Luos_ReceiveData(NULL, NULL, NULL);
            // Clear the routing table.
            RoutingTB_Erase();
            detect_state_machine++;
        case 2:
            // Generate the routing_table
            result = RoutingTB_Generate(service, nb_node, connection_table);
            if (result == 0)
            {
                // RTB generation not finished
                return false;
            }
            if (result < 0)
            {
                // another detection is in progress, stop this one
                detect_state_machine = 0;
                return true;
            }
            detect_state_machine++;
        case 3:
            // We have a complete routing table now share it with others.
            if (RoutingTB_Share(service, nb_node))
            {
                detect_state_machine++;
            }
            return false;
            break;
        case 4:
            // Send a message to indicate the end of the detection
            RoutingTB_SendEndDetection(service);
            // Clear statistic of node who start the detction
            Luos_ResetStatistic();
            detect_state_machine = 0;
            return true;
            break;
        default:
            LUOS_ASSERT(0);
            break;
    }
    LUOS_ASSERT(0);
    return false;
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
    if (nodeid == 0)
    {
        // This mean that a node crashed before detection, we don't have to remove it.
        return;
    }
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
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr - 1));
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
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr - 1));
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
    // If the result is not initialized return 0
    if (RTFilter_InitCheck(result) == FAILED)
    {
        result->result_nbr = 0;
    }
    // Search all the entries of the research table
    while (entry_nbr < result->result_nbr)
    {
        // find a service with the wanted node_id
        if (RoutingTB_NodeIDFromID(result->result_table[entry_nbr]->id) != node_id)
        {
            // if we find an other node_id, erase it from the research table
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr - 1));
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
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr - 1));
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
            memcpy(&result->result_table[entry_nbr], &result->result_table[entry_nbr + 1], sizeof(routing_table_t *) * (result->result_nbr - entry_nbr - 1));
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
