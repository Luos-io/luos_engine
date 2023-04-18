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
static void Convert_JsonToMsg(service_t *service, uint16_t id, luos_type_t type, char *property, const json_t *jobj, msg_t *msg, char *bin_data);
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
 * Luos Json data to Luos messages conversion
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
            // This service is known by the gate, loop on the parameters
            json_t const *parameter_jsn = json_getChild(service_jsn);
            while (parameter_jsn != NULL)
            {
                char *property = (char *)json_getName(parameter_jsn);
                Convert_JsonToMsg(service, result.result_table[0]->id, result.result_table[0]->type, property, parameter_jsn, &msg, (char *)data);
                parameter_jsn = json_getSibling(parameter_jsn);
            }
            // Get next service
            service_jsn = json_getSibling(service_jsn);
        }
        return;
    }
}
// Create msg from a service json data
void Convert_JsonToMsg(service_t *service, uint16_t id, luos_type_t type, char *property, const json_t *jobj, msg_t *msg, char *bin_data)
{
    time_luos_t time;
    float data                   = 0.0;
    msg->header.target_mode      = SERVICEIDACK;
    msg->header.target           = id;
    const uint16_t property_type = json_getType(jobj);
    //********** global convertion***********
    // ratio
    if ((property && !strcmp(property, "power_ratio")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        ratio_t ratio = RatioOD_RatioFrom_Percent(json_getReal(jobj));
        RatioOD_RatioToMsg(&ratio, msg);
        while (Luos_SendMsg(service, msg) == FAILED)
        {
            Luos_Loop();
        }
        return;
    }
    // target angular position
    if (property && !strcmp(property, "target_rot_position"))
    {
        if ((property_type == JSON_REAL) || (property_type == JSON_INTEGER))
        {
            angular_position_t angular_position = AngularOD_PositionFrom_deg(json_getReal(jobj));
            AngularOD_PositionToMsg(&angular_position, msg);
            Luos_SendMsg(service, msg);
            return;
        }
        if (property_type == JSON_ARRAY)
        {
            int i = 0;
            // this is a trajectory
            int size = (int)json_getInteger(json_getChild(jobj));
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
            return;
        }
        return;
    }
    // Limit angular position
    if ((property && !strcmp(property, "limit_rot_position")) && (property_type == JSON_ARRAY))
    {
        angular_position_t limits[2];
        json_t const *item = json_getChild(jobj);
        limits[0]          = AngularOD_PositionFrom_deg(json_getReal(item));
        item               = json_getSibling(item);
        limits[1]          = AngularOD_PositionFrom_deg(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(float));
        msg->header.cmd  = ANGULAR_POSITION_LIMIT;
        msg->header.size = 2 * sizeof(float);
        Luos_SendMsg(service, msg);
        return;
    }
    // Limit linear position
    if ((property && !strcmp(property, "limit_trans_position")) && (property_type == JSON_ARRAY))
    {
        linear_position_t limits[2];
        json_t const *item = json_getChild(jobj);
        limits[0]          = LinearOD_PositionFrom_mm(json_getReal(item));
        item               = json_getSibling(item);
        limits[1]          = LinearOD_PositionFrom_mm(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(linear_position_t));
        msg->header.cmd  = LINEAR_POSITION_LIMIT;
        msg->header.size = 2 * sizeof(linear_position_t);
        Luos_SendMsg(service, msg);
        return;
    }
    // Limit angular speed
    if ((property && !strcmp(property, "limit_rot_speed")) && (property_type == JSON_ARRAY))
    {
        angular_speed_t limits[2];
        json_t const *item = json_getChild(jobj);
        limits[0]          = AngularOD_SpeedFrom_deg_s(json_getReal(item));
        item               = json_getSibling(item);
        limits[1]          = AngularOD_SpeedFrom_deg_s(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(float));
        msg->header.cmd  = ANGULAR_SPEED_LIMIT;
        msg->header.size = 2 * sizeof(float);
        Luos_SendMsg(service, msg);
        return;
    }
    // Limit linear speed
    if ((property && !strcmp(property, "limit_trans_speed")) && (property_type == JSON_ARRAY))
    {
        linear_speed_t limits[2];
        json_t const *item = json_getChild(jobj);
        limits[0]          = LinearOD_SpeedFrom_mm_s(json_getReal(item));
        item               = json_getSibling(item);
        limits[1]          = LinearOD_SpeedFrom_mm_s(json_getReal(item));
        memcpy(&msg->data[0], limits, 2 * sizeof(linear_speed_t));
        msg->header.cmd  = LINEAR_SPEED_LIMIT;
        msg->header.size = 2 * sizeof(linear_speed_t);
        Luos_SendMsg(service, msg);
        return;
    }
    // Limit ratio
    if ((property && !strcmp(property, "limit_power")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        ratio_t ratio = RatioOD_RatioFrom_Percent((float)json_getReal(jobj));
        RatioOD_RatioToMsg(&ratio, msg);
        msg->header.cmd = RATIO_LIMIT;
        Luos_SendMsg(service, msg);
        return;
    }
    // Limit current
    if ((property && !strcmp(property, "limit_current")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        current_t current = ElectricOD_CurrentFrom_A(json_getReal(jobj));
        ElectricOD_CurrentToMsg(&current, msg);
        msg->header.cmd = CURRENT_LIMIT;
        Luos_SendMsg(service, msg);
        return;
    }
    // target Rotation speed
    if (property && !strcmp(property, "target_rot_speed"))
    {
        if ((property_type == JSON_REAL) || (property_type == JSON_INTEGER))
        {
            // this should be a function because it is frequently used
            angular_speed_t angular_speed = AngularOD_SpeedFrom_deg_s((float)json_getReal(jobj));
            AngularOD_SpeedToMsg(&angular_speed, msg);
            Luos_SendMsg(service, msg);
            return;
        }
        if (property_type == JSON_ARRAY)
        {
            int i = 0;
            // this is a trajectory
            int size = (int)json_getInteger(json_getChild(jobj));
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
            return;
        }
        return;
    }
    // target linear position
    if (property && !strcmp(property, "target_trans_position"))
    {
        if ((property_type == JSON_REAL) || (property_type == JSON_INTEGER))
        {
            linear_position_t linear_position = LinearOD_PositionFrom_mm((float)json_getReal(jobj));
            LinearOD_PositionToMsg(&linear_position, msg);
            Luos_SendMsg(service, msg);
            return;
        }
        if (property_type == JSON_ARRAY)
        {
            int i = 0;
            // this is a trajectory
            int size = (int)json_getInteger(json_getChild(jobj));
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
            return;
        }
    }
    // target Linear speed
    if ((property && !strcmp(property, "target_trans_speed")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        linear_speed_t linear_speed = LinearOD_SpeedFrom_mm_s((float)json_getReal(jobj));
        LinearOD_SpeedToMsg(&linear_speed, msg);
        Luos_SendMsg(service, msg);
        return;
    }
    // time
    if ((property && !strcmp(property, "time")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        time = TimeOD_TimeFrom_s((float)json_getReal(jobj));
        TimeOD_TimeToMsg(&time, msg);
        Luos_SendMsg(service, msg);
        return;
    }
    // Pid
    if ((property && !strcmp(property, "pid")) && (property_type == JSON_ARRAY))
    {
        float pid[3];
        json_t const *item = json_getChild(jobj);
        for (int i = 0; i < 3; i++)
        {
            pid[i] = (float)json_getReal(item);
            item   = json_getSibling(item);
        }
        memcpy(&msg->data[0], pid, sizeof(asserv_pid_t));
        msg->header.cmd  = PID;
        msg->header.size = sizeof(asserv_pid_t);
        Luos_SendMsg(service, msg);
        return;
    }
    // resolution
    if ((property && !strcmp(property, "resolution")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        data = (float)json_getReal(jobj);
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd  = RESOLUTION;
        msg->header.size = sizeof(data);
        Luos_SendMsg(service, msg);
        return;
    }
    // offset
    if ((property && !strcmp(property, "offset")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        data = (float)json_getReal(jobj);
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd  = OFFSET;
        msg->header.size = sizeof(data);
        Luos_SendMsg(service, msg);
        return;
    }
    // reduction ratio
    if ((property && !strcmp(property, "reduction")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        data = (float)json_getReal(jobj);
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd  = REDUCTION;
        msg->header.size = sizeof(data);
        Luos_SendMsg(service, msg);
        return;
    }
    // dimension (m)
    if ((property && !strcmp(property, "dimension")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        linear_position_t linear_position = LinearOD_PositionFrom_mm((float)json_getReal(jobj));
        LinearOD_PositionToMsg(&linear_position, msg);
        // redefine a specific message type.
        msg->header.cmd = DIMENSION;
        Luos_SendMsg(service, msg);
        return;
    }
    // voltage
    if ((property && !strcmp(property, "volt")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        voltage_t volt = ElectricOD_VoltageFrom_V((float)json_getReal(jobj));
        ElectricOD_VoltageToMsg(&volt, msg);
        Luos_SendMsg(service, msg);
        return;
    }
    // reinit
    if (property && !strcmp(property, "reinit"))
    {
        msg->header.cmd  = REINIT;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
        return;
    }
    // control (play, pause, stop, rec)
    if ((property && !strcmp(property, "control")) && (property_type == JSON_INTEGER))
    {
        msg->data[0]     = json_getInteger(jobj);
        msg->header.cmd  = CONTROL;
        msg->header.size = sizeof(control_t);
        Luos_SendMsg(service, msg);
        return;
    }
    // Pressure
    if ((property && !strcmp(property, "pressure")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        // this should be a function because it is frequently used
        data = (float)json_getReal(jobj);
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd  = PRESSURE;
        msg->header.size = sizeof(data);
        Luos_SendMsg(service, msg);
        return;
    }
    // Color
    if ((property && !strcmp(property, "color")) && (property_type == JSON_ARRAY))
    {
        json_t const *item = json_getChild(jobj);
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
        return;
    }
    // IO_STATE
    if ((property && !strcmp(property, "io_state")) && (property_type == JSON_BOOLEAN))
    {
        msg->data[0]     = json_getBoolean(jobj);
        msg->header.cmd  = IO_STATE;
        msg->header.size = sizeof(char);
        Luos_SendMsg(service, msg);
        return;
    }
    // update time
    if ((property && !strcmp(property, "update_time")) && ((property_type == JSON_REAL) || (property_type == JSON_INTEGER)))
    {
        if (type != GATE_TYPE)
        {
            // this should be a function because it is frequently used
            time = TimeOD_TimeFrom_s((float)json_getReal(jobj));
            TimeOD_TimeToMsg(&time, msg);
            msg->header.cmd = UPDATE_PUB;
            Luos_SendMsg(service, msg);
        }
        else
        {
            // Put all services with the same time value
            update_time = TimeOD_TimeFrom_s((float)json_getReal(jobj));
            DataManager_collect(service);
        }
        return;
    }
    // RENAMING
    if ((property && !strcmp(property, "rename")) && (property_type == JSON_TEXT))
    {
        // In this case we need to send the message as system message
        int i            = 0;
        char *alias      = (char *)json_getValue(jobj);
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
        return;
    }
    // FIRMWARE REVISION
    if (property && !strcmp(property, "revision"))
    {
        msg->header.cmd  = REVISION;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
        return;
    }
    // Luos REVISION
    if (property && !strcmp(property, "luos_revision"))
    {
        msg->header.cmd  = LUOS_REVISION;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
        return;
    }
    // Luos STAT
    if (property && !strcmp(property, "luos_statistics"))
    {
        msg->header.cmd  = LUOS_STATISTICS;
        msg->header.size = 0;
        Luos_SendMsg(service, msg);
        return;
    }
    // Parameters
    if (property && !strcmp(property, "parameters"))
    {
        if (property_type == JSON_INTEGER)
        {
            uint32_t val = (uint32_t)json_getInteger(jobj);
            memcpy(msg->data, &val, sizeof(uint32_t));
            msg->header.size = 4;
            msg->header.cmd  = PARAMETERS;
            Luos_SendMsg(service, msg);
            return;
        }
        if (property_type == JSON_ARRAY)
        {
            json_t const *item = json_getChild(jobj);
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
                unsigned int size = (int)json_getInteger(jobj);
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
            return;
        }
        return;
    }
    // Register
    if ((property && !strcmp(property, "register")) && (property_type == JSON_ARRAY)) // Watch out this one is only used by Dxl and specific to it.
    {
        json_t const *item = json_getChild(jobj);
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
        return;
    }
    // Set_id
    if ((property && !strcmp(property, "set_id")) && (property_type == JSON_INTEGER))
    {
        msg->data[0]     = (char)json_getInteger(jobj);
        msg->header.cmd  = SETID;
        msg->header.size = sizeof(char);
        Luos_SendMsg(service, msg);
        return;
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
        case PRESSURE:
            memcpy(&fdata, msg->data, sizeof(float));
            sprintf(data, "\"pressure\":%s,", Convert_Float(fdata));
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
                        stat->node_stat.memory.engine_msg_stack_ratio,
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
                sprintf(data, "\"%s\":[%s,", name, Convert_Float(value[0]));
                strcat(data, Convert_Float(value[1]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[2]));
                strcat(data, "],");
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
                strcat(data, Convert_Float(value[1]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[2]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[3]));
                strcat(data, "],");
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
                strcat(data, Convert_Float(value[1]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[2]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[3]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[4]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[5]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[6]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[7]));
                strcat(data, ",");
                strcat(data, Convert_Float(value[8]));
                strcat(data, "],");
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
    RTFilter_ID(RTFilter_Reset(&result), service->dead_service_spotted);
    if (result.result_nbr == 0)
    {
        // This can happen if a service is excluded during the detection.
        return;
    }
    sprintf(json, "{\"dead_service\":\"%s\"}\n", result.result_table[0]->alias);
    // Send the message to pipe
    PipeLink_Send(service, json, strlen(json));
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
        case PRESSURE_TYPE:
            return "Pressure";
            break;
        default:
            return "Unknown";
            break;
    }
}