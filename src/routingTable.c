/******************************************************************************
 * @file routingTable
 * @brief routing table descrption function
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <routingTable.h>

#include <string.h>
#include <stdio.h>
#include <luosHAL.h>
#include "context.h" //TODO to remove

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
route_table_t route_table[MAX_MODULES_NUMBER];
volatile int last_module = 0;
volatile int last_route_table_entry = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void RouteTB_AddNumToAlias(char *alias, int num);
static int8_t RouteTB_BigestID(void);
static int8_t RouteTB_WaitRouteTable(module_t *module, msg_t *intro_msg);

// ************************ route_table search tools ***************************

/******************************************************************************
 * @brief  Return an id from alias
 * @param pointer to alias
 * @return ID or Error
 ******************************************************************************/
int8_t RouteTB_IDFromAlias(char *alias)
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
/******************************************************************************
 * @brief  Return an id from type
 * @param type of module look at
 * @return ID or Error
 ******************************************************************************/
int8_t RouteTB_IDFromType(module_type_t type)
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
/******************************************************************************
 * @brief  Return an id from module
 * @param module look at
 * @return ID or Error
 ******************************************************************************/
int8_t RouteTB_IDFromModule(module_t *module)
{
    // make sure route table is clean before sharing id
    if (last_route_table_entry == 0)
    {
        return 0;
    }
    return (int)module->vm->id;
}
/******************************************************************************
 * @brief  Return module Alias from ID
 * @param id module look at
 * @return pointer to string or Error
 ******************************************************************************/
char *RouteTB_AliasFromId(uint16_t id)
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
/******************************************************************************
 * @brief  Return module type from ID
 * @param id module look at
 * @return pointer module or Error
 ******************************************************************************/
module_type_t RouteTB_TypeFromID(uint16_t id)
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
/******************************************************************************
 * @brief  Return module type from alias
 * @param alias module look at
 * @return pointer to module or Error
 ******************************************************************************/
module_type_t RouteTB_TypeFromAlias(char *alias)
{
    uint16_t id = RouteTB_IDFromAlias(alias);
    return RouteTB_TypeFromID(id);
}
/******************************************************************************
 * @brief  Create a string from a module type
 * @param type of module look at
 * @return pointer to string or Error
 ******************************************************************************/
char *RouteTB_StringFromType(module_type_t type)
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
/******************************************************************************
 * @brief  check if the module is a sensor or not
 * @param module look at
 * @return Error
 ******************************************************************************/
uint8_t RouteTB_ModuleIsSensor(module_type_t type)
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
/******************************************************************************
 * @brief  return bigest ID in list
 * @param None
 * @return ID
 ******************************************************************************/
static int8_t RouteTB_BigestID(void)
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
/******************************************************************************
 * @brief  get number of a node on network
 * @param None
 * @return Error
 ******************************************************************************/
int8_t RouteTB_GetNodeNB(void)
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
/******************************************************************************
 * @brief  get List of node on network
 * @param pointer to list of Node
 * @return None
 ******************************************************************************/
void RouteTB_GetNodeList(unsigned short *list)
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
/******************************************************************************
 * @brief  get ID of node on network
 * @param pointer to index of Node
 * @return None
 ******************************************************************************/
int8_t RouteTB_GetNodeID(unsigned short index)
{
    return route_table[index + 1].id;
}

// ********************* route_table management tools ************************

/******************************************************************************
 * @brief compute entry number
 * @param None
 * @return None
 ******************************************************************************/
void RouteTB_ComputeRouteTableEntryNB(void)
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
/******************************************************************************
 * @brief manage module name increment to never have same alias
 * @param alias to change
 * @param nb to add
 * @return None
 ******************************************************************************/
static void RouteTB_AddNumToAlias(char *alias, int num)
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
/******************************************************************************
 * @brief time out to receive en route table from
 * @param module receive
 * @param intro msg in route table
 * @return None
 ******************************************************************************/
static int8_t RouteTB_WaitRouteTable(module_t *module, msg_t *intro_msg)
{
    const int timeout = 15; // timeout in ms
    const int entry_bkp = last_route_table_entry;
    Luos_SendMsg(module, intro_msg);
    uint32_t timestamp = LuosHAL_GetSystick();
    while ((LuosHAL_GetSystick() - timestamp) < timeout)
    {
        // If this request is for a module in this board allow him to respond.
        Luos_Loop();
        if (entry_bkp != last_route_table_entry)
        {
            return 1;
        }
    }
    return 0;
}

unsigned char RouteTB_ResetNetworkDetection(vm_t *vm)
{
    msg_t msg;

    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd = RESET_DETECTION;
    msg.header.size = 0;

    //we don't have any way to tell every modules to reset their detection do it twice to be sure
    if (Robus_SendMsg(vm, &msg))
        return 1;
    if (Robus_SendMsg(vm, &msg))
        return 1;
    // run luos_loop() to manage the 2 previous broadcast msgs.
    Luos_Loop();
    return 0;
}

static unsigned char RouteTB_NetworkTopologyDetection(module_t *module)
{
    unsigned short newid = 1;
    // Reset all detection state of modules on the network
    RouteTB_ResetNetworkDetection(module->vm);
    ctx.detection_mode = MASTER_DETECT;
    // wait for some us
    for (volatile unsigned int i = 0; i < (2 * TIMERVAL); i++)
        ;

    // setup sending vm
    module->vm->id = newid++;

    // Parse internal vm other than the sending one
    for (unsigned char i = 0; i < ctx.vm_number; i++)
    {
        if (&ctx.vm_table[i] != module->vm)
        {
            ctx.vm_table[i].id = newid++;
        }
    }

    ctx.detection.detected_vm = ctx.vm_number;

    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        ctx.detection_mode = MASTER_DETECT;
        if (Detect_PokeBranch(branch))
        {
            // Someone reply to our poke!
            // loop while the line is released
            int module_number = 0;
            while ((ctx.detection.keepline != NO_BRANCH) & (module_number < 1024))
            {
                if (Luos_SetExternId(module, IDACK, DEFAULTID, newid++))
                {
                    // set extern id fail
                    // remove this id and stop topology detection
                    newid--;
                    break;
                }
                module_number++;
                // wait for some us
                for (volatile unsigned int i = 0; i < (2 * TIMERVAL); i++)
                    ;
            }
        }
    }
    ctx.detection_mode = NO_DETECT;

    return newid - 1;
}

// Detect all modules and create a route table with it.
// If multiple modules have the same name it will be changed with a number in it automatically.
// At the end this function create a list of sensors id
void RouteTB_DetectModules(module_t *module)
{
    msg_t intro_msg, auto_name;
    unsigned char i = 0;
    // clear network detection state and all previous info.
    RouteTB_Erase();
    // Starts the topology detection.
    int nb_mod = RouteTB_NetworkTopologyDetection(module);
    if (nb_mod > MAX_MODULES_NUMBER - 1)
    {
        nb_mod = MAX_MODULES_NUMBER - 1;
    }

    // Then, asks for introduction for every found modules.
    int try_nb = 0;
    int last_id = RouteTB_BigestID();
    while ((last_id < nb_mod) && (try_nb < nb_mod))
    {
        intro_msg.header.cmd = IDENTIFY_CMD;
        intro_msg.header.target_mode = IDACK;
        intro_msg.header.size = 0;
        // Target next unknown module (the first one of the next node)
        intro_msg.header.target = last_id + 1;
        try_nb++;
        // Ask to introduce and wait for a reply
        if (!RouteTB_WaitRouteTable(module, &intro_msg))
        {
            // We don't get the answer
            nb_mod = last_id;
            break;
        }
        last_id = RouteTB_BigestID();
    }
    for (int id = 1; id <= nb_mod; id++)
    {
        int computed_id = RouteTB_IDFromAlias(RouteTB_AliasFromId(id));
        if ((computed_id != id) & (computed_id != -1))
        {
            int annotation = 1;
            // this name already exist in the network change it and send it back.
            // find the new alias to give him
            char aliasbis[15] = {0};
            memcpy(aliasbis, RouteTB_AliasFromId(id), 15);
            RouteTB_AddNumToAlias(RouteTB_AliasFromId(id), annotation++);
            while (RouteTB_IDFromAlias(RouteTB_AliasFromId(id)) != id)
            {
                memcpy(RouteTB_AliasFromId(id), aliasbis, 15);
                RouteTB_AddNumToAlias(RouteTB_AliasFromId(id), annotation++);
            }
            auto_name.header.target_mode = ID;
            auto_name.header.target = id;
            auto_name.header.cmd = WRITE_ALIAS;
            auto_name.header.size = strlen(RouteTB_AliasFromId(id));
            // Copy the alias into the data field of the message
            for (i = 0; i < auto_name.header.size; i++)
            {
                auto_name.data[i] = RouteTB_AliasFromId(id)[i];
            }
            auto_name.data[auto_name.header.size] = '\0';
            // Send the message using the WRITE_ALIAS system command
            Robus_SendMsg(module->vm, &auto_name);
            //TODO update name into route_table
        }
    }

    // send route table to each nodes. Route tables are commonly usable for each modules of a node.
    int nb_node = RouteTB_GetNodeNB();
    unsigned short node_index_list[MAX_MODULES_NUMBER];
    RouteTB_GetNodeList(node_index_list);

    intro_msg.header.cmd = INTRODUCTION_CMD;
    intro_msg.header.target_mode = IDACK;

    for (int i = 1; i <= nb_node; i++)
    {
        intro_msg.header.target = RouteTB_GetNodeID(node_index_list[i]);
        Luos_SendData(module, &intro_msg, route_table, (last_route_table_entry * sizeof(route_table_t)));
    }
}
/******************************************************************************
 * @brief entry in routable node with associate module
 * @param route table
 * @param uuid node
 * @param node table
 * @param brach for node
 * @return None
 ******************************************************************************/
void RouteTB_ConvertNodeToRouteTable(route_table_t *entry, luos_uuid_t uuid, unsigned short *port_table, int branch_nb)
{
    entry->uuid = uuid;
    for (uint8_t i = 0; i < 4; i++)
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
/******************************************************************************
 * @brief entry in routable module associate to a node
 * @param route table
 * @param module in node
 * @return None
 ******************************************************************************/
void RouteTB_ConvertModuleToRouteTable(route_table_t *entry, module_t *module)
{
    entry->type = module->vm->type;
    entry->id = module->vm->id;
    entry->mode = MODULE;
    for (int i = 0; i < MAX_ALIAS_SIZE; i++)
    {
        entry->alias[i] = module->alias[i];
    }
}
/******************************************************************************
 * @brief add a new module on the route_table
 * @param route table
 * @return None
 ******************************************************************************/
void RouteTB_InsertOnRouteTable(route_table_t *entry)
{
    memcpy(&route_table[last_route_table_entry++], entry, sizeof(route_table_t));
    if (entry->mode == MODULE)
    {
        last_module = entry->id;
    }
}
/******************************************************************************
 * @brief remove an entry from route_table
 * @param index of module
 * @return None
 ******************************************************************************/
void RouteTB_RemoveOnRouteTable(int index)
{
    route_table[index].alias[0] = '\0';
    route_table[index].type = VOID_MOD;
    route_table[index].id = 0;
    route_table[index].mode = CLEAR;
}
/******************************************************************************
 * @brief eras eroute_table
 * @param None
 * @return None
 ******************************************************************************/
void RouteTB_Erase(void)
{
    memset(route_table, 0, sizeof(route_table));
    last_module = 0;
    last_route_table_entry = 0;
}
/******************************************************************************
 * @brief get route_table
 * @param None
 * @return route table
 ******************************************************************************/
route_table_t *RouteTB_Get(void)
{
    return route_table;
}
/******************************************************************************
 * @brief return the last ID registered into the route_table
 * @param None
 * @return last module ID
 ******************************************************************************/
int8_t RouteTB_GetLastModule(void)
{
    return last_module;
}
/******************************************************************************
 * @brief return the last ID registered into the route_table
 * @param index of module
 * @return Last entry
 ******************************************************************************/
int8_t RouteTB_GetLastEntry(void)
{
    return last_route_table_entry;
}
