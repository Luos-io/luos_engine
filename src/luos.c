/******************************************************************************
 * @file luos
 * @brief User functionalities of the Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos.h"

#include <stdio.h>
#include "message_mngr.h"
#include <luosHAL.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s
/*******************************************************************************
 * Variables
 ******************************************************************************/
static module_t *luos_module_pointer;
static volatile msg_t luos_pub_msg;
static volatile int luos_pub = LUOS_PROTOCOL_NB;
module_t module_table[MAX_VM_NUMBER];
unsigned char module_number;
volatile route_table_t *route_table_pt;
/*******************************************************************************
 * Function
 ******************************************************************************/
static int8_t Luos_MsgHandler(module_t *module, msg_t *input, msg_t *output);
static void Luos_CallbackMsg(vm_t *vm, msg_t *msg);
static module_t *Luos_GetModule(vm_t *vm);
static int8_t Luos_GetModuleIndex(module_t *module);
static void Luos_TransmitLocalRouteTable(void);
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
    Robus_Init(Luos_CallbackMsg);
}
/******************************************************************************
 * @brief Luos Loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Loop(void)
{
    mngr_t chunk;
    if (luos_pub != LUOS_PROTOCOL_NB)
    {
        if (luos_pub == IDENTIFY_CMD)
        {
            Luos_TransmitLocalRouteTable();
        }
        else
        {
            Luos_SendMsg(luos_module_pointer, (msg_t *)&luos_pub_msg);
        }
        luos_pub = LUOS_PROTOCOL_NB;
    }
    // filter stacked module with callback
    int i = Mngr_GetNextCallbackID();
    while (i >= 0)
    {
        Mngr_GetIndexMsg(i, 0, &chunk);
        chunk.module->mod_cb(chunk.module, chunk.msg);
        i = Mngr_GetNextCallbackID();
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
static int8_t Luos_MsgHandler(module_t *module, msg_t *input, msg_t *output)
{
    if (input->header.cmd == IDENTIFY_CMD)
    {
        // someone request a local route table
        // Just create an empty message, it will be filled in Luos_loop.
        output->header.cmd = INTRODUCTION_CMD;
        output->header.target_mode = IDACK;
        output->header.target = input->header.source;
        luos_pub = IDENTIFY_CMD;
        return 1;
    }
    if (input->header.cmd == INTRODUCTION_CMD)
    {
        volatile route_table_t *route_tab = &RouteTB_Get()[RouteTB_GetLastEntry()];
        if (Luos_ReceiveData(module, input, (void *)route_tab))
        {
            // route table of this board is finish
            RouteTB_ComputeRouteTableEntryNB();
        }
        return 1;
    }
    if ((input->header.cmd == REVISION) & (input->header.size == 0))
    {
        output->header.cmd = REVISION;
        output->header.target_mode = ID;
        sprintf((char *)output->data, "%s", module->firm_version);
        memcpy(output->data, module->firm_version, sizeof(output->data));
        output->header.size = strlen((char *)output->data);
        output->header.target = input->header.source;
        luos_pub = REVISION;
        return 1;
    }
    if ((input->header.cmd == LUOS_REVISION) & (input->header.size == 0))
    {
        output->header.cmd = LUOS_REVISION;
        output->header.target_mode = ID;
        const char *luos_version = STRINGIFY(VERSION);
        sprintf((char *)output->data, "%s", luos_version);
        memcpy(output->data, luos_version, sizeof(output->data));
        output->header.size = strlen((char *)output->data);
        output->header.target = input->header.source;
        luos_pub = LUOS_REVISION;
        return 1;
    }
    if ((input->header.cmd == NODE_UUID) & (input->header.size == 0))
    {
        output->header.cmd = NODE_UUID;
        output->header.target_mode = ID;
        output->header.size = sizeof(luos_uuid_t);
        output->header.target = input->header.source;
        luos_uuid_t uuid;
        uuid.uuid[0] = LUOS_UUID[0];
        uuid.uuid[1] = LUOS_UUID[1];
        uuid.uuid[2] = LUOS_UUID[2];
        memcpy(output->data, &uuid.unmap, sizeof(luos_uuid_t));
        luos_pub = NODE_UUID;
        return 1;
    }
    if (input->header.cmd == WRITE_ALIAS)
    {
        // Make a clean copy with full \0 at the end.
        memset(module->alias, '\0', sizeof(module->alias));
        if (input->header.size > 16)
            input->header.size = 16;
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
    }
    if (input->header.cmd == UPDATE_PUB)
    {
        // this module need to be auto updated
        time_luos_t time;
        TimeOD_TimeFromMsg(&time, input);
        module->auto_refresh.target = input->header.source;
        module->auto_refresh.time_ms = (uint16_t)TimeOD_TimeTo_ms(time);
        module->auto_refresh.last_update = LuosHAL_GetSystick();
        return 1;
    }

    return 0;
}
/******************************************************************************
 * @brief callback for msg need to be rename will be unstack msg
 * @param None
 * @return None
 ******************************************************************************/
static void Luos_CallbackMsg(vm_t *vm, msg_t *msg)
{
    // Luos message management
    volatile module_t *module = Luos_GetModule(vm);
    if (module == 0)
    {
        // module overwrited, it seem to be possible if ring_buffer overflow.
        while (1)
            ;
    }
    if (Luos_MsgHandler((module_t *)module, msg, (msg_t *)&luos_pub_msg))
    {
        luos_module_pointer = (module_t *)module;
        return;
    }
    if ((module->rt >= 1) & (module->mod_cb != 0))
    {
        module->mod_cb((module_t *)module, msg);
    }
    else
    {
        //store module and msg pointer
        Mngr_SetIndexMsg((module_t *)module, msg);
    }
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
static void Luos_TransmitLocalRouteTable(void)
{
    // We receive this command because someone creating a new route table
    // Reset the actual route table
    RouteTB_Erase();
    volatile int entry_nb = 0;
    volatile route_table_t local_route_table[module_number + 1];
    //start by saving board entry
    luos_uuid_t uuid;
    uuid.uuid[0] = LUOS_UUID[0];
    uuid.uuid[1] = LUOS_UUID[1];
    uuid.uuid[2] = LUOS_UUID[2];
    unsigned char table_size;
    uint16_t *detection_branches = Robus_GetNodeBranches(&table_size);
    RouteTB_ConvertNodeToRouteTable((route_table_t *)&local_route_table[entry_nb++], uuid, detection_branches, table_size);
    // save modules entry
    for (int i = 0; i < module_number; i++)
    {
        RouteTB_ConvertModuleToRouteTable((route_table_t *)&local_route_table[entry_nb++], &module_table[i]);
    }
    Luos_SendData(luos_module_pointer, (msg_t *)&luos_pub_msg, (void *)local_route_table, (entry_nb * sizeof(route_table_t)));
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
    module->message_available = 0;

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
 * @return received message pointer
 ******************************************************************************/
msg_t *Luos_ReadMsg(module_t *module)
{
    if (module->message_available > MSG_BUFFER_SIZE)
    {
        // msg read too slow
    }
    if (module->message_available)
    {
        // check if there is a message for this module
        int i = Mngr_GetNextModuleID(module);
        if (i >= 0)
        {
            // this module have a message, get it
            mngr_t chunk;
            Mngr_GetIndexMsg(i, 0, &chunk);
            return chunk.msg;
        }
    }
    return 0;
}
/******************************************************************************
 * @brief read last msg from buffer from a special id module
 * @param module who receive the message we are looking for
 * @param id who sent the message we are looking for
 * @return received message pointer
 ******************************************************************************/
msg_t *Luos_ReadFromModule(module_t *module, short id)
{
    if (module->message_available)
    {
        // Get the next message manager id containing something for this module
        int mngr_module_id = Mngr_GetNextModuleID(module);
        if (mngr_module_id >= 0)
        {
            // check if there is a message from this id in this module
            int mngr_msg_id = Mngr_GetNextMsgID(mngr_module_id, id);
            if (mngr_msg_id >= 0)
            {
                mngr_t chunk;
                Mngr_GetIndexMsg(mngr_module_id, mngr_msg_id, &chunk);
                return chunk.msg;
            }
        }
    }
    return 0;
}
/******************************************************************************
 * @brief how many messages are available
 * @param none
 * @return the number of message received
 ******************************************************************************/
uint8_t Luos_NbrAvailableMsg(void)
{
    return Mngr_AvailableMessage();
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
    int8_t i = (int8_t)(Luos_GetModule(&module->vm));
    if (i >= 0)
    {
        Luos_WriteAlias(i, alias);
    }
    return i;
}
/******************************************************************************
 * @brief write alias in flash
 * @param position in the route table
 * @param alias to store
 * @return error
 ******************************************************************************/
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
    Robus_SetBaudrate(module->vm, baudrate);
}
