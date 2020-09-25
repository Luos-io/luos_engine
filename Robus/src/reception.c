/******************************************************************************
 * @file reception
 * @brief reception state machine
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "reception.h"

#include <string.h>
#include <stdbool.h>
#include "luosHAL.h"
#include "target.h"
#include "sys_msg.h"
#include "msgAlloc.h"
#include "main.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef DEBUG
#include <stdio.h>
#endif

#define CURRENTMSG ctx.msg[ctx.current_buffer]
#define CURRENTMODULE ctx.vm_table[ctx.alloc_msg[ctx.current_buffer]]
/*******************************************************************************
 * Variables
 ******************************************************************************/
unsigned char keep = FALSE;
unsigned short data_count = 0;
unsigned short data_size = 0;
unsigned short crc_val = 0;
msg_t *current_msg;
vm_t *module_concerned_by_current_msg[MAX_VM_NUMBER]; /*!< Save the module concerned by the current message. */
uint16_t module_concerned_stack_pointer = 0;          /*!< The next empty case of module_concerned_by_current_msg */
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Reception init.
 * @param None
 * @return None
 ******************************************************************************/
void Recep_Init(void)
{
    // Initialize the reception state machine
    ctx.data_cb = Recep_GetHeader;
    // Get allocation values
    current_msg = MsgAlloc_GetCurrentMsg();
}
/******************************************************************************
 * @brief Callback to get a complete header
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_GetHeader(volatile unsigned char *data)
{
    ctx.tx_lock = true;
    // Catch a byte.
    MsgAlloc_SetData(*data);
    data_count++;

    // Check if we have all we need.
    if (data_count == (sizeof(header_t)))
    {
#ifdef DEBUG
        printf("*******header data*******\n");
        printf("protocol : 0x%04x\n", current_msg->header.protocol);       /*!< Protocol version. */
        printf("target : 0x%04x\n", current_msg->header.target);           /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
        printf("target_mode : 0x%04x\n", current_msg->header.target_mode); /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
        printf("source : 0x%04x\n", current_msg->header.source);           /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
        printf("cmd : 0x%04x\n", current_msg->header.cmd);                 /*!< msg definition. */
        printf("size : 0x%04x\n", current_msg->header.size);               /*!< Size of the data field. */
#endif
        // Reset the catcher.
        data_count = 0;
        // Switch state machiine to data reception
        ctx.data_cb = Recep_GetData;
        // Cap size for big messages
        if (current_msg->header.size > MAX_DATA_MSG_SIZE)
            data_size = MAX_DATA_MSG_SIZE;
        else
            data_size = current_msg->header.size;
        keep = Recep_ModuleConcerned((header_t *)&current_msg->header);
        if (keep)
        {
            // start crc computation
            LuosHAL_ComputeCRC((unsigned char *)current_msg->stream, sizeof(header_t), (unsigned char *)&crc_val);
        }
        else
        {
            MsgAlloc_InvalidMsg();
        }
    }
}
/******************************************************************************
 * @brief Callback to get a complete data
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_GetData(volatile unsigned char *data)
{
    if (keep)
    {
        MsgAlloc_SetData(*data);
        if (data_count < data_size)
        {
            // Continue CRC computation until the end of data
            LuosHAL_ComputeCRC((unsigned char *)data, 1, (unsigned char *)&crc_val);
        }
    }
    if (data_count > data_size)
    {
        if (keep)
        {
            uint16_t crc = ((unsigned short)current_msg->data[data_size]) |
                           ((unsigned short)current_msg->data[data_size + 1] << 8);
            if (crc == crc_val)
            {
                if ((current_msg->header.target_mode == IDACK) && (current_msg->header.target != DEFAULTID))
                {
                    Transmit_SendAck();
                }
                MsgAlloc_EndMsg(module_concerned_by_current_msg, &module_concerned_stack_pointer);
            }
            else
            {
                ctx.status.rx_error = TRUE;
                if ((current_msg->header.target_mode == IDACK))
                {
                    Transmit_SendAck();
                }
                module_concerned_stack_pointer = 0;
                MsgAlloc_InvalidMsg();
            }
            ctx.data_cb = Recep_GetHeader;
        }
        Recep_Reset();
        return;
    }
    data_count++;
}
/******************************************************************************
 * @brief Finish a message and alloc all concerned modules
 * @param None
 * @return None
 ******************************************************************************/
void Recep_EndMsg(void)
{
    MsgAlloc_EndMsg(module_concerned_by_current_msg, &module_concerned_stack_pointer);
}
/******************************************************************************
 * @brief Callback to get a collision beetween RX and Tx
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_GetCollision(volatile unsigned char *data)
{
    if ((*ctx.tx_data != *data) || (!ctx.tx_lock))
    {
        //data dont match, or we don't start to send, there is a collision
        ctx.collision = TRUE;
        //Stop TX trying to save input datas
        LuosHAL_SetTxState(false);
        // send all received datas
        Recep_GetHeader(data);
    }
    ctx.tx_data = ctx.tx_data + 1;
}
/******************************************************************************
 * @brief end of a reception
 * @param None
 * @return None
 ******************************************************************************/
void Recep_Timeout(void)
{
    if (ctx.data_cb != Recep_GetHeader)
    {
        ctx.status.rx_timeout = TRUE;
    }
    module_concerned_stack_pointer = 0;
    MsgAlloc_InvalidMsg();
    ctx.tx_lock = false;
    Recep_Reset();
}
/******************************************************************************
 * @brief reset the reception state machine
 * @param None
 * @return None
 ******************************************************************************/
void Recep_Reset(void)
{
    LuosHAL_SetIrqState(false);
    ctx.data_cb = Recep_GetHeader;
    keep = FALSE;
    data_count = 0;
    LuosHAL_SetIrqState(true);
}
/******************************************************************************
 * @brief Catch ack when needed for the sended msg
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_CatchAck(volatile unsigned char *data)
{
    ctx.ack = *data;
    ctx.data_cb = Recep_GetHeader;
}
/******************************************************************************
 * @brief Parse msg to find a module concerne
 * @param header of message
 * @return None
 ******************************************************************************/
uint8_t Recep_NodeConcerned(header_t *header)
{
    // Find if we are concerned by this message.
    switch (header->target_mode)
    {
    case IDACK:
    case ID:
        // Check all VM id
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if ((header->target == ctx.vm_table[i].id))
            {
                return true;
            }
        }
        break;
    case TYPE:
        // Check all VM type
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (header->target == ctx.vm_table[i].type)
            {
                return true;
            }
        }
        break;
    case BROADCAST:
        return true;
        break;
    case MULTICAST:
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (Trgt_MulticastTargetBank((vm_t *)&ctx.vm_table[i], header->target))
            { //TODO manage multiple slave concerned
                return true;
            }
        }
        break;
    default:
        return false;
        break;
    }
    return false;
}
/******************************************************************************
 * @brief Parse msg to find all modules concerned
 * @param header of message
 * @return None
 ******************************************************************************/
uint8_t Recep_ModuleConcerned(header_t *header)
{
    uint8_t concerned = FALSE;
    // Find if we are concerned by this message.
    switch (header->target_mode)
    {
    case IDACK:
        ctx.status.rx_error = FALSE;
    case ID:
        // Get ID even if this is default ID and we have an activ branch waiting to be linked to a module id
        if ((header->target == ctx.id) && (ctx.detection.activ_branch != NO_BRANCH))
        {
            module_concerned_by_current_msg[module_concerned_stack_pointer++] = (vm_t *)&ctx.vm_table[0];
            MsgAlloc_ValidHeader();
            return TRUE;
            break;
        }
        // Check all VM id
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (header->target == ctx.vm_table[i].id)
            {
                module_concerned_by_current_msg[module_concerned_stack_pointer++] = (vm_t *)&ctx.vm_table[i];
                MsgAlloc_ValidHeader();
                return TRUE;
                break;
            }
        }
        break;
    case TYPE:
        //check default type
        if (header->target == ctx.type)
        {
            module_concerned_by_current_msg[module_concerned_stack_pointer++] = (vm_t *)&ctx.vm_table[0];
            MsgAlloc_ValidHeader();
            return TRUE;
            break;
        }
        // Check all VM type
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (header->target == ctx.vm_table[i].type)
            {
                module_concerned_by_current_msg[module_concerned_stack_pointer++] = (vm_t *)&ctx.vm_table[i];
                MsgAlloc_ValidHeader();
                return TRUE;
            }
        }
        break;
    case BROADCAST:
        while (module_concerned_stack_pointer < ctx.vm_number)
        {
            module_concerned_by_current_msg[module_concerned_stack_pointer] = (vm_t *)&ctx.vm_table[module_concerned_stack_pointer];
            module_concerned_stack_pointer++;
        }
        MsgAlloc_ValidHeader();
        return TRUE;
        break;
    case MULTICAST:
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (Trgt_MulticastTargetBank((vm_t *)&ctx.vm_table[i], header->target))
            { //TODO manage multiple slave concerned
                module_concerned_by_current_msg[module_concerned_stack_pointer++] = (vm_t *)&ctx.vm_table[i];
                concerned = true;
            }
        }
        if (concerned == true)
        {
            MsgAlloc_ValidHeader();
            return true;
        }
        else
        {
            return false;
        }
        break;
    default:
        return FALSE;
        break;
    }
    return FALSE;
}
