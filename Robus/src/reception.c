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

    if (data_count == 3)
    {
        keep = Recep_NodeConcerned((header_t *)&current_msg->header);
    }
    else
    {
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
            {
                data_size = MAX_DATA_MSG_SIZE;
            }
            else
            {
                data_size = current_msg->header.size;
            }
            if (keep)
            {
                // start crc computation
                LuosHAL_ComputeCRC((unsigned char *)current_msg->stream, sizeof(header_t), (unsigned char *)&crc_val);
                if (data_size)
                {
                    MsgAlloc_ValidHeader();
                }
            }
            else
            {
                MsgAlloc_InvalidMsg();
            }
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
                MsgAlloc_EndMsg();
            }
            else
            {
                ctx.status.rx_error = TRUE;
                if ((current_msg->header.target_mode == IDACK))
                {
                    Transmit_SendAck();
                }
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
 * @brief Callback to get a collision beetween RX and Tx
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_GetCollision(volatile unsigned char *data)
{
    // send all received datas
    Recep_GetHeader(data);
    if ((*ctx.tx_data != *data) || (!ctx.tx_lock))
    {
        //data dont match, or we don't start to send, there is a collision
        ctx.collision = TRUE;
        //Stop TX trying to save input datas
        LuosHAL_SetTxState(false);
        // switch to get header.
        ctx.data_cb = Recep_GetHeader;
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
    LuosHAL_SetTxLockDetecState(true);
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
        ctx.status.rx_error = FALSE;
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
        if (header->target == BROADCAST_VAL)
        {
            return true;
        }
        break;
    case MULTICAST: // For now Multicast is disabled
    default:
        return false;
        break;
    }
    return false;
}
/******************************************************************************
 * @brief Parse msg to find all modules concerned and create
 * @param msg pointer
 * @return None
 ******************************************************************************/
void Recep_InterpretMsgProtocol(msg_t *msg)
{
    // Find if we are concerned by this message.
    switch (msg->header.target_mode)
    {
    case IDACK:
    case ID:
        // Get ID even if this is default ID and we have an activ branch waiting to be linked to a module id
        if ((msg->header.target == ctx.id) && (ctx.detection.activ_branch != NO_BRANCH))
        {
            MsgAlloc_LuosTaskAlloc((vm_t *)&ctx.vm_table[0], msg);
            return;
        }
        // Check all VM id
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (msg->header.target == ctx.vm_table[i].id)
            {
                MsgAlloc_LuosTaskAlloc((vm_t *)&ctx.vm_table[i], msg);
                return;
            }
        }
        break;
    case TYPE:
        //check default type
        if (msg->header.target == ctx.type)
        {
            MsgAlloc_LuosTaskAlloc((vm_t *)&ctx.vm_table[0], msg);
            return;
        }
        // Check all VM type
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (msg->header.target == ctx.vm_table[i].type)
            {
                MsgAlloc_LuosTaskAlloc((vm_t *)&ctx.vm_table[i], msg);
                return;
            }
        }
        break;
    case BROADCAST:
        for (int i = 0; i < ctx.vm_number; i++)
        {
            MsgAlloc_LuosTaskAlloc((vm_t *)&ctx.vm_table[i], msg);
        }
        return;
        break;
    case MULTICAST:
        for (int i = 0; i < ctx.vm_number; i++)
        {
            if (Trgt_MulticastTargetBank((vm_t *)&ctx.vm_table[i], msg->header.target))
            {
                //TODO manage multiple slave concerned
                MsgAlloc_LuosTaskAlloc((vm_t *)&ctx.vm_table[i], msg);
                return;
            }
        }
        break;
    default:
        break;
    }
}
