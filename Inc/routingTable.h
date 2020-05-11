#ifndef TABLE
#define TABLE

#define MAX_MODULES_NUMBER 40

#include "luos.h"

typedef enum
{
    CLEAR,  // No content
    MODULE, // Contain a module informations
    NODE,   // Contain a node informations
} entry_mode_t;

/**
 * \struct route_table_t
 * \brief Message structure.
 *
 * This structure is used to receive or send messages between modules in slave
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

// ********************* route_table search tools ************************
int id_from_alias(char *alias);
int id_from_type(module_type_t type);
char *string_from_type(module_type_t type);
char *alias_from_id(uint16_t id);
module_type_t type_from_id(uint16_t id);
uint8_t is_sensor(module_type_t type);
module_type_t type_from_alias(char *alias);
int get_node_nb(void);
void get_node_list(unsigned short *list);
int get_node_id(unsigned short index);

// ********************* route_table management tools ************************
void compute_route_table_entry_nb(void);
void detect_modules(module_t *module);
void convert_board_to_route_table(route_table_t *entry, luos_uuid_t uuid, unsigned short *port_table, int branch_nb);
void convert_module_to_route_table(route_table_t *entry, module_t *module);
void insert_on_route_table(route_table_t *entry);
void remove_on_route_table(int id);
void flush_route_table(void);
route_table_t *get_route_table(void);
int get_last_module(void);
int get_last_entry(void);

#endif /* TABLE */
