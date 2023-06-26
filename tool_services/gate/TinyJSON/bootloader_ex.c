/******************************************************************************
 * @file Bootloader extensions
 * @brief Bootloader functionnalities for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "bootloader_ex.h"
#include "luos_bootloader.h"
#include "gate_config.h"
#include "pipe_link.h"
#include "routing_table.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
/******************************************************************************
 * @brief Process node responses and send them to the Host
 * @param service pointer, luos message
 * @return None
 ******************************************************************************/
uint16_t Bootloader_StartData(char *data)
{
    sprintf(data, "{\"bootloader\":[");
    return (sizeof("{\"bootloader\":[") - 1);
}

/******************************************************************************
 * @brief Process node responses and send them to the Host
 * @param service pointer, luos message
 * @return None
 ******************************************************************************/
uint16_t Bootloader_LuosToJson(msg_t *msg, char *data)
{
    uint16_t response_cmd = msg->header.cmd;
    uint16_t node_id      = RoutingTB_NodeIDFromID(msg->header.source);
    switch (response_cmd)
    {
        case BOOTLOADER_READY:
            sprintf(data, "{\"response\":\"ready\",\"node\":%d},", node_id);
            break;

        case BOOTLOADER_ERASE:
            sprintf(data, "{\"response\":\"erase\",\"node\":%d},", node_id);
            break;

        case BOOTLOADER_BIN_CHUNK:
            sprintf(data, "{\"response\":\"bin_chunk\",\"node\":%d},", node_id);
            break;

        case BOOTLOADER_BIN_END:
            sprintf(data, "{\"response\":\"bin_end\",\"node\":%d},", node_id);
            break;

        case BOOTLOADER_CRC:
            sprintf(data, "{\"response\":\"crc\",\"crc_value\":%d,\"node\":%d},", msg->data[0], node_id);
            break;

        case BOOTLOADER_ERROR_SIZE:
            sprintf(data, "{\"response\":\"error_size\",\"node\":%d},", node_id);
            break;

        default:
            break;
    }
    return (uint16_t)strlen(data);
}

// This function start a Json structure and return the string size.
void Bootloader_EndData(service_t *service, char *data, char *data_ptr)
{
    // remove the last "," char
    *(--data_ptr) = '\0';
    // End the Json message
    memcpy(data_ptr, "]}\n", sizeof("]}\n"));
    data_ptr += sizeof("]}\n") - 1;
    // Send the message to pipe
    PipeLink_Send(service, data, data_ptr - data);
}

/******************************************************************************
 * @brief Process Host commands and send them to the node
 * @param service pointer, binary data and json object received
 * @return None
 ******************************************************************************/
void Bootloader_JsonToLuos(service_t *service, char *bin_data, json_t const *bootloader_json)
{
    if (json_getType(json_getProperty(bootloader_json, "command")) == JSON_OBJ)
    {
        // Get "command" json object
        json_t const *command_item = json_getProperty(bootloader_json, "command");
        // Parse all relevant values in json object
        const char *type     = json_getPropertyValue(command_item, "type");
        uint8_t topic_target = (uint8_t)json_getReal(json_getProperty(command_item, "topic"));
        uint16_t node_id     = (uint16_t)json_getReal(json_getProperty(command_item, "node"));

        // Create a message to send to nodes
        msg_t boot_msg;
        boot_msg.header.size = 0;
        if (node_id == 0)
        {
            boot_msg.header.target      = (uint16_t)topic_target; // topic
            boot_msg.header.target_mode = TOPIC;                  // msg send to the node
        }
        else
        {
            boot_msg.header.target      = node_id;   // node_id to send the message
            boot_msg.header.target_mode = NODEIDACK; // msg send to the node
        }

        uint32_t binary_size = 0;
        json_t *item         = NULL;
        if (strcmp(type, "start") == 0)
        {
            boot_msg.header.cmd = BOOTLOADER_START;
            Luos_SendMsg(service, &boot_msg);
        }
        else if (strcmp(type, "stop") == 0)
        {
            // Send stop command to bootloader app
            boot_msg.header.cmd = BOOTLOADER_STOP;
            Luos_SendMsg(service, &boot_msg);
        }
        else if (strcmp(type, "ready") == 0)
        {
            // Find binary size in json header
            binary_size = (uint32_t)json_getReal(json_getProperty(command_item, "size"));

            // Send ready command to bootloader app
            boot_msg.header.size = sizeof(char) + sizeof(uint32_t);
            boot_msg.header.cmd  = BOOTLOADER_READY;
            boot_msg.data[0]     = topic_target;
            memcpy(&(boot_msg.data[1]), &binary_size, sizeof(uint32_t));
            Luos_SendMsg(service, &boot_msg);
        }
        else if (strcmp(type, "erase") == 0)
        {
            // Send erase command to bootloader app
            boot_msg.header.cmd = BOOTLOADER_ERASE;
            Luos_SendMsg(service, &boot_msg);
        }
        else if (strcmp(type, "bin_chunk") == 0)
        {
            // Find binary size in json header
            item = (json_t *)json_getProperty(command_item, "size");
            if (json_getType(item) == JSON_ARRAY)
            {
                binary_size = (uint32_t)json_getInteger(json_getChild(item));
            }
            else
            {
                binary_size = (uint32_t)json_getReal(item);
            }

            // Send bin chunk command to bootloader app
            boot_msg.header.cmd = BOOTLOADER_BIN_CHUNK;
            int i               = 0;
            // Find the first \r of the current buf
            for (i = 0; i < GATE_BUFF_SIZE; i++)
            {
                if (bin_data[i] == '\n')
                {
                    i++;
                    break;
                }
            }
            if (i < GATE_BUFF_SIZE - 1)
            {
                boot_msg.header.size = binary_size;
                memcpy(boot_msg.data, &bin_data[i], binary_size);
                Luos_SendMsg(service, &boot_msg);
            }
        }
        else if (strcmp(type, "bin_end") == 0)
        {
            // Send bin end command to bootloader app
            boot_msg.header.cmd = BOOTLOADER_BIN_END;
            Luos_SendMsg(service, &boot_msg);
        }
        else if (strcmp(type, "crc") == 0)
        {
            // Send crc test command to bootloader app
            boot_msg.header.cmd = BOOTLOADER_CRC;
            Luos_SendMsg(service, &boot_msg);
        }
        else if (strcmp(type, "app_saved") == 0)
        {
            // send app saved command to bootloader app
            boot_msg.header.cmd = BOOTLOADER_APP_SAVED;
            Luos_SendMsg(service, &boot_msg);
        }
        else if (strcmp(type, "reset") == 0)
        {
            // send rescue command to nodes
            boot_msg.header.target      = BROADCAST_VAL;
            boot_msg.header.target_mode = BROADCAST;
            boot_msg.header.cmd         = BOOTLOADER_RESET; // bootloader cmd
            boot_msg.header.size        = 0;
            Luos_SendMsg(service, &boot_msg);
        }
    }
}
