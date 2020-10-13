/******************************************************************************
 * @file routingTable
 * @brief routing table descrption function
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TABLE
#define TABLE

#include "luos.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_CONTAINERS_NUMBER 40

typedef enum
{
    CLEAR,     // No content
    CONTAINER, // Contain a container informations
    NODE,      // Contain a node informations
} entry_mode_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/* This structure is used to receive or send messages between containers in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__))
{
    entry_mode_t mode;
    union
    {
        struct __attribute__((__packed__))
        {                               // CONTAINER mode entry
            unsigned short id;          // Container ID
            unsigned char type;         /*!< Container type. */
            char alias[MAX_ALIAS_SIZE]; /*!< Container alias. */
        };
        struct __attribute__((__packed__))
        {                                 // NODE mode entry
            luos_uuid_t uuid;             // Node UUID
            unsigned short port_table[4]; // Node link table
        };
    };
} routing_table_t;

/*******************************************************************************
 * Function
 ******************************************************************************/
// ********************* routing_table search tools ************************
int8_t RoutingTB_IDFromAlias(char *alias);
int8_t RoutingTB_IDFromType(luos_type_t type);
int8_t RoutingTB_IDFromContainer(container_t *container);
char *RoutingTB_AliasFromId(uint16_t id);
luos_type_t RoutingTB_TypeFromID(uint16_t id);
luos_type_t RoutingTB_TypeFromAlias(char *alias);
char *RoutingTB_StringFromType(luos_type_t type);
uint8_t RoutingTB_ContainerIsSensor(luos_type_t type);
int8_t RoutingTB_GetNodeNB(void);
void RoutingTB_GetNodeList(unsigned short *list);
int8_t RoutingTB_GetNodeID(unsigned short index);

// ********************* routing_table management tools ************************
void RoutingTB_ComputeRoutingTableEntryNB(void);
void RoutingTB_DetectContainers(container_t *container);
void RoutingTB_ConvertNodeToRoutingTable(routing_table_t *entry, luos_uuid_t uuid, unsigned short *port_table, int branch_nb);
void RoutingTB_ConvertContainerToRoutingTable(routing_table_t *entry, container_t *container);
void RoutingTB_InsertOnRoutingTable(routing_table_t *entry);
void RoutingTB_RemoveOnRoutingTable(int id);
void RoutingTB_Erase(void);
routing_table_t *RoutingTB_Get(void);
int8_t RoutingTB_GetLastContainer(void);
int8_t RoutingTB_GetLastEntry(void);

#endif /* TABLE */
