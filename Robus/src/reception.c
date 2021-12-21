/******************************************************************************
 * @file reception
 * @brief reception state machine
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "reception.h"

#include <string.h>
#include <stdbool.h>
#include "luos_hal.h"
#include "target.h"
#include "transmission.h"
#include "msg_alloc.h"
#include "luos_utils.h"
#include "timestamp.h"
#include "robus.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef DEBUG
#include <stdio.h>
#endif

#ifdef SELFTEST
#include "selftest.h"
#endif

#define COLLISION_DETECTION_NUMBER 4
#define BYTE_TRANSMIT_TIME         8
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint16_t data_count             = 0;
uint16_t data_size              = 0;
uint16_t crc_val                = 0;
static uint64_t ll_rx_timestamp = 0;
uint8_t large_data_num          = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static inline uint8_t Recep_IsAckNeeded(void);
static inline uint16_t Recep_CtxIndexFromID(uint16_t id);
void Recep_ComputeMsgNumber(void);
/******************************************************************************
 * @brief Reception init.
 * @param None
 * @return None
 ******************************************************************************/
void Recep_Init(void)
{
    // Initialize the reception state machine
    ctx.rx.status.unmap = 0;
    ctx.rx.callback     = Recep_GetHeader;
}
/******************************************************************************
 * @brief Callback to get a complete header
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_GetHeader(volatile uint8_t *data)
{
    // Catch a byte.
    MsgAlloc_SetData(*data);
    data_count++;

    // Check if we have all we need.
    switch (data_count)
    {
        case 1: // reset CRC computation
            // when we catch the first byte we timestamp the msg
            //  -8 : time to transmit 8 bits at 1 us/bit
            ll_rx_timestamp = LuosHAL_GetTimestamp() - BYTE_TRANSMIT_TIME;

            ctx.tx.lock = true;
            // Switch the transmit status to disable to be sure to not interpreat the end timeout as an end of transmission.
            ctx.tx.status = TX_DISABLE;
            crc_val       = 0xFFFF;
            break;

        case 3: // check if message is for the node
            if (Recep_NodeConcerned((header_t *)&current_msg->header) == false)
            {
                MsgAlloc_ValidHeader(false, data_size);
                ctx.rx.callback = Recep_Drop;
                return;
            }
            break;

        case (sizeof(header_t)): // Process at the header
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

            // Switch state machine to data reception
            ctx.rx.callback = Recep_GetData;
            // Cap size for big messages
            if (current_msg->header.size > MAX_DATA_MSG_SIZE)
            {
                data_size = MAX_DATA_MSG_SIZE;
                // store the number of the messages that compose the large message
                Recep_ComputeMsgNumber();
            }
            else
            {
                data_size = current_msg->header.size;
            }

            if ((ctx.rx.status.rx_framing_error == false))
            {
                if (data_size)
                {
                    MsgAlloc_ValidHeader(true, data_size);
                }
            }
            else
            {
                MsgAlloc_ValidHeader(false, data_size);
                ctx.rx.callback = Recep_Drop;
                return;
            }
            break;

        default:
            break;
    }
    LuosHAL_ComputeCRC((uint8_t *)data, (uint8_t *)&crc_val);
}
/******************************************************************************
 * @brief Callback to get a complete data
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_GetData(volatile uint8_t *data)
{
    static uint16_t last_crc = 0;

    MsgAlloc_SetData(*data);
    if (data_count < data_size)
    {
        // Continue CRC computation until the end of data
        LuosHAL_ComputeCRC((uint8_t *)data, (uint8_t *)&crc_val);
    }
    else if (data_count > data_size)
    {
        uint16_t crc = ((uint16_t)current_msg->data[data_size]) | ((uint16_t)current_msg->data[data_size + 1] << 8);
        if (crc == crc_val)
        {
            // if message is timestamped, update the timestamp
            if (Timestamp_IsTimestampMsg((msg_t *)current_msg))
            {
                uint64_t latency = 0;
                // get timestamp in message stream
                // timestamp is placed at the end of the payload, just before the crc.
                memcpy(&latency, (msg_t *)&current_msg->data[current_msg->header.size - sizeof(uint64_t)], sizeof(uint64_t));
                ll_rx_timestamp = (ll_rx_timestamp >= latency) ? ll_rx_timestamp - latency : 0;
                memcpy((msg_t *)&current_msg->data[current_msg->header.size - sizeof(uint64_t)], &ll_rx_timestamp, sizeof(uint64_t));
            }

            if (Recep_IsAckNeeded())
            {
                Transmit_SendAck();
            }

            // Make an exception for reset detection command
            if (current_msg->header.cmd == START_DETECTION)
            {
                MsgAlloc_Reset();
                ctx.tx.status = TX_DISABLE;
                Robus_SetNodeDetected(NETWORK_LINK_CONNECTING);
                Robus_SetVerboseMode(false);
            }
            else
            {
                // check if we have already received the same message
                if ((crc == last_crc) && (large_data_num > 0))
                {
                    // if yes remove this message from memory
                    MsgAlloc_InvalidMsg();
                    ctx.rx.callback = Recep_Drop;
                    return;
                }
                // if not treat message normally
                MsgAlloc_EndMsg();
                // reduce the number of remaining messages in case of a large message
                if (large_data_num > 0)
                {
                    large_data_num--;
                    // store the current crc for the next message comparison
                    last_crc = crc;
                }
                else
                {
                    last_crc = 0;
                }
            }
        }
        else
        {
            ctx.rx.status.rx_error = true;
            if (Recep_IsAckNeeded())
            {
                Transmit_SendAck();
            }
            MsgAlloc_InvalidMsg();
        }
        ctx.rx.callback = Recep_Drop;
        return;
    }
    data_count++;
}
/******************************************************************************
 * @brief Callback to get a collision beetween RX and Tx
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_GetCollision(volatile uint8_t *data)
{
    // Check data integrity
    if ((ctx.tx.data[data_count++] != *data) || (!ctx.tx.lock) || (ctx.rx.status.rx_framing_error == true))
    {
        // Data dont match, or we don't start to send the message, there is a collision
        ctx.tx.collision = true;
        // Stop TX trying to save input datas
        LuosHAL_SetTxState(false);
        // Save the received data into the allocator to be able to continue the reception
        for (uint8_t i = 0; i < data_count - 1; i++)
        {
            MsgAlloc_SetData(*ctx.tx.data + i);
        }
        MsgAlloc_SetData(*data);
        // Switch to get header.
        ctx.rx.callback = Recep_GetHeader;
        ctx.tx.status   = TX_NOK;
        if (data_count >= 3)
        {
            if (Recep_NodeConcerned((header_t *)&current_msg->header) == false)
            {
                MsgAlloc_ValidHeader(false, data_size);
                ctx.rx.callback = Recep_Drop;
                return;
            }
        }
    }
    else
    {
        if (data_count == COLLISION_DETECTION_NUMBER)
        {
#ifdef SELFTEST
            selftest_SetRxFlag();
#endif
            // collision detection end
            LuosHAL_SetRxState(false);
            LuosHAL_ResetTimeout(0);
            if (ctx.tx.status == TX_NOK)
            {
                // switch to catch Ack.
                ctx.rx.callback = Recep_CatchAck;
            }
            else
            {
                // switch to get header.
                ctx.rx.callback = Recep_GetHeader;
            }
            return;
        }
    }
    LuosHAL_ComputeCRC((uint8_t *)data, (uint8_t *)&crc_val);
}
/******************************************************************************
 * @brief Callback to get a complete header
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_Drop(volatile uint8_t *data)
{
    return;
}
/******************************************************************************
 * @brief end of a reception
 * @param None
 * @return None
 ******************************************************************************/
void Recep_Timeout(void)
{
    if ((ctx.rx.callback != Recep_GetHeader) && (ctx.rx.callback != Recep_Drop))
    {
        ctx.rx.status.rx_timeout = true;
    }
    MsgAlloc_InvalidMsg();
    Recep_Reset();
    Transmit_End(); // This is possibly the end of a transmission, check it.
}
/******************************************************************************
 * @brief reset the reception state machine
 * @param None
 * @return None
 ******************************************************************************/
void Recep_Reset(void)
{
    data_count                     = 0;
    crc_val                        = 0xFFFF;
    ctx.tx.lock                    = false;
    ctx.rx.status.rx_framing_error = false;
    ctx.rx.callback                = Recep_GetHeader;
    LuosHAL_SetRxDetecPin(true);
}
/******************************************************************************
 * @brief Catch ack when needed for the sent msg
 * @param data come from RX
 * @return None
 ******************************************************************************/
void Recep_CatchAck(volatile uint8_t *data)
{
    volatile status_t status;
    status.unmap = *data;
    if ((!status.rx_error) && (status.identifier == 0x0F))
    {
        ctx.tx.status = TX_OK;
    }
    else
    {
        ctx.tx.status = TX_NOK;
    }
}
/******************************************************************************
 * @brief Parse msg to find a service concerned
 * @param header of message
 * @return ll_service pointer
 ******************************************************************************/
ll_service_t *Recep_GetConcernedLLService(header_t *header)
{
    uint16_t i = 0;
    // Find if we are concerned by this message.
    switch (header->target_mode)
    {
        case IDACK:
        case ID:
            // Check all ll_service id
            for (i = 0; i < ctx.ll_service_number; i++)
            {
                if ((header->target == ctx.ll_service_table[i].id))
                {
                    return (ll_service_t *)&ctx.ll_service_table[i];
                }
            }
            break;
        case TYPE:
            // Check all ll_service type
            for (i = 0; i < ctx.ll_service_number; i++)
            {
                if (header->target == ctx.ll_service_table[i].type)
                {
                    return (ll_service_t *)&ctx.ll_service_table[i];
                }
            }
            break;
        case BROADCAST:
        case NODEIDACK:
        case NODEID:
            return (ll_service_t *)&ctx.ll_service_table[0];
            break;
        case MULTICAST: // For now Multicast is disabled
        default:
            return NULL;
            break;
    }
    return NULL;
}
/******************************************************************************
 * @brief Parse msg to find a service concerne
 * @param header of message
 * @return None
 ******************************************************************************/
static inline error_return_t Recep_NodeCompare(uint16_t ID)
{
    //--------------------------->|__________|
    //	Shift byte		            byte Mask of bit address

    uint16_t compare = 0;

    if ((ID > (8 * ctx.ShiftMask))) // IDMask aligned byte
    {
        compare = ((ID - 1) - ((8 * ctx.ShiftMask)));
        if ((ctx.IDMask[compare / 8] & (1 << (compare % 8))) != 0)
        {
            return SUCCEED;
        }
    }
    return FAILED;
}
/******************************************************************************
 * @brief Parse msg to find a service concerne
 * @param header of message
 * @return None
 ******************************************************************************/
luos_localhost_t Recep_NodeConcerned(header_t *header)
{
    uint16_t i = 0;

    // Find if we are concerned by this message.
    // check if we need to filter all the messages

    switch (header->target_mode)
    {
        case IDACK:
            ctx.rx.status.rx_error = false;
        case ID:
            // Check all ll_service id
            if (Recep_NodeCompare(header->target) == SUCCEED)
            {
                return ctx.verbose;
            }
            if (ctx.filter_state == false)
            {
                // check if it is message comes from service that demanded the filter desactivation
                if (ctx.filter_id != header->source)
                {
                    // if there is a service that deactivated the filtering occupy the message
                    return MULTIHOST;
                }
            }
            break;
        case TYPE:
            // Check all ll_service type
            for (i = 0; i < ctx.ll_service_number; i++)
            {
                if (header->target == ctx.ll_service_table[i].type)
                {
                    return MULTIHOST;
                }
            }
            if (ctx.filter_state == false)
            {
                // check if it is message comes from service that demanded the filter desactivation
                if (ctx.filter_id != header->source)
                {
                    // if there is a service that deactivated the filtering occupy the message
                    return MULTIHOST;
                }
            }
            break;
        case BROADCAST:
            if (header->target == BROADCAST_VAL)
            {
                return MULTIHOST;
            }
            break;
        case NODEIDACK:
            ctx.rx.status.rx_error = false;
        case NODEID:
            if ((header->target == 0) && (ctx.port.activ != NBR_PORT) && (ctx.port.keepLine == false))
            {
                return ctx.verbose; // discard message if ID = 0 and no Port activ
            }
            else
            {
                if ((header->target == ctx.node.node_id) && ((header->target != 0)))
                {
                    return ctx.verbose;
                }
                else if (ctx.filter_state == false)
                {
                    // check if it is message comes from service that demanded the filter desactivation
                    if (ctx.filter_id != header->source)
                    {
                        // if there is a service that deactivated the filtering occupy the message
                        return MULTIHOST;
                    }
                }
            }
            break;
        case MULTICAST: // For now Multicast is disabled
        default:
            return EXTERNALHOST;
            break;
    }
    return EXTERNALHOST;
}
/******************************************************************************
 * @brief Double Allocate msg_task in case of desactivated filter
 * @param msg pointer
 * @return None
 ******************************************************************************/
static inline void Recep_DoubleAlloc(msg_t *msg)
{
    // if there is a service that deactivated the filter we also allocate a message for it
    if (ctx.filter_state == false)
    {
        // check if it is message for the same service that demanded the filter desactivation
        if (ctx.filter_id != msg->header.target)
        {
            // find the position of this service in the node
            // store the message if it is not so that we dont have double messages in memory
            uint16_t idx = Recep_CtxIndexFromID(ctx.filter_id);
            MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[idx], msg);
        }
    }
}
/******************************************************************************
 * @brief Parse msg to find all services concerned and create
 * @param msg pointer
 * @return None
 ******************************************************************************/
void Recep_InterpretMsgProtocol(msg_t *msg)
{
    uint16_t i = 0;

    // Find if we are concerned by this message.
    switch (msg->header.target_mode)
    {
        case IDACK:
        case ID:
            // Check all ll_service id
            for (i = 0; i < ctx.ll_service_number; i++)
            {
                if (msg->header.target == ctx.ll_service_table[i].id)
                {
                    MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[i], msg);
                    break;
                }
            }
            // check if we need to double allocate msg_task
            Recep_DoubleAlloc(msg);
            return;
            break;
        case TYPE:
            // Check all ll_service type
            for (i = 0; i < ctx.ll_service_number; i++)
            {
                if (msg->header.target == ctx.ll_service_table[i].type)
                {
                    MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[i], msg);
                }
            }
            // check if we need to double allocate msg_task
            Recep_DoubleAlloc(msg);
            return;
            break;
        case BROADCAST:
            for (i = 0; i < ctx.ll_service_number; i++)
            {
                MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[i], msg);
            }
            return;
            break;
        case MULTICAST:
            for (i = 0; i < ctx.ll_service_number; i++)
            {
                if (Trgt_MulticastTargetBank((ll_service_t *)&ctx.ll_service_table[i], msg->header.target))
                {
                    // TODO manage multiple slave concerned
                    MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[i], msg);
                }
            }
            // check if we need to double allocate msg_task
            Recep_DoubleAlloc(msg);
            return;
            break;
        case NODEIDACK:
        case NODEID:
            if (msg->header.target == DEFAULTID) // on default ID it's always a luos command create only one task
            {
                MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[0], msg);
                return;
            }
            // check if the message is really for the node or it is a service that has no filter
            if (msg->header.target == ctx.node.node_id)
            {
                for (i = 0; i < ctx.ll_service_number; i++)
                {
                    MsgAlloc_LuosTaskAlloc((ll_service_t *)&ctx.ll_service_table[i], msg);
                }
            }
            // check if we need to double allocate msg_task
            Recep_DoubleAlloc(msg);
            return;
            break;
        default:
            break;
    }
}
/******************************************************************************
 * @brief Check if we need to send an ack
 * @param None
 * @return true or false
 ******************************************************************************/
static inline uint8_t Recep_IsAckNeeded(void)
{
    // check the mode of the message received
    if ((current_msg->header.target_mode == IDACK) && (Recep_NodeCompare(current_msg->header.target) == SUCCEED))
    {
        // when it is a idack and this message is destined to the node send an ack
        return 1;
    }
    else if ((current_msg->header.target_mode == NODEIDACK) && (ctx.node.node_id == current_msg->header.target))
    {
        // when it is nodeidack and this message is destined to the node send an ack
        return 1;
    }
    // if not failed
    return 0;
}

/******************************************************************************
 * @brief returns the index in context table from the service id
 * @param id
 * @return index
 ******************************************************************************/
static inline uint16_t Recep_CtxIndexFromID(uint16_t id)
{
    return (id - ctx.ll_service_table[0].id);
}

/******************************************************************************
 * @brief computes the number of the messages that compose a large data message
 * @param None
 * @return None
 ******************************************************************************/
void Recep_ComputeMsgNumber(void)
{
    // check if it is the first msg of large data received
    if (large_data_num == 0)
    {
        // find the number of the messages that belong to the same large message
        large_data_num = current_msg->header.size / MAX_DATA_MSG_SIZE;
        if (current_msg->header.size % MAX_DATA_MSG_SIZE != 0)
        {
            large_data_num++;
        }
    }
}
