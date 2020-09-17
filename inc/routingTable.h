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
#define MAX_MODULES_NUMBER 40

typedef enum
{
    CLEAR,  // No content
    MODULE, // Contain a module informations
    NODE,   // Contain a node informations
} entry_mode_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

 /* This structure is used to receive or send messages between modules in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__))
{
    entry_mode_t mode;
    union {
        struct __attribute__((__packed__))
        {                               // MODULE mode entry
            unsigned short id;          // Module ID
            unsigned char type;         /*!< Module type. */
            char alias[MAX_ALIAS_SIZE]; /*!< Module alias. */
        };
        struct __attribute__((__packed__))
        {                                 // NODE mode entry
            luos_uuid_t uuid;             // Node UUID
            unsigned short port_table[4]; // Node link table
        };
    };
} route_table_t;

/*******************************************************************************
 * Function
 ******************************************************************************/
// ********************* route_table search tools ************************
int8_t RouteTB_IDFromAlias(char *alias);
int8_t RouteTB_IDFromType(module_type_t type);
int8_t RouteTB_IDFromModule(module_t *module);
char *RouteTB_AliasFromId(uint16_t id);
module_type_t RouteTB_TypeFromID(uint16_t id);
module_type_t RouteTB_TypeFromAlias(char *alias);
char *RouteTB_StringFromType(module_type_t type);
uint8_t RouteTB_ModuleIsSensor(module_type_t type);
int8_t RouteTB_GetNodeNB(void);
void RouteTB_GetNodeList(unsigned short *list);
int8_t RouteTB_GetNodeID(unsigned short index);

// ********************* route_table management tools ************************
void RouteTB_ComputeRouteTableEntryNB(void);
void RouteTB_DetectModules(module_t *module);
void RouteTB_ConvertNodeToRouteTable(route_table_t *entry, luos_uuid_t uuid, unsigned short *port_table, int branch_nb);
void RouteTB_ConvertModuleToRouteTable(route_table_t *entry, module_t *module);
void RouteTB_InsertOnRouteTable(route_table_t *entry);
void RouteTB_RemoveOnRouteTable(int id);
void RouteTB_Erase(void);
route_table_t *RouteTB_Get(void);
int8_t RouteTB_GetLastModule(void);
int8_t RouteTB_GetLastEntry(void);

#endif /* TABLE */
