#include <routingTable.h>
#include <string.h>
#include <stdio.h>
#include "hal.h"

route_table_t route_table[MAX_MODULES_NUMBER];
volatile int last_module = 0;
volatile int last_route_table_entry = 0;

// ********************* route_table search tools ************************
// Return an id from an alias (return 0 if no alias match)
int id_from_alias(char *alias)
{
    if (*alias != -1)
    {
        for (int i = 0; i <= last_route_table_entry; i++)
        {
            if (route_table[i].mode == MODULE)
            {
                if (strcmp(route_table[i].alias, alias) == 0)
                {
                    return route_table[i].id;
                }
            }
        }
    }
    return -1;
}

int id_from_type(module_type_t type)
{
    for (int i = 0; i <= last_route_table_entry; i++)
    {
        if (route_table[i].mode == MODULE)
        {
            if (type == route_table[i].type)
            {
                return route_table[i].id;
            }
        }
    }
    return -1;
}

int id_from_module(module_t *module)
{
    // make sure route table is clean before sharing id
    if (last_route_table_entry == 0)
    {
        return 0;
    }
    return (int)module->vm->id;
}

// Create a string from a module type
char *string_from_type(module_type_t type)
{
    switch (type)
    {
    case STATE_MOD:
        return "State";
        break;
    case COLOR_MOD:
        return "Color";
        break;
    case SERVO_MOD:
        return "Servo";
        break;
    case ANGLE_MOD:
        return "Angle";
        break;
    case DISTANCE_MOD:
        return "DistanceSensor";
        break;
    case GATE_MOD:
        return "Gate";
        break;
    case DYNAMIXEL_MOD:
        return "DynamixelMotor";
        break;
    case STEPPER_MOD:
        return "Stepper";
        break;
    case DCMOTOR_MOD:
        return "DCMotor";
        break;
        break;
    case HANDY_MOD:
        return "Handy";
        break;
    case IMU_MOD:
        return "Imu";
        break;
    case LIGHT_MOD:
        return "LightSensor";
        break;
    case CONTROLLED_MOTOR_MOD:
        return "ControlledMotor";
    case VOID_MOD:
        return "Void";
        break;
    case LOAD_MOD:
        return "Load";
        break;
    case VOLTAGE_MOD:
        return "Voltage";
        break;
    default:
        return "Unknown";
        break;
    }
}

char *alias_from_id(uint16_t id)
{
    for (int i = 0; i <= last_route_table_entry; i++)
    {
        if (route_table[i].mode == MODULE)
        {
            if (id == route_table[i].id)
            {
                return route_table[i].alias;
            }
        }
    }
    return (char *)0;
}

int bigest_id()
{
    int max_id = 0;
    for (int i = 0; i < last_route_table_entry; i++)
    {
        if (route_table[i].mode == MODULE)
        {
            if (route_table[i].id > max_id)
            {
                max_id = route_table[i].id;
            }
        }
    }
    return max_id;
}

// check if the module is a sensor or not
uint8_t is_sensor(module_type_t type)
{
    if ((type == ANGLE_MOD) ||
        (type == STATE_MOD) ||
        (type == DYNAMIXEL_MOD) ||
        (type == DISTANCE_MOD) ||
        (type == IMU_MOD) ||
        (type == LOAD_MOD) ||
        (type == CONTROLLED_MOTOR_MOD) ||
        (type == VOLTAGE_MOD) ||
        (type == LIGHT_MOD))
    {
        return 1;
    }
    return 0;
}

// Return a module_type from an id
module_type_t type_from_id(uint16_t id)
{
    for (int i = 0; i <= last_route_table_entry; i++)
    {
        if (route_table[i].mode == MODULE)
        {
            if (route_table[i].id == id)
            {
                return route_table[i].type;
            }
        }
    }
    return -1;
}

// Return a module type from alias
module_type_t type_from_alias(char *alias)
{
    uint16_t id = id_from_alias(alias);
    return type_from_id(id);
}

int get_node_nb(void)
{
    int node_nb = 0;
    for (int i = 0; i <= last_route_table_entry; i++)
    {
        if (route_table[i].mode == NODE)
        {
            node_nb++;
        }
    }
    return node_nb - 1;
}

void get_node_list(unsigned short *list)
{
    int node_nb = 0;
    for (int i = 0; i <= last_route_table_entry; i++)
    {
        if (route_table[i].mode == NODE)
        {
            list[node_nb] = i;
            node_nb++;
        }
    }
}

int get_node_id(unsigned short index)
{
    return route_table[index + 1].id;
}

// ********************* route_table management tools ************************

// compute entry number
void compute_route_table_entry_nb(void)
{
    for (int i = 0; i < MAX_MODULES_NUMBER; i++)
    {
        if (route_table[i].mode == MODULE)
        {
            last_module = route_table[i].id;
        }
        if (route_table[i].mode == CLEAR)
        {
            last_route_table_entry = i;
            return;
        }
    }
}

// manage module name increment
void add_num_to_alias(char *alias, int num)
{
    int intsize = 1;
    if (num > 9)
    {
        // The string size of num is 2
        intsize = 2;
    }
    if (num > 99)
    {
        // This is probably a mistake, put an error into the alias
        memset(alias, 0, 15);
        memcpy(alias, "error", strlen("error"));
        return;
    }
    // Change size to fit into 15 characters
    if (strlen(alias) > (15 - intsize))
    {
        alias[(15 - intsize)] = '\0';
    }
    else
    {
        alias[strlen(alias)] = '\0';
    }
    // Add a number at the end of the alias
    sprintf(alias, "%s%d", alias, num);
}

int wait_route_table(module_t *module, msg_t *intro_msg)
{
    const int timeout = 15; // timeout in ms
    const int entry_bkp = last_route_table_entry;
    luos_send(module, intro_msg);
    uint32_t timestamp = node_get_systick();
    while ((node_get_systick() - timestamp) < timeout)
    {
        // If this request is for a module in this board allow him to respond.
        luos_loop();
        if (entry_bkp != last_route_table_entry)
        {
            return 1;
        }
    }
    return 0;
}

// Detect all modules and create a route table with it.
// If multiple modules have the same name it will be changed with a number in it automatically.
// At the end this function create a list of sensors id
void detect_modules(module_t *module)
{
    msg_t intro_msg, auto_name;
    unsigned char i = 0;
    // clear network detection state and all previous info.
    flush_route_table();
    // Starts the topology detection.
    int nb_mod = robus_topology_detection(module->vm);
    if (nb_mod > MAX_MODULES_NUMBER - 1)
        nb_mod = MAX_MODULES_NUMBER - 1;

    // Then, asks for introduction for every found modules.
    int try = 0;
    int last_id = bigest_id();
    while ((last_id < nb_mod) && (try < nb_mod))
    {
        intro_msg.header.cmd = IDENTIFY_CMD;
        intro_msg.header.target_mode = IDACK;
        intro_msg.header.size = 0;
        // Target next unknown module (the first one of the next node)
        intro_msg.header.target = last_id + 1;
        try
            ++;
        // Ask to introduce and wait for a reply
        if (!wait_route_table(module, &intro_msg))
        {
            // We don't get the answer
            nb_mod = last_id;
            break;
        }
        last_id = bigest_id();
    }
    for (int id = 1; id <= nb_mod; id++)
    {
        int computed_id = id_from_alias(alias_from_id(id));
        if ((computed_id != id) & (computed_id != -1))
        {
            int annotation = 1;
            // this name already exist in the network change it and send it back.
            // find the new alias to give him
            char aliasbis[15] = {0};
            memcpy(aliasbis, alias_from_id(id), 15);
            add_num_to_alias(alias_from_id(id), annotation++);
            while (id_from_alias(alias_from_id(id)) != id)
            {
                memcpy(alias_from_id(id), aliasbis, 15);
                add_num_to_alias(alias_from_id(id), annotation++);
            }
            auto_name.header.target_mode = ID;
            auto_name.header.target = id;
            auto_name.header.cmd = WRITE_ALIAS;
            auto_name.header.size = strlen(alias_from_id(id));
            // Copy the alias into the data field of the message
            for (i = 0; i < auto_name.header.size; i++)
            {
                auto_name.data[i] = alias_from_id(id)[i];
            }
            auto_name.data[auto_name.header.size] = '\0';
            // Send the message using the WRITE_ALIAS system command
            robus_send(module->vm, &auto_name);
            //TODO update name into route_table
        }
    }

    // send route table to each nodes. Route tables are commonly usable for each modules of a node.
    int nb_node = get_node_nb();
    unsigned short node_index_list[MAX_MODULES_NUMBER];
    get_node_list(node_index_list);

    intro_msg.header.cmd = INTRODUCTION_CMD;
    intro_msg.header.target_mode = IDACK;

    for (int i = 1; i <= nb_node; i++)
    {
        intro_msg.header.target = get_node_id(node_index_list[i]);
        luos_send_data(module, &intro_msg, route_table, (last_route_table_entry * sizeof(route_table_t)));
    }
}

void convert_board_to_route_table(route_table_t *entry, luos_uuid_t uuid, unsigned short *port_table, int branch_nb)
{
    entry->uuid = uuid;
    for (int i = 0; i < 4; i++)
    {
        if (i < branch_nb)
        {
            entry->port_table[i] = port_table[i];
        }
        else
        {
            entry->port_table[i] = 0;
        }
    }
    entry->mode = NODE;
}

// convert a module on the route_table
void convert_module_to_route_table(route_table_t *entry, module_t *module)
{
    entry->type = module->vm->type;
    entry->id = module->vm->id;
    entry->mode = MODULE;
    for (int i = 0; i < MAX_ALIAS_SIZE; i++)
    {
        entry->alias[i] = module->alias[i];
    }
}

// add a new module on the route_table
void insert_on_route_table(route_table_t *entry)
{
    memcpy(&route_table[last_route_table_entry++], entry, sizeof(route_table_t));
    if (entry->mode == MODULE)
    {
        last_module = entry->id;
    }
}

// remove an entry from route_table
void remove_on_route_table(int index)
{
    route_table[index].alias[0] = '\0';
    route_table[index].type = VOID_MOD;
    route_table[index].id = 0;
    route_table[index].mode = CLEAR;
}

// erase route_table
void flush_route_table()
{
    memset(route_table, 0, sizeof(route_table));
    last_module = 0;
    last_route_table_entry = 0;
}

//return the route_table
route_table_t *get_route_table(void)
{
    return route_table;
}

//return the last ID registered into the route_table
int get_last_module(void)
{
    return last_module;
}

//return the last ID registered into the route_table
int get_last_entry(void)
{
    return last_route_table_entry;
}
