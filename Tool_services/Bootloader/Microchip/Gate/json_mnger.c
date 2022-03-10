#include <stdio.h>
#include <stdbool.h>
#include "json_mnger.h"
#include "cmd.h"
#include "convert.h"
#include "gate.h"
#include "bootloader_ex.h"

static unsigned int delayms = 1;

//******************* sensor update ****************************
// This function will gather data from sensors and create a json string for you
void collect_data(container_t *container)
{
    msg_t json_msg;
    json_msg.header.target_mode = ID;
    json_msg.header.cmd = ASK_PUB_CMD;
    json_msg.header.size = 0;
    // ask containers to publish datas
    for (uint8_t i = 1; i <= RoutingTB_GetLastContainer(); i++)
    {
        // Check if this container is a sensor
        if ((RoutingTB_ContainerIsSensor(RoutingTB_TypeFromID(i))) || (RoutingTB_TypeFromID(i) >= LUOS_LAST_TYPE))
        {
            // This container is a sensor so create a msg and send it
            json_msg.header.target = i;
            Luos_SendMsg(container, &json_msg);
#ifdef GATE_TIMEOUT
            // Get the current number of message available
            int back_nbr_msg = Luos_NbrAvailableMsg();
            // Get the current time
            uint32_t send_time = Luos_GetSystick();
            // Wait for a reply before continuing
            while ((back_nbr_msg == Luos_NbrAvailableMsg()) & (send_time == Luos_GetSystick()))
            {
                Luos_Loop();
            }
#endif
        }
    }
}

// This function will create a json string for containers datas
void format_data(container_t *container, char *json)
{
    msg_t *json_msg = 0;
    uint8_t json_ok = false;
    if ((Luos_NbrAvailableMsg() > 0))
    {
        // Init the json string
        sprintf(json, "{\"containers\":{");
        // loop into containers.
        uint16_t i = 1;
        // get the oldest message
        while (Luos_ReadMsg(container, &json_msg) == SUCCEED)
        {
            // check if this is an assert
            if (json_msg->header.cmd == ASSERT)
            {
                char error_json[256] = "\0";
                luos_assert_t assertion;
                memcpy(assertion.unmap, json_msg->data, json_msg->header.size);
                assertion.unmap[json_msg->header.size] = '\0';
                sprintf(error_json, "{\"assert\":{\"node_id\":%d,\"file\":\"%s\",\"line\":%d}}\n", json_msg->header.source, assertion.file, (unsigned int)assertion.line);
                json_send(error_json);
                continue;
            }
            // we receive a message from a bootloader node
            if (json_msg->header.cmd == BOOTLOADER_RESP)
            {
                LuosBootloader_GateRcv(json_msg);
                continue;
            }
            // get the source of this message
            i = json_msg->header.source;
            // Create container description
            char *alias;
            alias = RoutingTB_AliasFromId(i);
            if (alias != 0)
            {
                json_ok = true;
                sprintf(json, "%s\"%s\":{", json, alias);
                // now add json data from container
                msg_to_json(json_msg, &json[strlen(json)]);
                // Check if we receive other messages from this container
                while (Luos_ReadFromContainer(container, i, &json_msg) == SUCCEED)
                {
                    // we receive some, add it to the Json
                    msg_to_json(json_msg, &json[strlen(json)]);
                }
                if (json[strlen(json) - 1] != '{')
                {
                    // remove the last "," char
                    json[strlen(json) - 1] = '\0';
                }
                // End the container section
                sprintf(json, "%s},", json);
            }
        }
        if (json_ok)
        {
            // remove the last "," char
            json[strlen(json) - 1] = '\0';
            // End the Json message
            sprintf(json, "%s}}\n", json);
        }
        else
        {
            //create a void string
            *json = '\0';
        }
    }
    else
    {
        //create a void string
        *json = '\0';
    }
}

unsigned int get_delay(void)
{
    return delayms;
}

void set_delay(unsigned int new_delayms)
{
    delayms = new_delayms;
}
