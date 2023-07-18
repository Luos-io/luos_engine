/******************************************************************************
 * @file routingTable
 * @brief routing table description function
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef ROUTING_TABLE
#define ROUTING_TABLE

#include "struct_luos.h"
#include "luos_list.h"
#include "node.h"
#include "routing_table.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
// ********************* routing_table search tools ************************
uint16_t RoutingTB_NodeIDFromID(uint16_t id);

// ********************* routing_table management tools ************************
void RoutingTB_ComputeRoutingTableEntryNB(void);
bool RoutingTB_DetectServices(service_t *service);
void RoutingTB_ConvertNodeToRoutingTable(routing_table_t *entry, node_t *node);
void RoutingTB_ConvertServiceToRoutingTable(routing_table_t *entry, service_t *service);
void RoutingTB_RemoveNode(uint16_t nodeid);
void RoutingTB_RemoveService(uint16_t id);
void RoutingTB_Erase(void);
routing_table_t *RoutingTB_Get(void);
uint16_t *RoutingTB_GetLastNode(void);
uint16_t RoutingTB_GetLastEntry(void);

#endif /* ROUTING_TABLE */
