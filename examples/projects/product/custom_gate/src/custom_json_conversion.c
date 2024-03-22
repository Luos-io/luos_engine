#include "custom-json.h"
#include "product_config.h"
#include <stdio.h>

// This function are called by the gate conversion functions in case of an unknown type.
// This functions allow you to easily add and manage your custom type and commands.

// This function is called by the gate to convert a service type into a string.
// This is typically used in the end of detection to create a Json representing the device routing table.
// The name of the type will be used by pyluos to create the right object.
const char *Convert_CustomStringFromType(luos_type_t type)
{
    if (type == POINT_2D)
    {
        return "point_2D";
    }
    if (type == POWER_TYPE)
    {
        return "power";
    }
    return NULL;
}

// This function is called by the gate to convert a piece of Json into a message.
// This is typically used when a Json is received by the gate with an unknown property.
// You can use it to compose your own message out of the Json data and send it to the right service.
void Convert_CustomJsonToMsg(service_t *service, uint16_t target_id, char *property, const json_t *jobj, char *json_str)
{
    msg_t msg;
    msg.header.target_mode = IDACK;
    msg.header.target      = target_id;
    // Target linear position 2D
    if (property && !strcmp(property, "linear_pos_2D"))
    {
        // Check the size of the array. If we have more than one data, this mean that this data is a binary size of a trajectory. If we have 2 data, this is an unique point.
        json_t const *item = json_getChild(jobj);
        if (json_getSibling(item) != NULL)
        {
            // We only have one point in this data
            pos_2d_t pos;
            pos.x = (uint16_t)json_getInteger(item);
            item  = json_getSibling(item);
            pos.y = (uint16_t)json_getInteger(item);
            // Create the message
            msg.header.cmd  = LINEAR_POSITION_2D;
            msg.header.size = sizeof(pos_2d_t);
            memcpy(msg.data, &pos, sizeof(pos_2d_t));
            // Send the message
            Luos_SendMsg(service, &msg);
        }
        else
        {
            int i = 0;
            // This is a binary
            int size = (int)json_getInteger(item);
            // Find the first \r of the current json_str
            for (i = 0; i < GATE_BUFF_SIZE; i++)
            {
                if (json_str[i] == '\n')
                {
                    i++;
                    break;
                }
            }
            if (i < GATE_BUFF_SIZE - 1)
            {
                // Create the message
                msg.header.cmd = LINEAR_POSITION_2D;
                Luos_SendData(service, &msg, &json_str[i], (unsigned int)size);
            }
        }
        return;
    }
    if (property && !strcmp(property, "buffer_mode"))
    {
        msg.data[0]     = (char)json_getInteger(jobj);
        msg.header.cmd  = BUFFER_MODE;
        msg.header.size = sizeof(char);
        Luos_SendMsg(service, &msg);
        return;
    }
    if (property && !strcmp(property, "sampling_freq"))
    {
        float freq         = (float)json_getReal(jobj);
        time_luos_t period = TimeOD_TimeFrom_s(1.0f / freq);
        TimeOD_TimeToMsg(&period, &msg);
        Luos_SendMsg(service, &msg);
        return;
    }
}

// This function is called by the gate to convert a message into a piece of Json.
// This is typically used when a message is received by the gate with an unknown command.
// You can use it to compose your own piece of Json out of the message data.
void Convert_CustomMsgToJson(msg_t *msg, char *data)
{
    if (msg->header.cmd == LINEAR_POSITION_2D)
    {
        // This is our custom message, so we can convert it to JSON
        // In this case we will don't need it but I did the code for the sake of the example.
        if (msg->header.size == sizeof(pos_2d_t))
        {
            // Size ok, now fill the struct from msg data
            pos_2d_t pos;
            memcpy(&pos, msg->data, msg->header.size);
            // create the Json content
            sprintf(data, "\"linear_pos_2D\":[%2d,%2d],", pos.x, pos.y);
        }
    }
}
