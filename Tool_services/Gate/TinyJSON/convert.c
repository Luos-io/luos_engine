/******************************************************************************
 * @file convert
 * @brief Functions allowing to manage JSON convertion
 * @author Luos
 ******************************************************************************/
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "convert.h"
#include "luos_utils.h"
#include "gate_config.h"
#include "pipe_link.h"
#include "data_manager.h"
#include "tiny-json.h"
#include "bootloader_ex.h"

#define MAX_JSON_FIELDS 50

static const char *Convert_Float(float value);
static void Convert_JsonToMsg(service_t *service, uint16_t id, luos_type_t type, const json_t *jobj, msg_t *msg, char *bin_data);
static const char *Convert_StringFromType(luos_type_t type);

/*******************************************************************************
 * Tools
 ******************************************************************************/
// This function reduce Float to string convertion without FPU to 1/3 of normal time.
// This function have been inspired by Benoit Blanchon Blog : https://blog.benoitblanchon.fr/lightweight-float-to-string/
const char *Convert_Float(float value)
{
    float remainder;
    static char output[39];
    output[0] = 0;
    if (value > -0.0001)
    {
        remainder = (value - (int32_t)value) * 1000.0;
        sprintf(output, "%" PRId32 ".%03" PRIu32 "", (int32_t)value, (uint32_t)remainder);
    }
    else
    {
        remainder = (-(value - (int32_t)value)) * 1000.0;
        if ((int32_t)value == 0)
        {
            sprintf(output, "-%" PRId32 ".%03" PRIu32 "", (int32_t)value, (uint32_t)remainder);
        }
        else
        {
            sprintf(output, "%" PRId32 ".%03" PRIu32 "", (int32_t)value, (uint32_t)remainder);
        }
    }

    // rounding values
    // remainder -= *decimalPart;
    // if (remainder >= 0.5)
    // {
    //     (*decimalPart)++;
    // }
    return output;
}

/*******************************************************************************
 * Luos Json data to Luos messages convertion
 ******************************************************************************/
// Convert a Json into messages
void Convert_DataToLuos(service_t *service, char *data)
{
    json_t pool[MAX_JSON_FIELDS];
    msg_t msg;

    // check if we have a complete received command
    json_t const *root = json_create(data, pool, MAX_JSON_FIELDS);
    // check json integrity
    if (root == NULL)
    {
        // Error
        return;
    }
    // check if this is a detection cmd
    if (json_getProperty(root, "detection") != NULL)
    {
        // Launch a detection
        Luos_Detect(service);
        // Run the Gate
        gate_running = PREPARING;
        return;
    }
    if (json_getProperty(root, "discover") != NULL)
    {
        PipeLink_Send(service, "{\"gate\":{}}\n", strlen("{\"gate\":{}}\n"));
        return;
    }
    if (json_getProperty(root, "baudrate") != NULL)
    {
        // create a message to setup the new baudrate
        json_t const *object = json_getProperty(root, "baudrate");
        if (json_getType(object) == JSON_INTEGER)
        {
            uint32_t baudrate = (float)json_getInteger(object);
            Luos_SendBaudrate(service, baudrate);
        }
        return;
    }
    if (json_getProperty(root, "benchmark") != NULL)
    {
        // manage benchmark
        json_t const *parameters = json_getProperty(root, "benchmark");
        if (json_getType(parameters) == JSON_OBJ)
        {
            // Get all parameters
            uint32_t repetition = (uint32_t)json_getInteger(json_getProperty(parameters, "repetitions"));
            uint32_t target_id  = (uint32_t)json_getInteger(json_getProperty(parameters, "target"));
            json_t const *item  = json_getProperty(parameters, "data");
            uint32_t size       = (uint32_t)json_getInteger(json_getChild(item));
            if (size > 0)
            {
                // find the first \r of the current buf
                int index = 0;
                for (index = 0; index < GATE_BUFF_SIZE; index++)
                {
                    if (data[index] == '\n')
                    {
                        index++;
                        break;
                    }
                }
                if (index < GATE_BUFF_SIZE - 1)
                {
                    // stop sensor polling during benchmark
                    update_time = 0.0;
                    DataManager_collect(service);
                    // create a message from parameters
                    msg.header.cmd         = REVISION;
                    msg.header.target_mode = IDACK;
                    msg.header.target      = target_id;
                    // save current time
                    uint32_t begin_systick = Luos_GetSystick();
                    uint32_t failed_msg_nb = 0;
                    // Before trying to send anything make sure to finish any transmission
                    while (Luos_TxComplete() == FAILED)
                        ;
                    // Wait 10ms allowing receiving messages to finish
                    uint32_t tickstart = Luos_GetSystick();
                    while ((Luos_GetSystick() - tickstart) < 10)
                        ;
                    // Flush every messages pending
                    Luos_Flush();
                    // To get the number of message failed we will use statistics
                    // We have to reinit the number of dropped message before start
                    uint8_t drop_back                                = service->node_statistics->memory.msg_drop_number;
                    service->node_statistics->memory.msg_drop_number = 0;
                    uint8_t retry_back                               = *service->ll_service->ll_stat.max_retry;
                    *service->ll_service->ll_stat.max_retry          = 0;
                    // send this message multiple time
                    int i = 0;
                    for (i = 0; i < repetition; i++)
                    {
                        Luos_SendData(service, &msg, &data[index], (unsigned int)size);
                    }
                    // Wait transmission end
                    while (Luos_TxComplete() == FAILED)
                        ;
                    // Get the number of failures on transmission
                    failed_msg_nb = service->node_statistics->memory.msg_drop_number;
                    // Get the number of retry
                    // If retry == max retry number consider all messages as lost
                    if (*service->ll_service->ll_stat.max_retry >= NBR_RETRY)
                    {
                        // We failed to transmit this message count all as failed
                        failed_msg_nb = repetition;
                    }
                    service->node_statistics->memory.msg_drop_number = drop_back;
                    *service->ll_service->ll_stat.max_retry          = retry_back;
                    uint32_t end_systick                             = Luos_GetSystick();
                    float data_rate                                  = (float)size * (float)(repetition - failed_msg_nb) / (((float)end_systick - (float)begin_systick) / 1000.0) * 8;
                    float fail_rate                                  = (float)failed_msg_nb * 100.0 / (float)repetition;
                    char tx_json[512];

                    sprintf(tx_json, "{\"benchmark\":{\"data_rate\":%s", Convert_Float(data_rate));
                    sprintf(tx_json, "%s\",\"fail_rate\":%s}}\n", tx_json, Convert_Float(fail_rate));
                    PipeLink_Send(service, tx_json, strlen(tx_json));

                    // restart sensor polling
                    update_time = GATE_REFRESH_TIME_S;
                    DataManager_collect(service);
                }
            }
        }
        return;
    }

    // bootloader commands
    json_t const *bootloader_json = json_getProperty(root, "bootloader");
    if (bootloader_json != 0)
    {
        Bootloader_JsonToLuos(service, (char *)data, bootloader_json);
        return;
    }

    json_t const *services = json_getProperty(root, "services");
    // Get services
    if (services != 0)
    {
        // Loop into services
        json_t const *service_jsn = json_getChild(services);
        while (service_jsn != NULL)
        {
            // Create msg
            search_result_t result;
            char *alias = (char *)json_getName(service_jsn);
            RTFilter_Alias(RTFilter_Reset(&result), alias);
            if (result.result_nbr == 0)
            {
                // If alias doesn't exist in our list id_from_alias send us back -1 = 65535
                // So here there is an error in alias.
                return;
            }
            Convert_JsonToMsg(service, result.result_table[0]->id, result.result_table[0]->type, service_jsn, &msg, (char *)data);
            // Get next service
            service_jsn = json_getSibling(service_jsn);
        }
        return;
    }
}
// Create msg from a service json data
void Convert_JsonToMsg(service_t *service, uint16_t id, luos_type_t type, const json_t *jobj, msg_t *msg, char *bin_data)
{
    time_luos_t time;
    float data = 0.0;
    json_t const *item;
    msg->header.target_mode = IDACK;
    msg->header.target      = id;
    //********** global convertion***********
    // ratio
    item = json_getProperty(jobj, "power_ratio");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        ratio_t ratio = (ratio_t)json_getReal(item);
        RatioOD_RatioToMsg(&ratio, msg);
        while (Luos_SendMsg(service, msg) == FAILED)
        {
            Luos_Loop();
        }
    }
    // target angular position
    item = json_getProperty(jobj, "target_rot_position");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        angular_position_t angular_position = AngularOD_PositionFrom_deg(json_getReal(item));
        AngularOD_PositionToMsg(&angular_position, msg);
        Luos_SendMsg(service, msg);
    }
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        int i = 0;
        // this is a trajectory
        int size = (int)json_getInteger(json_getChild(item));
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
            msg->header.cmd = ANGULAR_POSITION;
            Luos_SendData(service, msg, &bin_data[i], (unsigned int)size);
        }
    }
    // Limit angular position
    item = json_getProperty(jobj, "limit_rot_position");
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        angular_position_t limits[2];
        item      = json_getChild(item);
        limits[0] = AngularOD_PositionFrom_deg(json_getReal(item));
        item      = json_getSibling(item);
        limits[1] = AngularOD_PositionFrom_deg(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(float));
        msg->header.cmd  = ANGULAR_POSITION_LIMIT;
        msg->header.size = 2 * sizeof(float);
        Luos_SendMsg(service, msg);
    }
    // Limit linear position
    item = json_getProperty(jobj, "limit_trans_position");
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        linear_position_t limits[2];
        item      = json_getChild(item);
        limits[0] = LinearOD_PositionFrom_mm(json_getReal(item));
        item      = json_getSibling(item);
        limits[1] = LinearOD_PositionFrom_mm(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(linear_position_t));
        msg->header.cmd  = LINEAR_POSITION_LIMIT;
        msg->header.size = 2 * sizeof(linear_position_t);
        Luos_SendMsg(service, msg);
    }
    // Limit angular speed
    item = json_getProperty(jobj, "limit_rot_speed");
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        angular_speed_t limits[2];
        item      = json_getChild(item);
        limits[0] = AngularOD_SpeedFrom_deg_s(json_getReal(item));
        item      = json_getSibling(item);
        limits[1] = AngularOD_SpeedFrom_deg_s(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(float));
        msg->header.cmd  = ANGULAR_SPEED_LIMIT;
        msg->header.size = 2 * sizeof(float);
        Luos_SendMsg(service, msg);
    }
    // Limit linear speed
    item = json_getProperty(jobj, "limit_trans_speed");
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        linear_speed_t limits[2];
        item      = json_getChild(item);
        limits[0] = LinearOD_Speedfrom_mm_s(json_getReal(item));
        item      = json_getSibling(item);
        limits[1] = LinearOD_Speedfrom_mm_s(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(linear_speed_t));
        msg->header.cmd  = LINEAR_SPEED_LIMIT;
        msg->header.size = 2 * sizeof(linear_speed_t);
        Luos_SendMsg(service, msg);
    }
    // Limit ratio
    item = json_getProperty(jobj, "limit_power");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        ratio_t ratio = RatioOD_RatioFromPercent((float)json_getReal(item));
        RatioOD_RatioToMsg(&ratio, msg);
        msg->header.cmd = RATIO_LIMIT;
        Luos_SendMsg(service, msg);
    }
    // Limit current
    item = json_getProperty(jobj, "limit_current");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        current_t current = ElectricOD_CurrentFrom_A(json_getReal(item));
        ElectricOD_CurrentToMsg(&current, msg);
        Luos_SendMsg(service, msg);
    }
    // target Rotation speed
    item = json_getProperty(jobj, "target_rot_speed");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        angular_speed_t angular_speed = AngularOD_SpeedFrom_deg_s((float)json_getReal(item));
        AngularOD_SpeedToMsg(&angular_speed, msg);
        Luos_SendMsg(service, msg);
    }
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        int i = 0;
        // this is a trajectory
        int size = (int)json_getInteger(json_getChild(item));
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
            msg->header.cmd = ANGULAR_SPEED;
            Luos_SendData(service, msg, &bin_data[i], (unsigned int)size);
        }
    }
    // target linear position
    item = json_getProperty(jobj, "target_trans_position");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        linear_position_t linear_position = LinearOD_PositionFrom_mm((float)json_getReal(item));
        LinearOD_PositionToMsg(&linear_position, msg);
        Luos_SendMsg(service, msg);
    }
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        int i = 0;
        // this is a trajectory
        int size = (int)json_getInteger(json_getChild(item));
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
            msg->header.cmd = LINEAR_POSITION;
            // todo WATCHOUT this could be mm !
            Luos_SendData(service, msg, &bin_data[i], (unsigned int)size);
        }
    }
    // target Linear speed
    item = json_getProperty(jobj, "target_trans_speed");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        linear_speed_t linear_speed = LinearOD_Speedfrom_mm_s((float)json_getReal(item));
        LinearOD_SpeedToMsg(&linear_speed, msg);
        Luos_SendMsg(service, msg);
    }
    // time
    item = json_getProperty(jobj, "time");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        time = TimeOD_TimeFrom_s((float)json_getReal(item));
        TimeOD_TimeToMsg(&time, msg);
        Luos_SendMsg(service, msg);
    }
    // Pid
    item = json_getProperty(jobj, "pid");
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        float pid[3];
        item = json_getChild(item);
        for (int i = 0; i < 3; i++)
        {
            pid[i] = (float)json_getReal(item);
            item   = json_getSibling(item);
        }
        memcpy(&msg->data[0], pid, sizeof(asserv_pid_t));
        msg->header.cmd  = PID;
        msg->header.size = sizeof(asserv_pid_t);
        Luos_SendMsg(service, msg);
    }
    // resolution
    item = json_getProperty(jobj, "resolution");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        data = (float)json_getReal(item);
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd  = RESOLUTION;
        msg->header.size = sizeof(data);
        Luos_SendMsg(service, msg);
    }
    // offset
    item = json_getProperty(jobj, "offset");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        data = (float)json_getReal(item);
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd  = OFFSET;
        msg->header.size = sizeof(data);
        Luos_SendMsg(service, msg);
    }
    // reduction ratio
    item = json_getProperty(jobj, "reduction");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        data = (float)json_getReal(item);
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd  = REDUCTION;
        msg->header.size = sizeof(data);
        Luos_SendMsg(service, msg);
    }
    // dimension (m)
    item = json_getProperty(jobj, "dimension");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        linear_position_t linear_position = LinearOD_PositionFrom_mm((float)json_getReal(item));
        LinearOD_PositionToMsg(&linear_position, msg);
        // redefine a specific message type.
        msg->header.cmd = DIMENSION;
        Luos_SendMsg(service, msg);
    }
    // voltage
    item = json_getProperty(jobj, "volt");
    if ((item != NULL) && ((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        voltage_t volt = ElectricOD_VoltageFrom_V((float)json_getReal(item));
        ElectricOD_VoltageToMsg(&volt, msg);
        Luos_SendMsg(service, msg);
    }
    // reinit
    if (json_getProperty(jobj, "reinit") != NULL)
    {
        msg->header.cmd  = REINIT;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
    }
    // control (play, pause, stop, rec)
    item = json_getProperty(jobj, "control");
    if ((item != NULL) && (json_getType(item) == JSON_INTEGER))
    {
        msg->data[0]     = json_getInteger(item);
        msg->header.cmd  = CONTROL;
        msg->header.size = sizeof(control_t);
        Luos_SendMsg(service, msg);
    }
    // Color
    item = json_getProperty(jobj, "color");
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        item = json_getChild(item);
        if (json_getSibling(item) != NULL)
        {
            color_t color;
            for (int i = 0; i < 3; i++)
            {
                color.unmap[i] = (char)json_getInteger(item);
                item           = json_getSibling(item);
            }
            IlluminanceOD_ColorToMsg(&color, msg);
            Luos_SendMsg(service, msg);
        }
        else
        {
            int i = 0;
            // This is a binary
            int size = (int)json_getInteger(item);
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
                msg->header.cmd = COLOR;
                Luos_SendData(service, msg, &bin_data[i], (unsigned int)size);
            }
        }
    }
    // IO_STATE
    item = json_getProperty(jobj, "io_state");
    if ((item != NULL) && (json_getType(item) == JSON_BOOLEAN))
    {
        msg->data[0]     = json_getBoolean(item);
        msg->header.cmd  = IO_STATE;
        msg->header.size = sizeof(char);
        Luos_SendMsg(service, msg);
    }
    // update time
    item = json_getProperty(jobj, "update_time");
    if (((json_getType(item) == JSON_REAL) || (json_getType(item) == JSON_INTEGER)) && (type != GATE_TYPE))
    {
        // this should be a function because it is frequently used
        time = TimeOD_TimeFrom_s((float)json_getReal(item));
        TimeOD_TimeToMsg(&time, msg);
        msg->header.cmd = UPDATE_PUB;
        Luos_SendMsg(service, msg);
    }
    // UUID
    if (json_getProperty(jobj, "uuid") != NULL)
    {
        msg->header.cmd  = NODE_UUID;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
    }
    // RENAMING
    item = json_getProperty(jobj, "rename");
    if ((item != NULL) && (json_getType(item) == JSON_TEXT))
    {
        // In this case we need to send the message as system message
        int i            = 0;
        char *alias      = (char *)json_getValue(item);
        msg->header.size = strlen(alias);
        // Change size to fit into 16 characters
        if (msg->header.size > 15)
        {
            msg->header.size = 15;
        }
        // Clean the '\0' even if we short the alias
        alias[msg->header.size] = '\0';
        // Copy the alias into the data field of the message
        for (i = 0; i < msg->header.size; i++)
        {
            msg->data[i] = alias[i];
        }
        msg->data[msg->header.size] = '\0';
        msg->header.cmd             = WRITE_ALIAS;
        Luos_SendMsg(service, msg);
    }
    // FIRMWARE REVISION
    if (json_getProperty(jobj, "revision") != NULL)
    {
        msg->header.cmd  = REVISION;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
    }
    // Luos REVISION
    if (json_getProperty(jobj, "luos_revision") != NULL)
    {
        msg->header.cmd  = LUOS_REVISION;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
    }
    // Luos STAT
    if (json_getProperty(jobj, "luos_statistics") != NULL)
    {
        msg->header.cmd  = LUOS_STATISTICS;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
    }
    // Parameters
    item = json_getProperty(jobj, "parameters");
    if ((item != NULL) && (json_getType(item) == JSON_INTEGER))
    {
        uint32_t val = (uint32_t)json_getInteger(item);
        memcpy(msg->data, &val, sizeof(uint32_t));
        msg->header.size = 4;
        msg->header.cmd  = PARAMETERS;
        Luos_SendMsg(service, msg);
    }
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        item = json_getChild(item);
        if (json_getSibling(item) != NULL)
        {
            // We have multiple field on this array
            json_t const *val;
            int i = 0;
            for (val = item; val != 0; val = json_getSibling(val))
            {
                uint32_t value = (uint32_t)json_getInteger(val);
                memcpy(&msg->data[i * sizeof(uint32_t)], &value, sizeof(uint32_t));
                i++;
            }
            msg->header.cmd  = PARAMETERS;
            msg->header.size = i * sizeof(uint32_t);
            Luos_SendMsg(service, msg);
        }
        else
        {
            int i = 0;
            // This is a binary
            unsigned int size = (int)json_getInteger(item);
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
                msg->header.cmd = PARAMETERS;
                Luos_SendData(service, msg, &bin_data[i], (unsigned int)size);
            }
        }
    }

    item = json_getProperty(jobj, "register"); // Watch out this one is only used by Dxl and specific to it.
    if ((item != NULL) && (json_getType(item) == JSON_ARRAY))
    {
        item = json_getChild(item);
        if (json_getSibling(item) != NULL)
        {
            // We have multiple field on this array
            json_t const *val;
            int i = 0;
            for (val = item; val != 0; val = json_getSibling(val))
            {
                uint32_t value = (uint32_t)json_getInteger(val);
                memcpy(&msg->data[i * sizeof(uint32_t)], &value, sizeof(uint32_t));
                i++;
            }
            msg->header.cmd  = REGISTER;
            msg->header.size = i * sizeof(uint32_t);
            Luos_SendMsg(service, msg);
        }
        else
        {
            int i = 0;
            // This is a binary
            unsigned int size = (int)json_getInteger(item);
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
                msg->header.cmd = REGISTER;
                Luos_SendData(service, msg, &bin_data[i], (unsigned int)size);
            }
        }
    }
    item = json_getProperty(jobj, "set_id");
    if ((item != NULL) && (json_getType(item) == JSON_INTEGER))
    {
        msg->data[0]     = (char)json_getInteger(item);
        msg->header.cmd  = SETID;
        msg->header.size = sizeof(char);
        Luos_SendMsg(service, msg);
    }
    item = json_getProperty(jobj, "update_time");
    if ((item != NULL) && (json_getType(item) == JSON_INTEGER))
    {
        // Put all services with the same time value
        update_time = TimeOD_TimeFrom_s((float)json_getReal(item));
        DataManager_collect(service);
    }
}

/*******************************************************************************
 * Luos service information to Json convertion
 ******************************************************************************/
// This function start a Json structure and return the string size.
uint16_t Convert_StartData(char *data)
{
    memcpy(data, "{\"services\":{", sizeof("{\"services\":{"));
    return (sizeof("{\"services\":{") - 1);
}
// This function start a Service into a Json structure and return the string size.
uint16_t Convert_StartServiceData(char *data, char *alias)
{
    sprintf(data, "\"%s\":{", alias);
    return (uint16_t)strlen(data);
}
// This function create the Json content from a message and return the string size.
uint16_t Convert_MsgToData(msg_t *msg, char *data)
{
    float fdata;
    switch (msg->header.cmd)
    {
        case LINEAR_POSITION:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"trans_position\":%s,", Convert_Float(fdata));
            break;
        case LINEAR_SPEED:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"trans_speed\":%s,", Convert_Float(fdata));
            break;
        case ANGULAR_POSITION:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"rot_position\":%s,", Convert_Float(fdata));
            break;
        case ANGULAR_SPEED:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"rot_speed\":%s,", Convert_Float(fdata));
            break;
        case CURRENT:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"current\":%s,", Convert_Float(fdata));
            break;
        case ILLUMINANCE:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"lux\":%s,", Convert_Float(fdata));
            break;
        case TEMPERATURE:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"temperature\":%s,", Convert_Float(fdata));
            break;
        case FORCE:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"force\":%s,", Convert_Float(fdata));
            break;
        case MOMENT:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"moment\":%s,", Convert_Float(fdata));
            break;
        case VOLTAGE:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"volt\":%s,", Convert_Float(fdata));
            break;
        case POWER:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"power\":%s,", Convert_Float(fdata));
            break;
        case NODE_UUID:
            if (msg->header.size == sizeof(luos_uuid_t))
            {
                luos_uuid_t value;
                memcpy(value.unmap, msg->data, msg->header.size);
                sprintf(data, "\"uuid\":[%" PRIu32 ",%" PRIu32 ",%" PRIu32 "],", value.uuid[0], value.uuid[1], value.uuid[2]);
            }
            break;
        case REVISION:
            // clean data to be used as string
            if (msg->header.size < MAX_DATA_MSG_SIZE)
            {
                // create the Json content
                sprintf(data, "\"revision\":\"%d.%d.%d\",", msg->data[0], msg->data[1], msg->data[2]);
            }
            break;
        case LUOS_REVISION:
            // clean data to be used as string
            if (msg->header.size < MAX_DATA_MSG_SIZE)
            {
                // create the Json content
                sprintf(data, "\"luos_revision\":\"%d.%d.%d\",", msg->data[0], msg->data[1], msg->data[2]);
            }
            break;
        case LUOS_STATISTICS:
            if (msg->header.size == sizeof(general_stats_t))
            {
                general_stats_t *stat = (general_stats_t *)msg->data;
                // create the Json content
                sprintf(data, "\"luos_statistics\":{\"rx_msg_stack\":%d,\"luos_stack\":%d,\"tx_msg_stack\":%d,\"buffer_occupation\":%d,\"msg_drop\":%d,\"loop_ms\":%d,\"max_retry\":%d},",
                        stat->node_stat.memory.rx_msg_stack_ratio,
                        stat->node_stat.memory.luos_stack_ratio,
                        stat->node_stat.memory.tx_msg_stack_ratio,
                        stat->node_stat.memory.buffer_occupation_ratio,
                        stat->node_stat.memory.msg_drop_number,
                        stat->node_stat.max_loop_time_ms,
                        stat->service_stat.max_retry);
            }
            break;
        case IO_STATE:
            // check size
            if (msg->header.size == sizeof(char))
            {
                // Size ok, now fill the struct from msg data
                // create the Json content
                if (msg->data[0])
                {
                    memcpy(data, "\"io_state\":true,", sizeof("\"io_state\":true,"));
                }
                else
                {
                    memcpy(data, "\"io_state\":false,", sizeof("\"io_state\":false,"));
                }
            }
            break;
        case EULER_3D:
        case COMPASS_3D:
        case GYRO_3D:
        case ACCEL_3D:
        case LINEAR_ACCEL:
        case GRAVITY_VECTOR:
            // check size
            if (msg->header.size == (3 * sizeof(float)))
            {
                // Size ok, now fill the struct from msg data
                float value[3];
                memcpy(value, msg->data, msg->header.size);
                char name[20] = {0};
                switch (msg->header.cmd)
                {
                    case LINEAR_ACCEL:
                        strcpy(name, "linear_accel");
                        break;
                    case GRAVITY_VECTOR:
                        strcpy(name, "gravity_vector");
                        break;
                    case COMPASS_3D:
                        strcpy(name, "compass");
                        break;
                    case GYRO_3D:
                        strcpy(name, "gyro");
                        break;
                    case ACCEL_3D:
                        strcpy(name, "accel");
                        break;
                    case EULER_3D:
                        strcpy(name, "euler");
                        break;
                }
                // Create the Json content
                sprintf(data, "\"%s\":[%s", name, Convert_Float(value[0]));
                sprintf(data, "%s,%s", data, Convert_Float(value[1]));
                sprintf(data, "%s,%s],", data, Convert_Float(value[2]));
            }
            break;
        case QUATERNION:
            // check size
            if (msg->header.size == (4 * sizeof(float)))
            {
                // Size ok, now fill the struct from msg data
                float value[4];
                memcpy(value, msg->data, msg->header.size);
                // create the Json content
                sprintf(data, "\"quaternion\":[%s,", Convert_Float(value[0]));
                sprintf(data, "%s%s,", data, Convert_Float(value[1]));
                sprintf(data, "%s%s,", data, Convert_Float(value[2]));
                sprintf(data, "%s%s],", data, Convert_Float(value[3]));
            }
            break;
        case ROT_MAT:
            // check size
            if (msg->header.size == (9 * sizeof(float)))
            {
                // Size ok, now fill the struct from msg data
                float value[9];
                memcpy(value, msg->data, msg->header.size);
                // create the Json content
                sprintf(data, "\"rotational_matrix\":[%s,", Convert_Float(value[0]));
                sprintf(data, "%s%s,", data, Convert_Float(value[1]));
                sprintf(data, "%s%s,", data, Convert_Float(value[2]));
                sprintf(data, "%s%s,", data, Convert_Float(value[3]));
                sprintf(data, "%s%s,", data, Convert_Float(value[4]));
                sprintf(data, "%s%s,", data, Convert_Float(value[5]));
                sprintf(data, "%s%s,", data, Convert_Float(value[6]));
                sprintf(data, "%s%s,", data, Convert_Float(value[7]));
                sprintf(data, "%s%s],", data, Convert_Float(value[8]));
            }
            break;
        case HEADING:
            // check size
            if (msg->header.size == (sizeof(float)))
            {
                // Size ok, now fill the struct from msg data
                float value;
                memcpy(&value, msg->data, msg->header.size);
                // create the Json content
                sprintf(data, "\"heading\":%s,", Convert_Float(value));
            }
            break;
        case PEDOMETER:
            // check size
            if (msg->header.size == (2 * sizeof(unsigned long)))
            {
                // Size ok, now fill the struct from msg data
                unsigned long value[2];
                memcpy(value, msg->data, msg->header.size);
                // create the Json content
                sprintf(data, "\"pedometer\":%2ld,\"walk_time\":%2ld,", value[0], value[1]);
            }
            break;
        default:
            break;
    }
    return (uint16_t)strlen(data);
}
// This function end a Service into a Json structure and return the string size.
uint16_t Convert_EndServiceData(char *data)
{
    if (*data != '{')
    {
        // remove the last "," char
        *(--data) = '\0';
    }
    // End the service section
    memcpy(data, "},", sizeof("},"));
    return sizeof("},") - 2;
}
// This function start a Json structure and return the string size.
void Convert_EndData(service_t *service, char *data, char *data_ptr)
{
    // remove the last "," char
    *(--data_ptr) = '\0';
    // End the Json message
    memcpy(data_ptr, "}}\n", sizeof("}}\n"));
    data_ptr += sizeof("}}\n") - 1;
    // Send the message to pipe
    PipeLink_Send(service, data, data_ptr - data);
}
// If there is no message receive for sometime we need to send void Json for synchronization.
void Convert_VoidData(service_t *service)
{
    char data[sizeof("{}\n")] = "{}\n";
    PipeLink_Send(service, data, strlen("{}\n"));
}

/*******************************************************************************
 * Luos default information to Json convertion
 ******************************************************************************/
// This function generate a Json about assertion and send it.
void Convert_AssertToData(service_t *service, uint16_t source, luos_assert_t assertion)
{
    char assert_json[512];
    sprintf(assert_json, "{\"assert\":{\"node_id\":%d,\"file\":\"%s\",\"line\":%d}}\n", source, assertion.file, (unsigned int)assertion.line);
    // Send the message to pipe
    PipeLink_Send(service, assert_json, strlen(assert_json));
}
// This function generate a Json about excluded services and send it.
void Convert_ExcludedServiceData(service_t *service)
{
    char json[300];
    search_result_t result;
    RTFilter_ID(RTFilter_Reset(&result), service->ll_service->dead_service_spotted);
    sprintf(json, "{\"dead_service\":\"%s\"", result.result_table[0]->alias);
    sprintf(json, "%s}\n", json);
    // Send the message to pipe
    PipeLink_Send(service, json, strlen(json));
}
// This function is directly called by Luos_utils in case of curent node assert. DO NOT RENAME IT
void node_assert(char *file, uint32_t line)
{
    // manage self crashing scenario
    char json[512];
    sprintf(json, "{\"assert\":{\"node_id\":1,\"file\":\"%s\",\"line\":%d}}\n", file, (unsigned int)line);
    // Send the message to pipe
    PipeLink_Send(0, json, strlen(json));
}

/*******************************************************************************
 * Luos routing table information to Json convertion
 ******************************************************************************/
void Convert_RoutingTableData(service_t *service)
{
    // Init the json string
    char json[GATE_BUFF_SIZE * 2];
    char *json_ptr = json;
    sprintf(json_ptr, "{\"routing_table\":[");
    json_ptr += strlen(json_ptr);
    // loop into services.
    routing_table_t *routing_table = RoutingTB_Get();
    int last_entry                 = RoutingTB_GetLastEntry();
    int i                          = 0;
    // for (uint8_t i = 0; i < last_entry; i++) { //TODO manage all entries, not only services.
    while (i < last_entry)
    {
        if (routing_table[i].mode == NODE)
        {
            sprintf(json_ptr, "{\"node_id\":%d", routing_table[i].node_id);
            json_ptr += strlen(json_ptr);
            if (routing_table[i].certified)
            {
                sprintf(json_ptr, ",\"certified\":true");
                json_ptr += strlen(json_ptr);
            }
            else
            {
                sprintf(json_ptr, ",\"certified\":false");
                json_ptr += strlen(json_ptr);
            }
            sprintf(json_ptr, ",\"port_table\":[");
            json_ptr += strlen(json_ptr);
            // Port loop
            for (int port = 0; port < 4; port++)
            {
                if (routing_table[i].port_table[port])
                {
                    sprintf(json_ptr, "%d,", routing_table[i].port_table[port]);
                    json_ptr += strlen(json_ptr);
                }
                else
                {
                    // remove the last "," char
                    *(--json_ptr) = '\0';
                    break;
                }
            }
            sprintf(json_ptr, "],\"services\":[");
            json_ptr += strlen(json_ptr);
            i++;
            // Services loop
            while (i < last_entry)
            {
                if (routing_table[i].mode == SERVICE)
                {
                    // Create service description
                    sprintf(json_ptr, "{\"type\":\"%s\",\"id\":%d,\"alias\":\"%s\"},", Convert_StringFromType(routing_table[i].type), routing_table[i].id, routing_table[i].alias);
                    json_ptr += strlen(json_ptr);
                    i++;
                }
                else
                    break;
            }
            // remove the last "," char
            *(--json_ptr) = '\0';
            sprintf(json_ptr, "]},");
            json_ptr += strlen(json_ptr);
        }
        else
        {
            i++;
        }
    }
    // remove the last "," char
    *(--json_ptr) = '\0';
    // End the Json message
    sprintf(json_ptr, "]}\n");
    // reset all the msg in pipe link
    PipeLink_Reset(service);
    // call Luos loop to generap a Luos Task with this msg
    Luos_Loop();
    // Send the message to pipe
    PipeLink_Send(service, json, strlen(json));
}
/*******************************************************************************
 * Convert a type number to Type string
 ******************************************************************************/
const char *Convert_StringFromType(luos_type_t type)
{
    switch (type)
    {
        case STATE_TYPE:
            return "State";
            break;
        case COLOR_TYPE:
            return "Color";
            break;
        case MOTOR_TYPE:
            return "Motor";
            break;
        case SERVO_MOTOR_TYPE:
            return "ServoMotor";
            break;
        case ANGLE_TYPE:
            return "Angle";
            break;
        case DISTANCE_TYPE:
            return "Distance";
            break;
        case GATE_TYPE:
            return "Gate";
            break;
        case IMU_TYPE:
            return "Imu";
            break;
        case LIGHT_TYPE:
            return "Light";
            break;
        case VOID_TYPE:
            return "Void";
            break;
        case LOAD_TYPE:
            return "Load";
            break;
        case VOLTAGE_TYPE:
            return "Voltage";
            break;
        case PIPE_TYPE:
            return "Pipe";
            break;
        default:
            return "Unknown";
            break;
    }
}