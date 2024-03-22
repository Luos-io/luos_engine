/******************************************************************************
 * @file data_manager
 * @brief Manage data conversion strategy.
 * @author Luos
 ******************************************************************************/
#include <stdbool.h>
#include "convert.h"
#include "gate_config.h"
#include "data_manager.h"
#include "pipe_link.h"
#include "bootloader_ex.h"

uint8_t DataManager_ServiceIsSensor(luos_type_t type);

// This function will manage msg collection from sensors
void DataManager_collect(service_t *service)
{
    msg_t update_msg;
    search_result_t result;
    update_msg.header.target_mode = SERVICEIDACK;
    RTFilter_Reset(&result);
    // ask services to publish datas
    for (uint8_t i = 0; i < result.result_nbr; i++)
    {
        // Check if this service is a sensor
        if ((DataManager_ServiceIsSensor(result.result_table[i]->type)) || (result.result_table[i]->type >= LUOS_LAST_TYPE))
        {
            // This service is a sensor so create a msg to enable auto update

            update_msg.header.target = result.result_table[i]->id;
            TimeOD_TimeToMsg(&update_time, &update_msg);
            update_msg.header.cmd = UPDATE_PUB;
            Luos_SendMsg(service, &update_msg);
        }
    }
}

// This function will create a data string for services datas
void DataManager_Run(service_t *service, const msg_t *data_msg)
{
    char data[GATE_BUFF_SIZE];
    search_result_t result;
    luos_assert_t assertion;
    dead_target_t *dead_target;
    char *data_ptr  = data;

    // Init the data string
    data_ptr += Convert_StartData(data_ptr);
    switch (data_msg->header.cmd)
    {
        case ASSERT:
            memcpy(assertion.unmap, data_msg->data, data_msg->header.size);
            assertion.unmap[data_msg->header.size] = '\0';
            Convert_AssertToData(service, data_msg->header.source, assertion);
            break;

        case DEADTARGET:
            dead_target = (dead_target_t *)data_msg->data;
            if (dead_target->node_id != 0)
            {
                Convert_DeadNodeToData(service, dead_target->node_id);
            }
            if (dead_target->service_id != 0)
            {
                Convert_DeadServiceToData(service, dead_target->service_id);
            }
            break;
        case BOOTLOADER_START ... BOOTLOADER_ERROR_SIZE:
            data_ptr = data;
            data_ptr += Bootloader_StartData(data_ptr);
            data_ptr += Bootloader_LuosToJson(data_msg, data_ptr);
            Bootloader_EndData(service, data, data_ptr);
            return;
            break;
        case END_DETECTION:
            // find a pipe
            PipeLink_Find(service);
            break;

        default:
            // Check if this is a message from pipe
            if (data_msg->header.source == PipeLink_GetId())
            {
                // This message is a command from pipe
                static char data_cmd[GATE_BUFF_SIZE];
                // Convert the received data into Luos commands
                int size = Luos_ReceiveData(service, data_msg, data_cmd);
                if (size > 0)
                {
                    // We finish to receive this data, execute the received command
                    char *data_ptr = data_cmd;
                    if (data_msg->header.cmd == SET_CMD)
                    {
                        while (size > 0 && *data_ptr == '{')
                        {
                            uint32_t data_consumed = strlen(data_ptr) + 1;
                            Convert_DataToMsg(service, data_ptr);
                            size -= data_consumed;
                            data_ptr += data_consumed;
                        }
                    }
                }
                break;
            }
            // get the source of this message
            // Create service description
            char *alias;
            RTFilter_Reset(&result);
            RTFilter_ID(&result, data_msg->header.source);
            alias = result.result_table[0]->alias;
            LUOS_ASSERT(alias != 0);
            data_ptr += Convert_StartServiceData(data_ptr, alias);
            // Convert all msgs from this service into data
            data_ptr += Convert_MsgToData(data_msg, data_ptr);
            data_ptr += Convert_EndServiceData(data_ptr);
            Convert_EndData(service, data, data_ptr);
            LUOS_ASSERT((data_ptr - data) < GATE_BUFF_SIZE);
            break;
    }
}

// This function manage only commands incoming from pipe
void DataManager_RunPipeOnly(service_t *service, const msg_t *data_msg)
{
    if ((PipeLink_GetId() != 0) && (data_msg->header.source == PipeLink_GetId()))
    {
        // This message is a command from pipe
        // Convert the received data into Luos commands
        static char data_cmd[GATE_BUFF_SIZE];
        if (data_msg->header.cmd == PARAMETERS)
        {
            uintptr_t pointer;
            memcpy(&pointer, data_msg->data, sizeof(void *));
            PipeLink_SetDirectPipeSend((void *)pointer);
        }
        else if (Luos_ReceiveData(service, data_msg, data_cmd) > 0)
        {
            // We finish to receive this data, execute the received command
            Convert_DataToMsg(service, data_cmd);
        }
    }
    // Check if a node send a end detection
    if (data_msg->header.cmd == END_DETECTION)
    {
        // Find a pipe
        PipeLink_Find(service);
        if (gate_running == PREPARING)
        {
            // Generate routing table datas
            Convert_RoutingTableData(service);
            // Run the gate
            gate_running     = RUNNING;
            first_conversion = true;
        }
    }
}

uint8_t DataManager_ServiceIsSensor(luos_type_t type)
{
    if ((type == ANGLE_TYPE)
        || (type == STATE_TYPE)
        || (type == DISTANCE_TYPE)
        || (type == IMU_TYPE)
        || (type == LOAD_TYPE)
        || (type == VOLTAGE_TYPE)
        || (type == LIGHT_TYPE)
        || (type == SERVO_MOTOR_TYPE)
        || (type == PRESSURE_TYPE))
    {
        return 1;
    }
    return 0;
}
