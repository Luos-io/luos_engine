/******************************************************************************
 * @file Bootloader extensions
 * @brief Bootloader functionnalities for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "bootloader_ex.h"
#include "bootloader_core.h"
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
void Bootloader_LuosToJson(service_t *service, msg_t *msg)
{
    char boot_json[64]   = "\0";
    uint8_t response_cmd = msg->data[0];
    uint16_t node_id     = RoutingTB_NodeIDFromID(msg->header.source);
    switch (response_cmd)
    {
        case BOOTLOADER_READY_RESP:
            sprintf(boot_json, "{\"bootloader\":{\"response\":%d, \"node\":%d}}\n", BOOTLOADER_READY_RESP, node_id);
            break;

        case BOOTLOADER_ERASE_RESP:
            sprintf(boot_json, "{\"bootloader\":{\"response\":%d, \"node\":%d}}\n", BOOTLOADER_ERASE_RESP, node_id);
            break;

        case BOOTLOADER_BIN_CHUNK_RESP:
            sprintf(boot_json, "{\"bootloader\":{\"response\":%d, \"node\":%d}}\n", BOOTLOADER_BIN_CHUNK_RESP, node_id);
            break;

        case BOOTLOADER_BIN_END_RESP:
            sprintf(boot_json, "{\"bootloader\":{\"response\":%d, \"node\":%d}}\n", BOOTLOADER_BIN_END_RESP, node_id);
            break;

        case BOOTLOADER_CRC_RESP:
            sprintf(boot_json, "{\"bootloader\":{\"response\":%d,\"crc_value\":%d, \"node\":%d}}\n", BOOTLOADER_CRC_RESP, msg->data[1], node_id);
            break;

        case BOOTLOADER_ERROR_SIZE:
            sprintf(boot_json, "{\"bootloader\":{\"response\":%d, \"node\":%d}}\n", BOOTLOADER_ERROR_SIZE, node_id);
            break;

        default:
            break;
    }

    // Send the message to pipe
    PipeLink_Send(service, boot_json, strlen(boot_json));
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
        // get "command" json object
        json_t const *command_item = json_getProperty(bootloader_json, "command");
        // parse all relevant values in json object
        uint16_t type        = (uint16_t)json_getReal(json_getProperty(command_item, "type"));
        uint8_t topic_target = (uint8_t)json_getReal(json_getProperty(command_item, "topic"));
        // create a message to send to nodes
        msg_t boot_msg;
        boot_msg.header.target      = topic_target;   // first node of the network
        boot_msg.header.cmd         = BOOTLOADER_CMD; // bootloader cmd
        boot_msg.header.target_mode = TOPIC;          // msg send to the node

        uint32_t binary_size = 0;
        json_t *item         = NULL;
        switch (type)
        {
            case BOOTLOADER_START:
                boot_msg.header.target_mode = NODEIDACK;
                boot_msg.header.target      = (uint16_t)json_getReal(json_getProperty(command_item, "node"));
                boot_msg.header.size        = sizeof(char);
                boot_msg.data[0]            = BOOTLOADER_START;
                Luos_SendMsg(service, &boot_msg);
                break;

            case BOOTLOADER_STOP:
                // send stop command to bootloader app
                boot_msg.header.target_mode = NODEIDACK;
                boot_msg.header.target      = (uint16_t)json_getReal(json_getProperty(command_item, "node"));
                boot_msg.header.size        = sizeof(char);
                boot_msg.data[0]            = BOOTLOADER_STOP;
                Luos_SendMsg(service, &boot_msg);

                break;

            case BOOTLOADER_READY:
                // find binary size in json header
                binary_size = (uint32_t)json_getReal(json_getProperty(command_item, "size"));

                // send ready command to bootloader app
                boot_msg.header.target_mode = NODEIDACK;
                boot_msg.header.target      = (uint16_t)json_getReal(json_getProperty(command_item, "node"));
                boot_msg.header.size        = 2 * sizeof(char) + sizeof(uint32_t);
                boot_msg.data[0]            = BOOTLOADER_READY;
                boot_msg.data[1]            = topic_target;
                memcpy(&(boot_msg.data[2]), &binary_size, sizeof(uint32_t));
                Luos_SendMsg(service, &boot_msg);
                break;

            case BOOTLOADER_ERASE:
                // send erase command to bootloader app
                boot_msg.header.size = sizeof(char);
                boot_msg.data[0]     = BOOTLOADER_ERASE;
                Luos_SendMsg(service, &boot_msg);
                break;

            case BOOTLOADER_BIN_CHUNK:
                // find binary size in json header
                item = (json_t *)json_getProperty(command_item, "size");
                if (json_getType(item) == JSON_ARRAY)
                {
                    binary_size = (uint32_t)json_getInteger(json_getChild(item));
                }
                else
                {
                    binary_size = (uint32_t)json_getReal(item);
                }

                // send bin chunk command to bootloader app
                boot_msg.data[0] = BOOTLOADER_BIN_CHUNK;
                int i            = 0;
                // find the first \r of the current buf
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
                    boot_msg.header.size = binary_size + sizeof(char);
                    memcpy(&(boot_msg.data[1]), &bin_data[i], binary_size);
                    Luos_SendMsg(service, &boot_msg);
                }
                break;

            case BOOTLOADER_BIN_END:
                // send bin end command to bootloader app
                boot_msg.header.size = sizeof(char);
                boot_msg.data[0]     = BOOTLOADER_BIN_END;
                Luos_SendMsg(service, &boot_msg);
                break;

            case BOOTLOADER_CRC_TEST:
                // send crc test command to bootloader app
                boot_msg.header.size = sizeof(char);
                boot_msg.data[0]     = BOOTLOADER_CRC_TEST;
                Luos_SendMsg(service, &boot_msg);
                break;

            case BOOTLOADER_APP_SAVED:
                // send app saved command to bootloader app
                boot_msg.header.size = sizeof(char);
                boot_msg.data[0]     = BOOTLOADER_APP_SAVED;
                Luos_SendMsg(service, &boot_msg);
                break;

            case BOOTLOADER_RESET:
                // send rescue command to nodes
                boot_msg.header.target      = BROADCAST_VAL;
                boot_msg.header.target_mode = BROADCAST;
                boot_msg.header.cmd         = BOOTLOADER_CMD; // bootloader cmd
                boot_msg.header.size        = sizeof(char);
                boot_msg.data[0]            = BOOTLOADER_RESET;
                Luos_SendMsg(service, &boot_msg);
                break;

            default:
                break;
        }
    }
}