/******************************************************************************
 * @file luos
 * @brief User functionalities of the Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos.h"

#include <stdio.h>
#include "context.h"  // TODO remove it
#include "sys_msg.h"  // TODO remove it
#include "msgAlloc.h" // TODO this is not really clean
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s

/*******************************************************************************
 * Variables
 ******************************************************************************/
module_t module_table[MAX_VM_NUMBER];
unsigned char module_number;
volatile route_table_t *route_table_pt;
/*******************************************************************************
 * Function
 ******************************************************************************/
static int8_t Luos_MsgHandler(module_t *module, msg_t *input);
static module_t *Luos_GetModule(vm_t *vm);
static int8_t Luos_GetModuleIndex(module_t *module);
static void Luos_TransmitLocalRouteTable(module_t *module, msg_t *routeTB_msg);
static void Luos_AutoUpdateManager(void);
static uint8_t Luos_SaveAlias(module_t *module, char *alias);
static void Luos_WriteAlias(unsigned short local_id, char *alias);
static char Luos_ReadAlias(unsigned short local_id, char *alias);

/******************************************************************************
 * @brief Luos init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Init(void)
{
    module_number = 0;
    Robus_Init();
}
/******************************************************************************
 * @brief Luos Loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Loop(void)
{
    uint16_t remaining_msg_number = 0;
    vm_t *oldest_vm = NULL;
    msg_t *unconsumed_msg = NULL;
    msg_t *returned_msg = NULL;
    // look at all received messages
    while (MsgAlloc_LookAtAllocatorSlot(remaining_msg_number, &oldest_vm, &unconsumed_msg) != FAIL)
    {
        // There is a message available find the module linked to it
        module_t *module = Luos_GetModule(oldest_vm);
        // Is this module having a callback?
        if (module->mod_cb != 0)
        {
            // This module have a callback pull the message
            if (MsgAlloc_PullMsg(oldest_vm, &returned_msg) == SUCESS)
            {
                // check if the content of this message need to be managed by Luos and do it if it is.
                if (!Luos_MsgHandler((module_t *)module, returned_msg))
                {
                    // This message is for the user, pass it to the user.
                    module->mod_cb(module, returned_msg);
                }
            }
        }
        else
        {
            //check if this msg cmd should be consumed by Luos_MsgHandler
            if (Luos_MsgHandler((module_t *)module, unconsumed_msg))
            {
                // Luos_MsgHandler use it. clear this slot.
                MsgAlloc_PullMsg(oldest_vm, &returned_msg);
            }
            else
            {
                remaining_msg_number++;
            }
        }
    }
    // manage timed auto update
    Luos_AutoUpdateManager();
}
/******************************************************************************
 * @brief handling msg for Luos library
 * @param module
 * @param input msg
 * @param output msg
 * @return None
 ******************************************************************************/
static int8_t Luos_MsgHandler(module_t *module, msg_t *input)
{
    uint32_t baudrate;
    msg_t routeTB_msg;
    route_table_t *route_tab = &RouteTB_Get()[RouteTB_GetLastEntry()];
    time_luos_t time;
    if (input->header.target_mode == ID ||
        input->header.target_mode == IDACK ||
        input->header.target_mode == TYPE ||
        input->header.target_mode == BROADCAST)
    {
        switch (input->header.cmd)
        {
        case WRITE_ID:
            if (ctx.detection.activ_branch == NO_BRANCH)
            {
                // Get and save a new given ID
                if ((ctx.vm_table[ctx.detection.detected_vm].id == DEFAULTID) &
                    (ctx.detection.keepline != NO_BRANCH) &
                    (ctx.detection_mode != MASTER_DETECT))
                {
                    if (input->header.target_mode == IDACK)
                    {
                        // Acknoledge ID reception
                        Transmit_SendAck();
                    }
                    // We are on topology detection mode, and this is our turn
                    // Save id for the next module we have on this board
                    ctx.vm_table[ctx.detection.detected_vm++].id =
                        (((unsigned short)input->data[1]) |
                         ((unsigned short)input->data[0] << 8));
                    if (ctx.detection.detected_vm == 1)
                    {
                        // This is the first internal module, save the input branch with the previous ID
                        ctx.detection.branches[ctx.detection.keepline] = ctx.vm_table[0].id - 1;
                    }
                    // Check if that was the last virtual module
                    if (ctx.detection.detected_vm >= ctx.vm_number)
                    {
                        Detect_PokeNextBranch();
                    }
                }
                else if (input->header.target != DEFAULTID)
                {
                    module->vm->id = (((unsigned short)input->data[1]) |
                                      ((unsigned short)input->data[0] << 8));
                }
            }
            else
            {
                unsigned short value = (((unsigned short)input->data[1]) |
                                        ((unsigned short)input->data[0] << 8));
                //We need to save this ID as a connection on a branch
                ctx.detection.branches[ctx.detection.activ_branch] = value;
                ctx.detection.activ_branch = NO_BRANCH;
            }
            return 1;
            break;
        case RESET_DETECTION:
            Detec_InitDetection();
            return 1;
            break;
        case SET_BAUDRATE:
            memcpy(&baudrate, input->data, input->header.size);
            Luos_SetBaudrate(module, baudrate);
            return 1;
            break;

        case IDENTIFY_CMD:
            // someone request a local route table
            routeTB_msg.header.cmd = INTRODUCTION_CMD;
            routeTB_msg.header.target_mode = IDACK;
            routeTB_msg.header.target = input->header.source;
            Luos_TransmitLocalRouteTable(module, &routeTB_msg);
            return 1;
            break;
        case INTRODUCTION_CMD:
            if (Luos_ReceiveData(module, input, (void *)route_tab))
            {
                // route table of this board is finish
                RouteTB_ComputeRouteTableEntryNB();
            }
            return 1;
            break;
        case REVISION:
            if (input->header.size == 0)
            {
                msg_t output;
                output.header.cmd = REVISION;
                output.header.target_mode = ID;
                sprintf((char *)output.data, "%s", module->firm_version);
                memcpy(output.data, module->firm_version, sizeof(output.data));
                output.header.size = strlen((char *)output.data);
                output.header.target = input->header.source;
                Luos_SendMsg(module, &output);
                return 1;
            }
            break;
        case LUOS_REVISION:
            if (input->header.size == 0)
            {
                msg_t output;
                output.header.cmd = LUOS_REVISION;
                output.header.target_mode = ID;
                const char *luos_version = STRINGIFY(VERSION);
                sprintf((char *)output.data, "%s", luos_version);
                memcpy(output.data, luos_version, sizeof(output.data));
                output.header.size = strlen((char *)output.data);
                output.header.target = input->header.source;
                Luos_SendMsg(module, &output);
                return 1;
            }
            break;
        case NODE_UUID:
            if (input->header.size == 0)
            {
                msg_t output;
                output.header.cmd = NODE_UUID;
                output.header.target_mode = ID;
                output.header.size = sizeof(luos_uuid_t);
                output.header.target = input->header.source;
                luos_uuid_t uuid;
                uuid.uuid[0] = LUOS_UUID[0];
                uuid.uuid[1] = LUOS_UUID[1];
                uuid.uuid[2] = LUOS_UUID[2];
                memcpy(output.data, &uuid.unmap, sizeof(luos_uuid_t));
                Luos_SendMsg(module, &output);
                return 1;
            }
            break;
        case WRITE_ALIAS:
            // Make a clean copy with full \0 at the end.
            memset(module->alias, '\0', sizeof(module->alias));
            if (input->header.size > 16)
            {
                input->header.size = 16;
            }
            if ((((input->data[0] >= 'A') & (input->data[0] <= 'Z')) | ((input->data[0] >= 'a') & (input->data[0] <= 'z')) | (input->data[0] == '\0')) & (input->header.size != 0))
            {
                memcpy(module->alias, input->data, input->header.size);
                Luos_SaveAlias(module, module->alias);
            }
            else
            {
                // This is an alias erase instruction, get back to default one
                Luos_SaveAlias(module, '\0');
                memcpy(module->alias, module->default_alias, MAX_ALIAS_SIZE);
            }
            return 1;
            break;
        case UPDATE_PUB:
            // this module need to be auto updated
            TimeOD_TimeFromMsg(&time, input);
            module->auto_refresh.target = input->header.source;
            module->auto_refresh.time_ms = (uint16_t)TimeOD_TimeTo_ms(time);
            module->auto_refresh.last_update = LuosHAL_GetSystick();
            return 1;
            break;
        default:
            return 0;
            break;
        }
    }
    return 0;
}
/******************************************************************************
 * @brief get pointer to a module in route table
 * @param vm
 * @return module from list
 ******************************************************************************/
static module_t *Luos_GetModule(vm_t *vm)
{
    for (int8_t i = 0; i < module_number; i++)
    {
        if (vm == module_table[i].vm)
        {
            return &module_table[i];
        }
    }
    return 0;
}
/******************************************************************************
 * @brief get this index of the module
 * @param module
 * @return module from list
 ******************************************************************************/
static int8_t Luos_GetModuleIndex(module_t *module)
{
    for (int8_t i = 0; i < module_number; i++)
    {
        if (module == &module_table[i])
        {
            return i;
        }
    }
    return -1;
}
/******************************************************************************
 * @brief transmit local to network
 * @param none
 * @return none
 ******************************************************************************/
static void Luos_TransmitLocalRouteTable(module_t *module, msg_t *routeTB_msg)
{
    // We receive this command because someone creating a new route table
    // Reset the actual route table
    RouteTB_Erase();
    uint16_t entry_nb = 0;
    route_table_t local_route_table[module_number + 1];
    //start by saving board entry
    luos_uuid_t uuid;
    uuid.uuid[0] = LUOS_UUID[0];
    uuid.uuid[1] = LUOS_UUID[1];
    uuid.uuid[2] = LUOS_UUID[2];
    unsigned char table_size;
    uint16_t *detection_branches = Robus_GetNodeBranches(&table_size);
    RouteTB_ConvertNodeToRouteTable(&local_route_table[entry_nb], uuid, detection_branches, table_size);
    entry_nb++;
    // save modules entry
    for (int i = 0; i < module_number; i++)
    {
        RouteTB_ConvertModuleToRouteTable((route_table_t *)&local_route_table[entry_nb++], &module_table[i]);
    }
    Luos_SendData(module, routeTB_msg, (void *)local_route_table, (entry_nb * sizeof(route_table_t)));
}
/******************************************************************************
 * @brief auto update publication for module
 * @param none
 * @return none
 ******************************************************************************/
static void Luos_AutoUpdateManager(void)
{
    // check all modules timed_update_t contexts
    for (int i = 0; i < module_number; i++)
    {
        // check if modules have an actual ID. If not, we are in detection mode and should reset the auto refresh
        if (module_table[i].vm->id == DEFAULTID)
        {
            // this module have not been detected or is in detection mode. remove auto_refresh parameters
            module_table[i].auto_refresh.target = 0;
            module_table[i].auto_refresh.time_ms = 0;
            module_table[i].auto_refresh.last_update = 0;
        }
        else
        {
            // check if there is a timed update setted and if it's time to update it.
            if (module_table[i].auto_refresh.time_ms)
            {
                if ((LuosHAL_GetSystick() - module_table[i].auto_refresh.last_update) >= module_table[i].auto_refresh.time_ms)
                {
                    // This module need to send an update
                    // Create a fake message for it from the module asking for update
                    msg_t updt_msg;
                    updt_msg.header.target = module_table[i].vm->id;
                    updt_msg.header.source = module_table[i].auto_refresh.target;
                    updt_msg.header.target_mode = IDACK;
                    updt_msg.header.cmd = ASK_PUB_CMD;
                    updt_msg.header.size = 0;
                    if ((module_table[i].mod_cb != 0))
                    {
                        module_table[i].mod_cb(&module_table[i], &updt_msg);
                    }
                    else
                    {
                        //store module and msg pointer
                        // todo this can't work for now because this message is not permanent.
                        //mngr_set(&module_table[i], &updt_msg);
                    }
                    module_table[i].auto_refresh.last_update = LuosHAL_GetSystick();
                }
            }
        }
    }
}
/******************************************************************************
 * @brief clear list of module
 * @param none
 * @return none
 ******************************************************************************/
void Luos_ModulesClear(void)
{
    module_number = 0;
    Robus_ModulesClear();
}
/******************************************************************************
 * @brief API to Create a module
 * @param callback msg handler for the module
 * @param type of module corresponding to object dictionnary
 * @param alias for the module string (15 caracters max).
 * @param version FW for the module
 * @return module object pointer.
 ******************************************************************************/
module_t *Luos_CreateModule(MOD_CB mod_cb, uint8_t type, const char *alias, char *firm_revision)
{
    unsigned char i = 0;
    module_t *module = &module_table[module_number];
    module->vm = Robus_ModuleCreate(type);
    module->rt = 0;

    // Link the module to his callback
    module->mod_cb = mod_cb;
    // Save default alias
    for (i = 0; i < MAX_ALIAS_SIZE - 1; i++)
    {
        module->default_alias[i] = alias[i];
        if (module->default_alias[i] == '\0')
            break;
    }
    module->default_alias[i] = '\0';
    // Initialise the module alias to 0
    memset((void *)module->alias, 0, sizeof(module->alias));
    if (!Luos_ReadAlias(module_number, (char *)module->alias))
    {
        // if no alias saved keep the default one
        for (i = 0; i < MAX_ALIAS_SIZE - 1; i++)
        {
            module->alias[i] = alias[i];
            if (module->alias[i] == '\0')
                break;
        }
        module->alias[i] = '\0';
    }

    //Initialise the module firm_version to 0
    memset((void *)module->firm_version, 0, sizeof(module->firm_version));
    // Save firmware version
    for (i = 0; i < 20; i++)
    {
        module->firm_version[i] = firm_revision[i];
        if (module->firm_version[i] == '\0')
            break;
    }

    module_number++;
    return module;
}
/******************************************************************************
 * @brief Real time module will be change to real time msg
 * @param none
 * @return module
 ******************************************************************************/
void Luos_ModuleEnableRT(module_t *module)
{
    module->rt = 1;
}
/******************************************************************************
 * @brief Send msg through network
 * @param Module who send
 * @param Message to send
 * @return error
 ******************************************************************************/
uint8_t Luos_SendMsg(module_t *module, msg_t *msg)
{
    return Robus_SendMsg(module->vm, msg);
}
/******************************************************************************
 * @brief read last msg from buffer for a module
 * @param module who receive the message we are looking for
 * @param returned_msg oldest message of the module
 * @return FAIL if no message available
 ******************************************************************************/
error_return_t Luos_ReadMsg(module_t *module, msg_t **returned_msg)
{
    error_return_t error = SUCESS;
    while (error == SUCESS)
    {
        error = MsgAlloc_PullMsg(module->vm, returned_msg);
        // check if the content of this message need to be managed by Luos and do it if it is.
        if ((!Luos_MsgHandler(module, *returned_msg)) & (error == SUCESS))
        {
            // This message is for the user, pass it to the user.
            return SUCESS;
        }
    }
    return FAIL;
}

/******************************************************************************
 * @brief read last msg from buffer from a special id module
 * @param module who receive the message we are looking for
 * @param id who sent the message we are looking for
 * @param returned_msg oldest message of the module
 * @return FAIL if no message available
 ******************************************************************************/
error_return_t Luos_ReadFromModule(module_t *module, short id, msg_t **returned_msg)
{
    uint16_t remaining_msg_number = 0;
    vm_t *oldest_vm = NULL;
    msg_t *source_id_msg = NULL;
    error_return_t error = SUCESS;
    while (MsgAlloc_LookAtAllocatorSlot(remaining_msg_number, &oldest_vm, &source_id_msg) != FAIL)
    {
        // Check the source id
        if ((oldest_vm == module->vm) & (source_id_msg->header.source == id))
        {
            // Source id of this message match, get it and treat it.
            error = MsgAlloc_PullMsg(module->vm, returned_msg);
            // check if the content of this message need to be managed by Luos and do it if it is.
            if ((!Luos_MsgHandler(module, *returned_msg)) & (error == SUCESS))
            {
                // This message is for the user, pass it to the user.
                return SUCESS;
            }
        }
        else
        {
            remaining_msg_number++;
        }
    }
    return FAIL;
}
/******************************************************************************
 * @brief Send large among of data and formating to send into multiple msg
 * @param Module who send
 * @param Message to send
 * @param Pointer to the message data table
 * @param Size of the data to transmit
 * @return error
 ******************************************************************************/
uint8_t Luos_SendData(module_t *module, msg_t *msg, void *bin_data, unsigned short size)
{
    // Compute number of message needed to send this data
    int msg_number = 1;
    int sent_size = 0;
    if (size > MAX_DATA_MSG_SIZE)
    {
        msg_number = (size / MAX_DATA_MSG_SIZE);
        msg_number += (msg_number * MAX_DATA_MSG_SIZE < size);
    }

    // Send messages one by one
    for (volatile int chunk = 0; chunk < msg_number; chunk++)
    {
        // Compute chunk size
        int chunk_size = 0;
        if ((size - sent_size) > MAX_DATA_MSG_SIZE)
            chunk_size = MAX_DATA_MSG_SIZE;
        else
            chunk_size = size - sent_size;

        // Copy data into message
        memcpy(msg->data, (char *)bin_data + sent_size, chunk_size);
        msg->header.size = size - sent_size;

        // Send message
        if (Luos_SendMsg(module, msg))
        {
            // This message fail stop transmission and return an error
            return 1;
        }

        // Save current state
        sent_size = sent_size + chunk_size;
    }
    return 0;
}
/******************************************************************************
 * @brief receive a multi msg data
 * @param Module who receive
 * @param Message chunk received
 * @param pointer to data
 * @return error
 ******************************************************************************/
uint8_t Luos_ReceiveData(module_t *module, msg_t *msg, void *bin_data)
{
    // Manage buffer session (one per module)
    static uint32_t data_size[MAX_VM_NUMBER] = {0};
    static int last_msg_size = 0;
    int id = Luos_GetModuleIndex(module);

    // check message integrity
    if ((last_msg_size > 0) && (last_msg_size - MAX_DATA_MSG_SIZE > msg->header.size))
    {
        // we miss a message (a part of the data),
        // reset session and return an error.
        data_size[id] = 0;
        last_msg_size = 0;
        return 2;
    }

    // Get chunk size
    unsigned short chunk_size = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
        chunk_size = MAX_DATA_MSG_SIZE;
    else
        chunk_size = msg->header.size;

    // Copy data into buffer
    memcpy((char *)bin_data + data_size[id], msg->data, chunk_size);

    // Save buffer session
    data_size[id] = data_size[id] + chunk_size;
    last_msg_size = msg->header.size;

    // Check end of data
    if (!(msg->header.size > MAX_DATA_MSG_SIZE))
    {
        // Data collection finished, reset buffer session state
        data_size[id] = 0;
        last_msg_size = 0;
        return 1;
    }
    return 0;
}
/******************************************************************************
 * @brief Send datas of a streaming channel
 * @param Module who send
 * @param Message to send
 * @param streaming channel pointer
 * @return error
 ******************************************************************************/
uint8_t Luos_SendStreaming(module_t *module, msg_t *msg, streaming_channel_t *stream)
{
    // Compute number of message needed to send available datas on ring buffer
    int msg_number = 1;
    int data_size = Stream_GetAvailableSampleNB(stream);
    const int max_data_msg_size = (MAX_DATA_MSG_SIZE / stream->data_size);
    if (data_size > max_data_msg_size)
    {
        msg_number = (data_size / max_data_msg_size);
        msg_number += ((msg_number * max_data_msg_size) < data_size);
    }

    // Send messages one by one
    for (volatile int chunk = 0; chunk < msg_number; chunk++)
    {
        // compute chunk size
        int chunk_size = 0;
        if (data_size > max_data_msg_size)
            chunk_size = max_data_msg_size;
        else
            chunk_size = data_size;

        // Copy data into message
        Stream_GetSample(stream, msg->data, chunk_size);

        // Send message
        if (Luos_SendMsg(module, msg))
        {
            // this message fail stop transmission, retrieve datas and return an error
            stream->sample_ptr = stream->sample_ptr - (chunk_size * stream->data_size);
            if (stream->sample_ptr < stream->ring_buffer)
            {
                stream->sample_ptr = stream->end_ring_buffer - (stream->ring_buffer - stream->sample_ptr);
            }
            return 1;
        }

        // check end of data
        if (data_size > max_data_msg_size)
        {
            data_size -= max_data_msg_size;
        }
        else
        {
            data_size = 0;
        }
    }
    return 0;
}
/******************************************************************************
 * @brief Receive a streaming channel datas
 * @param Module who send
 * @param Message to send
 * @param streaming channel pointer
 * @return error
 ******************************************************************************/
uint8_t Luos_ReceiveStreaming(module_t *module, msg_t *msg, streaming_channel_t *stream)
{
    // Get chunk size
    unsigned short chunk_size = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
        chunk_size = MAX_DATA_MSG_SIZE;
    else
        chunk_size = msg->header.size;

    // Copy data into buffer
    Stream_PutSample(stream, msg->data, (chunk_size / stream->data_size));

    // Check end of data
    if ((msg->header.size <= MAX_DATA_MSG_SIZE))
    {
        // Chunk collection finished
        return 1;
    }
    return 0;
}
/******************************************************************************
 * @brief store alias name module in flash
 * @param module to store
 * @param alias to store
 * @return error
 ******************************************************************************/
static uint8_t Luos_SaveAlias(module_t *module, char *alias)
{
    // Get module index
    int8_t i = (int8_t)(Luos_GetModuleIndex(module));
    if (i >= 0)
    {
        Luos_WriteAlias(i, alias);
    }
    return i;
}

static void Luos_WriteAlias(unsigned short local_id, char *alias)
{
    uint32_t addr = ADDRESS_ALIASES_FLASH + (local_id * (MAX_ALIAS_SIZE + 1));
    LuosHAL_FlashWriteLuosMemoryInfo(addr, 16, (uint8_t *)alias);
}
/******************************************************************************
 * @brief read alias from flash
 * @param position in the route table
 * @param alias to store
 * @return error
 ******************************************************************************/
static char Luos_ReadAlias(unsigned short local_id, char *alias)
{
    uint32_t addr = ADDRESS_ALIASES_FLASH + (local_id * (MAX_ALIAS_SIZE + 1));
    LuosHAL_FlashReadLuosMemoryInfo(addr, 16, (uint8_t *)alias);
    // Check name integrity
    if ((((alias[0] < 'A') | (alias[0] > 'Z')) & ((alias[0] < 'a') | (alias[0] > 'z'))) | (alias[0] == '\0'))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/******************************************************************************
 * @brief set network bauderate
 * @param module sending request
 * @param baudrate
 * @return None
 ******************************************************************************/
void Luos_SetBaudrate(module_t *module, uint32_t baudrate)
{
    msg_t msg;
    memcpy(msg.data, &baudrate, sizeof(unsigned int));
    msg.header.target_mode = BROADCAST;
    msg.header.target = BROADCAST_VAL;
    msg.header.cmd = SET_BAUDRATE;
    msg.header.size = sizeof(unsigned int);
    Robus_SendMsg(module->vm, &msg);
}
/******************************************************************************
 * @brief set id of a module trough the network
 * @param module sending request
 * @param target_mode
 * @param target
 * @param newid : The new Id of module(s)
 * @return None
 ******************************************************************************/
uint8_t Luos_SetExternId(module_t *module, target_mode_t target_mode, uint16_t target, uint16_t newid)
{
    msg_t msg;
    msg.header.target = target;
    msg.header.target_mode = target_mode;
    msg.header.cmd = WRITE_ID;
    msg.header.size = 2;
    msg.data[1] = newid;
    msg.data[0] = (newid << 8);
    if (Robus_SendMsg(module->vm, &msg))
    {
        return 1;
    }
    return 0;
}
/******************************************************************************
 * @brief return the number of messages available
 * @param None
 * @return the number of messages
 ******************************************************************************/
uint16_t Luos_NbrAvailableMsg(void)
{
    return MsgAlloc_AllocNbr();
}