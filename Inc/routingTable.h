#ifndef TABLE
#define TABLE

#define MAX_MODULES_NUMBER 40

#include "luos.h"

/**
 * \struct route_table_t
 * \brief Message structure.
 *
 * This structure is used to receive or send messages between modules in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)){
    unsigned char type; /*!< Module type. */
    char alias[MAX_ALIAS_SIZE];/*!< Module alias. */
}route_table_t;


// ********************* route_table search tools ************************
int id_from_alias(char* alias);
int id_from_type(module_type_t type);
char* string_from_type(module_type_t type);
module_type_t type_from_id(uint16_t id);
uint8_t is_sensor(module_type_t type);
module_type_t type_from_alias(char* alias);

// ********************* route_table management tools ************************
void detect_modules(module_t* module);
void add_on_route_table (int id, char type, char* alias);
void insert_on_route_table (int id, route_table_t entry);
void remove_on_route_table (int id);
void flush_route_table(void);
route_table_t* get_route_table(void);
volatile int get_last_module(void);
route_table_t deser_introduction(msg_t *msg);

#endif /* TABLE */
