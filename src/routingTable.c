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
routing_table_t routing_table[MAX_CONTAINERS_NUMBER];
volatile int last_container = 0;
volatile int last_routing_table_entry = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void RoutingTB_AddNumToAlias(char *alias, int num);
static int8_t RoutingTB_BigestID(void);
static int8_t RoutingTB_WaitRoutingTable(container_t *container, msg_t *intro_msg);

// ************************ routing_table search tools ***************************

/******************************************************************************
 * @brief  Return an id from alias
 * @param pointer to alias
 * @return ID or Error
 ******************************************************************************/
int8_t RoutingTB_IDFromAlias(char *alias)
{
    if (*alias != -1)
    {
        for (int i = 0; i <= last_routing_table_entry; i++)
        {
            if (routing_table[i].mode == CONTAINER)
            {
                if (strcmp(routing_table[i].alias, alias) == 0)
                {
                    return routing_table[i].id;
                }
            }
        }
    }
    return -1;
}
/******************************************************************************
 * @brief  Return an id from type
 * @param type of container look at
 * @return ID or Error
 ******************************************************************************/
int8_t RoutingTB_IDFromType(luos_type_t type)
{
    for (int i = 0; i <= last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == CONTAINER)
        {
            if (type == routing_table[i].type)
            {
                return routing_table[i].id;
            }
        }
    }
    return -1;
}
/******************************************************************************
 * @brief  Return an id from container
 * @param container look at
 * @return ID or Error
 ******************************************************************************/
int8_t RoutingTB_IDFromContainer(container_t *container)
{
    // make sure route table is clean before sharing id
    if (last_routing_table_entry == 0)
    {
        return 0;
    }
    return (int)container->vm->id;
}
/******************************************************************************
 * @brief  Return container Alias from ID
 * @param id container look at
 * @return pointer to string or Error
 ******************************************************************************/
char *RoutingTB_AliasFromId(uint16_t id)
{
    for (int i = 0; i <= last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == CONTAINER)
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
 * @brief  Return container type from ID
 * @param id container look at
 * @return pointer container or Error
 ******************************************************************************/
luos_type_t RoutingTB_TypeFromID(uint16_t id)
{
    for (int i = 0; i <= last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == CONTAINER)
        {
            if (routing_table[i].id == id)
            {
                return routing_table[i].type;
            }
        }
    }
    return -1;
}
/******************************************************************************
 * @brief  Return container type from alias
 * @param alias container look at
 * @return pointer to container or Error
 ******************************************************************************/
luos_type_t RoutingTB_TypeFromAlias(char *alias)
{
    uint16_t id = RoutingTB_IDFromAlias(alias);
    return RoutingTB_TypeFromID(id);
}
/******************************************************************************
 * @brief  Create a string from a container type
 * @param type of container look at
 * @return pointer to string or Error
 ******************************************************************************/
char *RoutingTB_StringFromType(luos_type_t type)
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
 * @brief  check if the container is a sensor or not
 * @param container look at
 * @return Error
 ******************************************************************************/
uint8_t RoutingTB_ContainerIsSensor(luos_type_t type)
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
static int8_t RoutingTB_BigestID(void)
{
    int max_id = 0;
    for (int i = 0; i < last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == CONTAINER)
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
 * @brief  get number of a node on network
 * @param None
 * @return Error
 ******************************************************************************/
int8_t RoutingTB_GetNodeNB(void)
{
    int node_nb = 0;
    for (int i = 0; i <= last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == NODE)
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
void RoutingTB_GetNodeList(unsigned short *list)
{
    int node_nb = 0;
    for (int i = 0; i <= last_routing_table_entry; i++)
    {
        if (routing_table[i].mode == NODE)
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
int8_t RoutingTB_GetNodeID(unsigned short index)
{
    return routing_table[index + 1].id;
}

// ********************* routing_table management tools ************************

/******************************************************************************
 * @brief compute entry number
 * @param None
 * @return None
 ******************************************************************************/
void RoutingTB_ComputeRoutingTableEntryNB(void)
{
    for (int i = 0; i < MAX_CONTAINERS_NUMBER; i++)
    {
        if (routing_table[i].mode == CONTAINER)
        {
            last_container = routing_table[i].id;
        }
        if (routing_table[i].mode == CLEAR)
        {
            last_routing_table_entry = i;
            return;
        }
    }
}
/******************************************************************************
 * @brief manage container name increment to never have same alias
 * @param alias to change
 * @param nb to add
 * @return None
 ******************************************************************************/
static void RoutingTB_AddNumToAlias(char *alias, int num)
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
 * @param container receive
 * @param intro msg in route table
 * @return None
 ******************************************************************************/
static int8_t RoutingTB_WaitRoutingTable(container_t *container, msg_t *intro_msg)
{
    const int timeout = 15; // timeout in ms
    const int entry_bkp = last_routing_table_entry;
    Luos_SendMsg(container, intro_msg);
    uint32_t timestamp = LuosHAL_GetSystick();
    while ((LuosHAL_GetSystick() - timestamp) < timeout)
    {
        // If this request is for a container in this board allow him to respond.
        Luos_Loop();
        if (entry_bkp != last_routing_table_entry)
        {
            return 1;
        }
    }
    return 0;
}

unsigned char RoutingTB_ResetNetworkDetection(vm_t *vm)
{
    msg_t msg;

    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd = RESET_DETECTION;
    msg.header.size = 0;

    //we don't have any way to tell every containers to reset their detection do it twice to be sure
    if (Robus_SendMsg(vm, &msg))
        return 1;
    if (Robus_SendMsg(vm, &msg))
        return 1;
    // run luos_loop() to manage the 2 previous broadcast msgs.
    Luos_Loop();
    return 0;
}

static unsigned char RoutingTB_NetworkTopologyDetection(container_t *container)
{
    unsigned short newid = 1;
    // Reset all detection state of containers on the network
    RoutingTB_ResetNetworkDetection(container->vm);
    ctx.detection_mode = MASTER_DETECT;
    // wait for some us
    for (volatile unsigned int i = 0; i < (2 * TIMERVAL); i++)
        ;

    // setup sending vm
    container->vm->id = newid++;

    // Parse internal vm other than the sending one
    for (unsigned char i = 0; i < ctx.vm_number; i++)
    {
        if (&ctx.vm_table[i] != container->vm)
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
            int container_number = 0;
            while ((ctx.detection.keepline != NO_BRANCH) & (container_number < 1024))
            {
                if (Luos_SetExternId(container, IDACK, DEFAULTID, newid++))
                {
                    // set extern id fail
                    // remove this id and stop topology detection
                    newid--;
                    break;
                }
                container_number++;
                // wait for some us
                for (volatile unsigned int i = 0; i < (2 * TIMERVAL); i++)
                    ;
            }
        }
    }
    ctx.detection_mode = NO_DETECT;

    return newid - 1;
}

// Detect all containers and create a route table with it.
// If multiple containers have the same name it will be changed with a number in it automatically.
// At the end this function create a list of sensors id
void RoutingTB_DetectContainers(container_t *container)
{
    msg_t intro_msg, auto_name;
    unsigned char i = 0;
    // clear network detection state and all previous info.
    RoutingTB_Erase();
    // Starts the topology detection.
    int nb_mod = RoutingTB_NetworkTopologyDetection(container);
    if (nb_mod > MAX_CONTAINERS_NUMBER - 1)
    {
        nb_mod = MAX_CONTAINERS_NUMBER - 1;
    }

    // Then, asks for introduction for every found containers.
    int try_nb = 0;
    int last_id = RoutingTB_BigestID();
    while ((last_id < nb_mod) && (try_nb < nb_mod))
    {
        intro_msg.header.cmd = IDENTIFY_CMD;
        intro_msg.header.target_mode = IDACK;
        intro_msg.header.size = 0;
        // Target next unknown container (the first one of the next node)
        intro_msg.header.target = last_id + 1;
        try_nb++;
        // Ask to introduce and wait for a reply
        if (!RoutingTB_WaitRoutingTable(container, &intro_msg))
        {
            // We don't get the answer
            nb_mod = last_id;
            break;
        }
        last_id = RoutingTB_BigestID();
    }
    for (int id = 1; id <= nb_mod; id++)
    {
        int computed_id = RoutingTB_IDFromAlias(RoutingTB_AliasFromId(id));
        if ((computed_id != id) & (computed_id != -1))
        {
            int annotation = 1;
            // this name already exist in the network change it and send it back.
            // find the new alias to give him
            char aliasbis[15] = {0};
            memcpy(aliasbis, RoutingTB_AliasFromId(id), 15);
            RoutingTB_AddNumToAlias(RoutingTB_AliasFromId(id), annotation++);
            while (RoutingTB_IDFromAlias(RoutingTB_AliasFromId(id)) != id)
            {
                memcpy(RoutingTB_AliasFromId(id), aliasbis, 15);
                RoutingTB_AddNumToAlias(RoutingTB_AliasFromId(id), annotation++);
            }
            auto_name.header.target_mode = ID;
            auto_name.header.target = id;
            auto_name.header.cmd = WRITE_ALIAS;
            auto_name.header.size = strlen(RoutingTB_AliasFromId(id));
            // Copy the alias into the data field of the message
            for (i = 0; i < auto_name.header.size; i++)
            {
                auto_name.data[i] = RoutingTB_AliasFromId(id)[i];
            }
            auto_name.data[auto_name.header.size] = '\0';
            // Send the message using the WRITE_ALIAS system command
            Robus_SendMsg(container->vm, &auto_name);
            //TODO update name into routing_table
        }
    }

    // send route table to each nodes. Routing tables are commonly usable for each containers of a node.
    int nb_node = RoutingTB_GetNodeNB();
    unsigned short node_index_list[MAX_CONTAINERS_NUMBER];
    RoutingTB_GetNodeList(node_index_list);

    intro_msg.header.cmd = INTRODUCTION_CMD;
    intro_msg.header.target_mode = IDACK;

    for (int i = 1; i <= nb_node; i++)
    {
        intro_msg.header.target = RoutingTB_GetNodeID(node_index_list[i]);
        Luos_SendData(container, &intro_msg, routing_table, (last_routing_table_entry * sizeof(routing_table_t)));
    }
}
/******************************************************************************
 * @brief entry in routable node with associate container
 * @param route table
 * @param uuid node
 * @param node table
 * @param brach for node
 * @return None
 ******************************************************************************/
void RoutingTB_ConvertNodeToRoutingTable(routing_table_t *entry, luos_uuid_t uuid, unsigned short *port_table, int branch_nb)
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
 * @brief entry in routable container associate to a node
 * @param route table
 * @param container in node
 * @return None
 ******************************************************************************/
void RoutingTB_ConvertContainerToRoutingTable(routing_table_t *entry, container_t *container)
{
    entry->type = container->vm->type;
    entry->id = container->vm->id;
    entry->mode = CONTAINER;
    for (int i = 0; i < MAX_ALIAS_SIZE; i++)
    {
        entry->alias[i] = container->alias[i];
    }
}
/******************************************************************************
 * @brief add a new container on the routing_table
 * @param route table
 * @return None
 ******************************************************************************/
void RoutingTB_InsertOnRoutingTable(routing_table_t *entry)
{
    memcpy(&routing_table[last_routing_table_entry++], entry, sizeof(routing_table_t));
    if (entry->mode == CONTAINER)
    {
        last_container = entry->id;
    }
}
/******************************************************************************
 * @brief remove an entry from routing_table
 * @param index of container
 * @return None
 ******************************************************************************/
void RoutingTB_RemoveOnRoutingTable(int index)
{
    routing_table[index].alias[0] = '\0';
    routing_table[index].type = VOID_MOD;
    routing_table[index].id = 0;
    routing_table[index].mode = CLEAR;
}
/******************************************************************************
 * @brief eras erouting_table
 * @param None
 * @return None
 ******************************************************************************/
void RoutingTB_Erase(void)
{
    memset(routing_table, 0, sizeof(routing_table));
    last_container = 0;
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
 * @return last container ID
 ******************************************************************************/
int8_t RoutingTB_GetLastContainer(void)
{
    return last_container;
}
/******************************************************************************
 * @brief return the last ID registered into the routing_table
 * @param index of container
 * @return Last entry
 ******************************************************************************/
int8_t RoutingTB_GetLastEntry(void)
{
    return last_routing_table_entry;
}
