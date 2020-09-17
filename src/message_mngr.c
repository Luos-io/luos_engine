/******************************************************************************
 * @file message_mngr
 * @brief function relative to message processing
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "message_mngr.h"

#include <stdbool.h>
#include <luosHAL.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
// no real time callback management
volatile int module_msg_available = 0;
volatile module_t *module_msg_mngr[MSG_BUFFER_SIZE];
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief nbr of msg available in buffer
 * @param None
 * @return None
 ******************************************************************************/
uint8_t Mngr_AvailableMessage(void)
{
    return module_msg_available;
}
/******************************************************************************
 * @brief  index module and pointer to msg
 * @param module to index
 * @param pointer to msg index
 * @return None
 ******************************************************************************/
void Mngr_SetIndexMsg(module_t *module, msg_t *msg)
{
    // Todo check if this message address is already used in the Luos stack.
    // Todo Watch out the next one on Robus could be corrupted because it is used to receive the next message...
    // Todo perhaps we could track the currently used message slot on robus : https://community.luos-robotics.com/t/buffering-overflow-resuling-on-strange-message-reception/233
    if ((module_msg_available + 1 < MSG_BUFFER_SIZE) && (module->message_available + 1 < MSG_BUFFER_SIZE))
    {
        module_msg_mngr[module_msg_available++] = module;
        module->msg_stack[module->message_available++] = msg;
    }
    else
    {
        // out of buffer. remove the oldest message and add this new one.
        mngr_t trash;
        Mngr_GetIndexMsg(0, 0, &trash);
        module_msg_mngr[module_msg_available++] = module;
        module->msg_stack[module->message_available++] = msg;
    }
}
/******************************************************************************
 * @brief  from list get index and module for a msg
 * @param module index look at
 * @param msg index look at
 * @param chunk
 * @return None
 ******************************************************************************/
void Mngr_GetIndexMsg(int module_index, int msg_index, mngr_t *chunk)
{
    int i;
    if ((module_index < 0) | (msg_index < 0))
    {
        return;
    }
    // get module
    chunk->module = (module_t *)module_msg_mngr[module_index];
    LuosHAL_SetIrqState(false);
    for (i = module_index; i < module_msg_available; i++)
    {
        module_msg_mngr[i] = module_msg_mngr[i + 1];
    }
    module_msg_mngr[i] = 0;
    module_msg_available--;
    LuosHAL_SetIrqState(true);

    // get msg
    chunk->msg = chunk->module->msg_stack[msg_index];
    LuosHAL_SetIrqState(false);
    for (int i = msg_index; i < chunk->module->message_available; i++)
    {
        chunk->module->msg_stack[i] = chunk->module->msg_stack[i + 1];
    }
    chunk->module->msg_stack[i] = 0;
    chunk->module->message_available--;
    LuosHAL_SetIrqState(true);
}

/******************************************************************************
 * @brief  find if there is a callback in this module return this id
 * @param None
 * @return ID module with a callback
 ******************************************************************************/
int8_t Mngr_GetNextCallbackID(void)
{
    for (int i = 0; i < module_msg_available; i++)
    {
        if (module_msg_mngr[i]->mod_cb)
        {
            //
            return i;
        }
    }
    return -1;
}
/******************************************************************************
 * @brief  find the next message for a specific module
 * @param module to look at
 * @return ID module
 ******************************************************************************/
int8_t Mngr_GetNextModuleID(module_t *module)
{
    for (int i = 0; i < module_msg_available; i++)
    {
        if (module_msg_mngr[i] == module)
        {
            //there is the module we are looking for
            return i;
        }
    }
    return -1;
}
/******************************************************************************
 * @brief  find the next message from a specific id for a specific module
 * @param ID module msg come from
 * @param ID module to look at
 * @return ID module
 ******************************************************************************/
int8_t Mngr_GetNextMsgID(int mngr_id, short msg_from)
{
    // find the next message from the specified id
    for (int i = 0; i < module_msg_mngr[mngr_id]->message_available; i++)
    {
        if (module_msg_mngr[mngr_id]->msg_stack[i]->header.source == msg_from)
        {
            return i;
        }
    }
    return -1;
}
