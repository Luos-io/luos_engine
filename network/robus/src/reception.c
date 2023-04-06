/******************************************************************************
 * @file reception
 * @brief reception state machine
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

/******************************* Description of the RX process ***********************************
 *                                          Byte Received
 *                                                +
 *                                                |
 *                                        no      v      yes
 *                         +-----------------+Tx Enable+---------------+
 *                         |                                           |
 *  +------+     true      v                                           v
 *  | Drop | <--------+Drop state<-----------------------+      +------+------+
 *  | byte |               +                             |      |  Get        |
 *  +------+               | false                       |      |   Collision |
 *                         v                             |      |             |
 *                   Header complete                     |      |  source ID  |yes
 *                         +                             |      |   received  +------>Disable Rx
 *                         | yes                         |      |      +      |           +
 *                         v                             |      |      |no    |           |
 *                  +------+------+   +-------------+    |    no|      v      |           v
 *                  |  Get        |   |  Get        |    +------+  Collision  |       Wait End
 *                  |    Header   |   |     Data    |           +-------------+        Transmit
 *                  |             |   |             |                  |yes               +
 *                  | +---------+ |   |             |                  v                  |
 *                no| |   Node  | |   |  Message    |             Disable Tx       no     v
 *Drop = true <-------+Concerned| |   |    Complete |                           +----+Ack needed
 *     ^            | +---------+ |   |      +      |                           |         +
 *     |            |      |yes   |   |      |yes   |    Drop message           |         |yes
 *     |            |      v      |   |      v      |no   Send NACK             |         v
 *     |          no|    valid    |   |  Valid    +---->   if needed +--+       |     +---+----+
 *     +---------------+ Header   |   |       CRC   |                   |       |     | Get    |
 *                  |      +      |   +------+------+                   |       |     |    Ack |
 *                  |      |yes   |          |yes                       v       v     +---+----+
 *                  |      v      |          v                  +-------+-------+-+       |
 *                  |   Header    |    Store message            |     Timeout     |       |
 *                  |   Complete  |   Send ACK if needed+------>+  End reception  +<------+
 *                  +-------------+                             +-----------------+
 ***********************************************************************************************/

#include <string.h>
#include <stdbool.h>
#include "reception.h"
#include "robus_hal.h"
#include "luos_hal.h"
#include "pub_sub.h"
#include "msg_alloc.h"
#include "luos_utils.h"
#include "_timestamp.h"
#include "robus.h"
#include "bootloader_core.h"
#include "filter.h"
#include "struct_engine.h"
#include "context.h"
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
uint16_t data_count = 0;
uint16_t data_size  = 0;
uint16_t crc_val    = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static inline uint8_t Recep_IsAckNeeded(void);
/******************************************************************************
 * @brief Reception init.
 * @param None
 * @return None
 ******************************************************************************/
void Recep_Init(luos_phy_t *phy_robus)
{
    // Initialize the reception state machine
    ctx.rx.status.unmap      = 0;
    ctx.rx.callback          = Recep_GetHeader;
    ctx.rx.status.identifier = 0xF;
    phy_robus->rx_timestamp  = 0;
}
/******************************************************************************
 * @brief Callback to get a complete header
 * @param data come from RX
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Recep_GetHeader(luos_phy_t *phy_robus, volatile uint8_t *data)
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
            phy_robus->rx_timestamp = LuosHAL_GetTimestamp() - BYTE_TRANSMIT_TIME;

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
            printf("protocol : 0x%04x\n", current_msg->header.config);         /*!< Protocol version. */
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
            }
            else
            {
                data_size = current_msg->header.size;
                // we need to check if we have a timestamped message and increase the data size if yes
                if (Luos_IsMsgTimstamped((msg_t *)current_msg) == true)
                {
                    data_size += sizeof(time_luos_t);
                }
            }

            if (ctx.rx.status.rx_framing_error == false)
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
    RobusHAL_ComputeCRC((uint8_t *)data, (uint8_t *)&crc_val);
}
/******************************************************************************
 * @brief Callback to get a complete data
 * @param data come from RX
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Recep_GetData(luos_phy_t *phy_robus, volatile uint8_t *data)
{
    MsgAlloc_SetData(*data);
    if (data_count < data_size)
    {
        // Continue CRC computation until the end of data
        RobusHAL_ComputeCRC((uint8_t *)data, (uint8_t *)&crc_val);
    }
    else if (data_count > data_size)
    {
        uint16_t crc = ((uint16_t)current_msg->data[data_size]) | ((uint16_t)current_msg->data[data_size + 1] << 8);
        if (crc == crc_val)
        {
            if (Recep_IsAckNeeded())
            {
                Transmit_SendAck();
            }
            MsgAlloc_ValidDataIntegrity();
            // If message is timestamped, convert the latency to date
            if (Luos_IsMsgTimstamped((msg_t *)current_msg))
            {
                // This conversion also remove the timestamp from the message size.
                Timestamp_ConvertToDate((msg_t *)current_msg, phy_robus->rx_timestamp);
            }

            // Make an exception for bootloader command
            if ((current_msg->header.cmd == BOOTLOADER_CMD) && (current_msg->data[0] == BOOTLOADER_RESET))
            {
                LuosHAL_SetMode((uint8_t)BOOT_MODE);
                LuosHAL_Reboot();
            }

            // Make an exception for reset detection command
            if (current_msg->header.cmd == START_DETECTION)
            {
                MsgAlloc_Reset();
                ctx.tx.status = TX_DISABLE;
                Node_SetState(EXTERNAL_DETECTION);
                PortMng_Init();
            }
            else
            {
                MsgAlloc_EndMsg();
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
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Recep_GetCollision(luos_phy_t *phy_robus, volatile uint8_t *data)
{
    // Check data integrity
    if ((ctx.tx.data[data_count++] != *data) || (!ctx.tx.lock) || (ctx.rx.status.rx_framing_error == true))
    {
        // Data dont match, or we don't start to send the message, there is a collision
        ctx.tx.collision = true;
        // Stop TX trying to save input datas
        RobusHAL_SetTxState(false);
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
            // Collision detection end
            RobusHAL_SetRxState(false);
            RobusHAL_ResetTimeout(0);
            if (ctx.tx.status == TX_NOK)
            {
                // Switch to catch Ack.
                ctx.rx.callback = Recep_CatchAck;
            }
            else
            {
                // Switch to get header.
                ctx.rx.callback = Recep_GetHeader;
            }
            return;
        }
    }
    RobusHAL_ComputeCRC((uint8_t *)data, (uint8_t *)&crc_val);
}
/******************************************************************************
 * @brief Callback to drop received data wrong header, data, or collision
 * @param data come from RX
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Recep_Drop(luos_phy_t *phy_robus, volatile uint8_t *data)
{
    return;
}
/******************************************************************************
 * @brief end of a reception and prepare next receive or transmit
 * @param None
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Recep_Timeout(void)
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
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Recep_Reset(void)
{
    data_count                     = 0;
    crc_val                        = 0xFFFF;
    ctx.rx.status.rx_framing_error = false;
    ctx.rx.callback                = Recep_GetHeader;
    RobusHAL_SetRxDetecPin(true);
}
/******************************************************************************
 * @brief Catch ack when needed for the sent msg
 * @param data come from RX
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void Recep_CatchAck(luos_phy_t *phy_robus, volatile uint8_t *data)
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
 * @brief Parse msg to find a service concerne
 * @param header of message
 * @return None
 * warning : this function can be redefined only for mock testing purpose
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL luos_localhost_t Recep_NodeConcerned(header_t *header)
{
    // Find if we are concerned by this message.
    // check if we need to filter all the messages

    switch (header->target_mode)
    {
        case SERVICEIDACK:
            ctx.rx.status.rx_error = false;
        case SERVICEID:
            // Check all service id
            if (Filter_ServiceID(header->target))
            {
                return LOCALHOST;
            }
            break;
        case TYPE:

            if (Filter_Type(header->target))
            {
                return MULTIHOST;
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
                return LOCALHOST; // discard message if ID = 0 and no Port activ
            }
            else
            {
                if ((header->target == Node_Get()->node_id) && ((header->target != 0)))
                {
                    return LOCALHOST;
                }
            }
            break;
        case TOPIC:
            if (Filter_Topic(header->target))
            {
                return MULTIHOST;
            }
            break;
        default:
            return EXTERNALHOST;
            break;
    }
    return EXTERNALHOST;
}

/******************************************************************************
 * @brief Check if we need to send an ack
 * @param None
 * @return true or false
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL static inline uint8_t Recep_IsAckNeeded(void)
{
    // check the mode of the message received
    if ((current_msg->header.target_mode == SERVICEIDACK) && (Filter_ServiceID(current_msg->header.target)))
    {
        // when it is a serviceidack and this message is destined to the node send an ack
        return 1;
    }
    else if ((current_msg->header.target_mode == NODEIDACK) && (Node_Get()->node_id == current_msg->header.target))
    {
        // when it is nodeidack and this message is destined to the node send an ack
        return 1;
    }
    // if not failed
    return 0;
}
