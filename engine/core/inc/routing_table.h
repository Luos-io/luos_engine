/******************************************************************************
 * @file routingTable
 * @brief routing table description function
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TABLE
#define TABLE

#include "struct_luos.h"
#include "luos_list.h"
#include "node.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum
{
    CLEAR,   // No content
    SERVICE, // Contain a service informations
    NODE,    // Contain a node informations
} entry_mode_t;

/* This structure is used to receive or send messages between services in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__))
{
    uint8_t mode; // entry_mode_t
    union
    {
        struct __attribute__((__packed__))
        {                               // SERVICE mode entry
            uint16_t id;                // Service ID.
            uint16_t type;              // Service type.
            uint8_t access;             // Service Access access_t
            char alias[MAX_ALIAS_SIZE]; // Service alias.
        };
        struct __attribute__((__packed__))
        { // NODE mode entry
            // Watch out this structure have a lot similarities with the node_t struct.
            // It is similar to allow copy of a node_t struct directly in this one.
            // But you there is potentially a port_table size difference so
            // Do not replace it with node_t struct.
            struct __attribute__((__packed__))
            {
                uint16_t node_id : 12;  // Node id
                uint16_t certified : 4; // True if the node have a certificate
                uint8_t node_info;      // node info can contain info such as the saving of routing table
            };
            connection_t connection; // Node connection source
        };
        uint8_t unmap_data[MAX_ALIAS_SIZE + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t)];
    };
} routing_table_t;

/******************************************************************************
 * @struct search_result_t
 * @brief allow user to get back result of routing table filtering
 ******************************************************************************/
typedef struct
{
    uint16_t result_nbr;
    routing_table_t *result_table[MAX_RTB_ENTRY];
} search_result_t;

/*******************************************************************************
 * Function
 ******************************************************************************/

// ********************* routing table  filtering ********************************
error_return_t RTFilter_InitCheck(search_result_t *result);
search_result_t *RTFilter_Reset(search_result_t *result);
search_result_t *RTFilter_ID(search_result_t *result, uint16_t id);
search_result_t *RTFilter_Type(search_result_t *result, luos_type_t type);
search_result_t *RTFilter_Node(search_result_t *result, uint16_t node_id);
search_result_t *RTFilter_Alias(search_result_t *result, char *alias);
search_result_t *RTFilter_Service(search_result_t *result, service_t *service);

#endif /* TABLE */
