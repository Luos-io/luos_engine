#include "convert.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Create msg from a module json data
void json_to_msg(module_t *module, uint16_t id, module_type_t type, cJSON *jobj, msg_t* msg, char* bin_data) {
    time_luos_t time;
    float data = 0.0;
    cJSON* item;
    msg->header.target_mode = IDACK;
    msg->header.target = id;
    //********** global convertion***********
    // ratio
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "power_ratio"))) {
        ratio_t ratio = (ratio_t)cJSON_GetObjectItem(jobj, "power_ratio")->valuedouble;
        ratio_to_msg(&ratio, msg);
        luos_send(module, msg);
    }
    // target angular position
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "target_rot_position"))) {
        angular_position_t angular_position = (angular_position_t)cJSON_GetObjectItem(jobj, "target_rot_position")->valuedouble;
        angular_position_to_msg(&angular_position, msg);
        luos_send(module, msg);
    }
    if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "target_rot_position"))) {
        int i = 0;
        // this is a trajectory
        item = cJSON_GetObjectItem(jobj, "target_rot_position");
        int size = (int)cJSON_GetArrayItem(item, 0)->valueint;
        // find the first \r of the current buf
        for (i = 0; i < JSON_BUFF_SIZE; i++) {
            if (bin_data[i] == '\r') {
                i++;
                break;
            }
        }
        if (i < JSON_BUFF_SIZE-1) {
            msg->header.cmd = ANGULAR_POSITION;
            luos_send_data(module, msg, &bin_data[i], (unsigned int)size);
        }
    }
    // Limit angular position
    if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "limit_rot_position"))) {
        angular_position_t limits[2];
        item = cJSON_GetObjectItem(jobj, "limit_rot_position");
        limits[0] = angular_position_from_deg(cJSON_GetArrayItem(item, 0)->valuedouble);
        limits[1] = angular_position_from_deg(cJSON_GetArrayItem(item, 1)->valuedouble);
        memcpy(&msg->data[0], limits, 2 * sizeof(float));
        msg->header.cmd = ANGULAR_POSITION_LIMIT;
        msg->header.size = 2 * sizeof(float);
        luos_send(module, msg);
    }
    // Limit linear position
    if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "limit_trans_position"))) {
        linear_position_t limits[2];
        item = cJSON_GetObjectItem(jobj, "limit_trans_position");
        limits[0] = linear_position_from_mm((float)cJSON_GetArrayItem(item, 0)->valuedouble);
        limits[1] = linear_position_from_mm((float)cJSON_GetArrayItem(item, 1)->valuedouble);
        memcpy(&msg->data[0], limits, 2 * sizeof(linear_position_t));
        msg->header.cmd = LINEAR_POSITION_LIMIT;
        msg->header.size = 2 * sizeof(linear_position_t);
        luos_send(module, msg);
    }
    // Limit ratio
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "limit_power"))) {
        data = (float)cJSON_GetObjectItem(jobj, "limit_power")->valuedouble;
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd = RATIO_LIMIT;
        msg->header.size = sizeof(float);
        luos_send(module, msg);
    }
    // Limit current
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "limit_current"))) {
        current_t current = (current_t)cJSON_GetObjectItem(jobj, "limit_current")->valuedouble;
        current_to_msg(&current, msg);
        luos_send(module, msg);
    }
    // target Rotation speed
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "target_rot_speed"))) {
        // this should be a function because it is frequently used
        angular_speed_t angular_speed = (angular_speed_t)cJSON_GetObjectItem(jobj, "target_rot_speed")->valuedouble;
        angular_speed_to_msg(&angular_speed, msg);
        luos_send(module, msg);
    }
    // target linear position
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "target_trans_position"))) {
        linear_position_t linear_position = linear_position_from_mm((float)cJSON_GetObjectItem(jobj, "target_trans_position")->valuedouble);
        linear_position_to_msg(&linear_position, msg);
        luos_send(module, msg);
    }
    if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "target_trans_position"))) {
        int i = 0;
        // this is a trajectory
        item = cJSON_GetObjectItem(jobj, "target_trans_position");
        int size = (int)cJSON_GetArrayItem(item, 0)->valueint;
        // find the first \r of the current buf
        for (i = 0; i < JSON_BUFF_SIZE; i++) {
            if (bin_data[i] == '\r') {
                i++;
                break;
            }
        }
        if (i < JSON_BUFF_SIZE-1) {
            msg->header.cmd = LINEAR_POSITION;
            // todo WATCHOUT this could be mm !
            luos_send_data(module, msg, &bin_data[i], (unsigned int)size);
        }
    }
    // target Linear speed
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "target_trans_speed"))) {
        linear_speed_t linear_speed = linear_speed_from_mm_s((float)cJSON_GetObjectItem(jobj, "target_trans_speed")->valuedouble);
        linear_speed_to_msg(&linear_speed, msg);
        luos_send(module, msg);
    }
    // time
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "time"))) {
        // this should be a function because it is frequently used
        time =time_from_sec((float)cJSON_GetObjectItem(jobj, "time")->valuedouble);
        time_to_msg(&time,msg);
        luos_send(module, msg);
    }
    // Compliance
    if (cJSON_IsBool(cJSON_GetObjectItem(jobj, "compliant"))) {
        msg->data[0] = cJSON_IsTrue(cJSON_GetObjectItem(jobj, "compliant"));
        msg->header.cmd = COMPLIANT;
        msg->header.size = sizeof(char);
        luos_send(module, msg);
    }
    // Pid
    if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "pid"))) {
        float pid[3];
        item = cJSON_GetObjectItem(jobj, "pid");
        pid[0] = (float)cJSON_GetArrayItem(item, 0)->valuedouble;
        pid[1] = (float)cJSON_GetArrayItem(item, 1)->valuedouble;
        pid[2] = (float)cJSON_GetArrayItem(item, 2)->valuedouble;
        memcpy(&msg->data[0], pid, sizeof(asserv_pid_t));
        msg->header.cmd = PID;
        msg->header.size = sizeof(asserv_pid_t);
        luos_send(module, msg);
    }
    // resolution
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "resolution"))) {
        // this should be a function because it is frequently used
        data = (float)cJSON_GetObjectItem(jobj, "resolution")->valuedouble;
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd = RESOLUTION;
        msg->header.size = sizeof(data);
        luos_send(module, msg);
    }
    //offset
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "offset"))) {
        // this should be a function because it is frequently used
        data = (float)cJSON_GetObjectItem(jobj, "offset")->valuedouble;
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd = OFFSET;
        msg->header.size = sizeof(data);
        luos_send(module, msg);
    }
    // reduction ratio
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "reduction"))) {
        // this should be a function because it is frequently used
        data = (float)cJSON_GetObjectItem(jobj, "reduction")->valuedouble;
        memcpy(msg->data, &data, sizeof(data));
        msg->header.cmd = REDUCTION;
        msg->header.size = sizeof(data);
        luos_send(module, msg);
    }
    // dimension (m)
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "dimension"))) {
        linear_position_t linear_position = linear_position_from_mm((float)cJSON_GetObjectItem(jobj, "dimension")->valuedouble);
        linear_position_to_msg(&linear_position, msg);
        // redefine a specific message type.
        msg->header.cmd = DIMENSION;
        luos_send(module, msg);
    }
    // voltage
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "volt"))) {
        // this should be a function because it is frequently used
        voltage_t volt = (voltage_t)cJSON_GetObjectItem(jobj, "volt")->valuedouble;
        voltage_to_msg(&volt, msg);
        luos_send(module, msg);
    }
    // reinit
    if (cJSON_GetObjectItem(jobj, "reinit")) {
        msg->header.cmd = REINIT;
        msg->header.size = 0;
        luos_send(module, msg);
    }
    // control (play, pause, stop, rec)
    if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "control"))) {
        msg->data[0] = cJSON_GetObjectItem(jobj, "control")->valueint;
        msg->header.cmd = CONTROL;
        msg->header.size = sizeof(control_mode_t);
        luos_send(module, msg);
    }
    // Color
    if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "color"))) {
        item = cJSON_GetObjectItem(jobj, "color");
        int size = cJSON_GetArraySize(item);
        if (size == 3) {
            color_t color;
            for (int i = 0; i < 3; i++) {
                color.unmap[i] = (char)cJSON_GetArrayItem(item, i)->valueint;
            }
            color_to_msg(&color, msg);
            luos_send(module, msg);
        } else {
            int i = 0;
            // This is a binary
            size = (int)cJSON_GetArrayItem(item, 0)->valueint;
            // find the first \r of the current buf
            for (i = 0; i < JSON_BUFF_SIZE; i++) {
                if (bin_data[i] == '\r') {
                    i++;
                    break;
                }
            }
            if (i < JSON_BUFF_SIZE-1) {
                msg->header.cmd = COLOR;
                luos_send_data(module, msg, &bin_data[i], (unsigned int)size);
            }
        }
    }
    // IO_STATE
    if (cJSON_IsBool(cJSON_GetObjectItem(jobj, "io_state"))) {
        msg->data[0] = cJSON_IsTrue(cJSON_GetObjectItem(jobj, "io_state"));
        msg->header.cmd = IO_STATE;
        msg->header.size = sizeof(char);
        luos_send(module, msg);
    }
    // NODE_LED
    if (cJSON_IsBool(cJSON_GetObjectItem(jobj, "led"))) {
        msg->data[0] = cJSON_IsTrue(cJSON_GetObjectItem(jobj, "led"));
        msg->header.cmd = NODE_LED;
        msg->header.size = sizeof(char);
        luos_send(module, msg);
    }
    // NODE_TEMPERATURE
    if (cJSON_GetObjectItem(jobj, "node_temperature")) {
        msg->header.cmd = NODE_TEMPERATURE;
        msg->header.size = 0;
        luos_send(module, msg);
    }
    // NODE_VOLTAGE
    if (cJSON_GetObjectItem(jobj, "node_voltage")) {
        msg->header.cmd = NODE_VOLTAGE;
        msg->header.size = 0;
        luos_send(module, msg);
    }
    // UUID
    if (cJSON_GetObjectItem(jobj, "uuid")) {
        msg->header.cmd = NODE_UUID;
        msg->header.size = 0;
        luos_send(module, msg);
    }
    // RENAMING
    if (cJSON_IsString(cJSON_GetObjectItem(jobj, "rename"))) {
        // In this case we need to send the message as system message
        int i = 0;
        char* alias = cJSON_GetStringValue(cJSON_GetObjectItem(jobj, "rename"));
        msg->header.size = strlen(alias);
        // Change size to fit into 16 characters
        if (msg->header.size > 15) {
                msg->header.size = 15;
        }
        // Clean the '\0' even if we short the alias
        alias[msg->header.size] = '\0';
        // Copy the alias into the data field of the message
        for (i=0; i < msg->header.size; i++) {
            msg->data[i] = alias[i];
        }
        msg->data[msg->header.size] = '\0';
        msg->header.cmd = WRITE_ALIAS;
        luos_send(module, msg);
    }
    // FIRMWARE REVISION
    if (cJSON_GetObjectItem(jobj, "revision")) {
        msg->header.cmd = REVISION;
        msg->header.size = 0;
        luos_send(module, msg);
    }
    handy_t position;
    switch (type) {
        case VOID_MOD:
        case DYNAMIXEL_MOD:
            if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "register"))) {
                item = cJSON_GetObjectItem(jobj, "register");
                int val = (int)cJSON_GetArrayItem(item, 0)->valueint;
                memcpy(&msg->data[0],&val, sizeof(uint16_t));
                val = (int)cJSON_GetArrayItem(item, 1)->valueint;
                if (val <= 0xFF) {
                    memcpy(&msg->data[2], &val, sizeof(uint8_t));
                    msg->header.size = sizeof(uint16_t) + sizeof(uint8_t);
                } else if (val <= 0xFFFF) {
                    memcpy(&msg->data[2], &val, sizeof(uint16_t));
                    msg->header.size = sizeof(uint16_t) + sizeof(uint16_t);
                } else {
                    memcpy(&msg->data[2], &val, sizeof(uint32_t));
                    msg->header.size = sizeof(uint16_t) + sizeof(uint32_t);
                }
                msg->header.cmd = REGISTER;
                luos_send(module, msg);
            }
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "set_id"))) {
                msg->data[0] = (char)(cJSON_GetObjectItem(jobj, "set_id")->valueint);
                msg->header.cmd = SETID;
                msg->header.size = sizeof(char);
                luos_send(module, msg);
            }
            if (cJSON_IsBool(cJSON_GetObjectItem(jobj, "wheel_mode"))) {
                msg->data[0] = cJSON_IsTrue(cJSON_GetObjectItem(jobj, "wheel_mode"));
                msg->header.cmd = DXL_WHEELMODE;
                msg->header.size = sizeof(char);
                luos_send(module, msg);
            }
        break;
        case HANDY_MOD:
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "pinky"))) {
                position.pinky = (char)(cJSON_GetObjectItem(jobj, "pinky")->valueint);
                if (position.pinky > 100) position.pinky = 100;
            } else {
                position.pinky = 200;
            }
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "index"))) {
                position.index = (char)(cJSON_GetObjectItem(jobj, "index")->valueint);
                if (position.index > 100) position.index = 100;
            } else {
                position.index = 200;
            }
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "middle"))) {
                position.middle = (char)(cJSON_GetObjectItem(jobj, "middle")->valueint);
                if (position.middle > 100) position.middle = 100;
            } else {
                position.middle = 200;
            }
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "ring"))) {
                position.ring = (char)(cJSON_GetObjectItem(jobj, "ring")->valueint);
                if (position.ring > 100) position.ring = 100;
            } else {
                position.ring = 200;
            }
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "thumb"))) {
                position.thumb = (char)(cJSON_GetObjectItem(jobj, "thumb")->valueint);
                if (position.thumb > 100) position.thumb = 100;
            } else {
                position.thumb = 200;
            }
            memcpy(msg->data, &position.unmap, sizeof(handy_t));
            msg->header.cmd = HANDY_SET_POSITION;
            msg->header.size = sizeof(handy_t);
            luos_send(module, msg);
        break;
        case IMU_MOD:
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "parameters"))) {
                uint16_t val = cJSON_GetObjectItem(jobj, "parameters")->valueint;
                memcpy(msg->data, &val, sizeof(uint16_t));
                msg->header.cmd = PARAMETERS;
                msg->header.size = sizeof(imu_report_t);
                luos_send(module, msg);
            }
        break;
        case STEPPER_MOD:
        case CONTROLLED_MOTOR_MOD:
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "parameters"))) {
                uint16_t val = cJSON_GetObjectItem(jobj, "parameters")->valueint;
                memcpy(msg->data, &val, sizeof(uint16_t));
                msg->header.cmd = PARAMETERS;
                msg->header.size = sizeof(motor_mode_t);
                luos_send(module, msg);
            }
        break;
        case SERVO_MOD:
            if (cJSON_IsArray(cJSON_GetObjectItem(jobj, "parameters"))) {
                servo_parameters_t servo_param;
                item = cJSON_GetObjectItem(jobj, "parameters");
                servo_param.max_angle = (float)cJSON_GetArrayItem(item, 0)->valuedouble;
                servo_param.min_pulse_time = (float)cJSON_GetArrayItem(item, 1)->valuedouble;
                servo_param.max_pulse_time = (float)cJSON_GetArrayItem(item, 2)->valuedouble;
                memcpy(msg->data, servo_param.unmap, sizeof(servo_parameters_t));
                msg->header.cmd = PARAMETERS;
                msg->header.size = sizeof(servo_parameters_t);
                luos_send(module, msg);
            }
        break;
        case GATE_MOD:
            if (cJSON_IsNumber(cJSON_GetObjectItem(jobj, "delay"))) {
                set_delay(cJSON_GetObjectItem(jobj, "delay")->valueint);
            }
        break;
        default:
        break;
    }

}

// Create Json from a module msg
void msg_to_json(msg_t* msg, char *json) {
    switch (msg->header.cmd) {
    case NODE_TEMPERATURE:
    case NODE_VOLTAGE:
    case LINEAR_POSITION:
    case LINEAR_SPEED:
    case ANGULAR_POSITION:
    case ANGULAR_SPEED:
    case VOLTAGE:
    case CURRENT:
    case POWER:
    case ILLUMINANCE :
    case TEMPERATURE:
    case FORCE:
    case MOMENT:
        // check size
        if (msg->header.size == sizeof(float)) {
            // Size ok, now fill the struct from msg data
            float data;
            memcpy(&data, msg->data, msg->header.size);
            char name[20] = {0};
            switch (msg->header.cmd) {
                case NODE_TEMPERATURE: strcpy(name , "node_temperature"); break;
                case NODE_VOLTAGE: strcpy(name , "node_voltage"); break;
                case LINEAR_POSITION: strcpy(name , "trans_position"); break;
                case LINEAR_SPEED: strcpy(name , "trans_speed"); break;
                case ANGULAR_POSITION: strcpy(name , "rot_position"); break;
                case ANGULAR_SPEED: strcpy(name , "rot_speed"); break;
                case CURRENT: strcpy(name , "current"); break;
                case ILLUMINANCE: strcpy(name , "lux"); break;
                case TEMPERATURE: strcpy(name , "temperature"); break;
                case FORCE: strcpy(name , "force"); break;
                case MOMENT: strcpy(name , "moment"); break;
                case VOLTAGE: strcpy(name , "volt"); break;
                case POWER: strcpy(name , "power"); break;
            }
            //create the Json content
            sprintf(json, "\"%s\":%.3f,",name, data);
        }
    break;
    case NODE_UUID:
        if (msg->header.size == sizeof(luos_uuid_t)) {
            luos_uuid_t value;
            memcpy(value.unmap, msg->data, msg->header.size);
            sprintf(json, "\"uuid\":[%"PRIu32",%"PRIu32",%"PRIu32"],", value.uuid[0], value.uuid[1], value.uuid[2]);
        }
    break;
    case REVISION:
        // clean data to be used as string
        if (msg->header.size < MAX_DATA_MSG_SIZE) {
            msg->data[msg->header.size] = '\0';
            //create the Json content
            sprintf(json, "\"revision\":\"%s\",", msg->data);
        }
    break;
    case IO_STATE:
        // check size
        if (msg->header.size == sizeof(char)) {
            // Size ok, now fill the struct from msg data
            //create the Json content
            if (msg-> data[0]) {
                sprintf(json, "\"io_state\":true,");
            } else {
                sprintf(json, "\"io_state\":false,");
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
        if (msg->header.size == (3 * sizeof(float))) {
            // Size ok, now fill the struct from msg data
            float value[3];
            memcpy(value, msg->data, msg->header.size);
            char name[20] = {0};
            switch (msg->header.cmd) {
                case LINEAR_ACCEL: strcpy(name , "linear_accel"); break;
                case GRAVITY_VECTOR: strcpy(name , "gravity_vector"); break;
                case COMPASS_3D: strcpy(name , "compass"); break;
                case GYRO_3D: strcpy(name , "gyro"); break;
                case ACCEL_3D: strcpy(name , "accel"); break;
                case EULER_3D: strcpy(name , "euler"); break;
            }
            //create the Json content
            sprintf(json, "\"%s\":[%2f,%2f,%2f],", name, value[0], value[1], value[2]);
        }
    break;
    case QUATERNION:
        // check size
        if (msg->header.size == (4 * sizeof(float))) {
            // Size ok, now fill the struct from msg data
            float value[4];
            memcpy(value, msg->data, msg->header.size);
            //create the Json content
            sprintf(json, "\"quaternion\":[%2f,%2f,%2f,%2f],", value[0], value[1], value[2], value[3]);
        }
    break;
    case ROT_MAT:
        // check size
        if (msg->header.size == (9 * sizeof(float))) {
            // Size ok, now fill the struct from msg data
            float value[9];
            memcpy(value, msg->data, msg->header.size);
            //create the Json content
            sprintf(json, "\"rotational_matrix\":[%2f,%2f,%2f,%2f,%2f,%2f,%2f,%2f,%2f],", value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7], value[8]);
        }
    break;
    case HEADING:
        // check size
        if (msg->header.size == (sizeof(float))) {
            // Size ok, now fill the struct from msg data
            float value;
            memcpy(&value, msg->data, msg->header.size);
            //create the Json content
            sprintf(json, "\"heading\":%2f,", value);
        }
    break;
    case PEDOMETER:
        // check size
        if (msg->header.size == (2 * sizeof(unsigned long))) {
            // Size ok, now fill the struct from msg data
            unsigned long value[2];
            memcpy(value, msg->data, msg->header.size);
            //create the Json content
            sprintf(json, "\"pedometer\":%2ld,\"walk_time\":%2ld,", value[0], value[1]);
        }
    break;
    default:
    break;
    }
}

void route_table_to_json(char *json) {
    // Init the json string
    sprintf(json, "{\"route_table\":[");
    // loop into modules.
    route_table_t* route_table = get_route_table();
    int last_entry = get_last_entry();
    int i = 0;
    //for (uint8_t i = 0; i < last_entry; i++) { //TODO manage all entries, not only modules.
    while(i < last_entry) {
        if (route_table[i].mode == NODE) {
            sprintf(json, "%s{\"uuid\":[%"PRIu32",%"PRIu32",%"PRIu32"]", json, route_table[i].uuid.uuid[0], route_table[i].uuid.uuid[1], route_table[i].uuid.uuid[2]);
            sprintf(json, "%s,\"port_table\":[", json);
            // Port loop
            for (int port = 0; port < 4; port++) {
                if (route_table[i].port_table[port]) {
                    sprintf(json, "%s%d,", json, route_table[i].port_table[port]);
                } else {
                    // remove the last "," char
                    json[strlen(json) - 1] = '\0';
                    break;
                }
            }
            sprintf(json, "%s],\"modules\":[", json);
            i++;
            // Modules loop
            while (i < last_entry) {
                if (route_table[i].mode == MODULE){
                    // Create module description
                    sprintf(json, "%s{\"type\":\"%s", json, string_from_type(route_table[i].type));
                    sprintf(json, "%s\",\"id\":%d", json, route_table[i].id);
                    sprintf(json, "%s,\"alias\":\"%s\"},", json, route_table[i].alias);
                    i++;
                } else break;
            }
            // remove the last "," char
            json[strlen(json) - 1] = '\0';
            sprintf(json, "%s]},", json);
        }
    }
    // remove the last "," char
    json[strlen(json) - 1] = '\0';
    // End the Json message
    sprintf(json, "%s]}\n", json);
}

void exclude_module_to_json(int id, char* json) {
    sprintf(json, "%s", "{\"dead_module\": ");
    sprintf(json, "%s\"%s\"", json, alias_from_id(id));
    sprintf(json, "%s}\n", json);
    remove_on_route_table(id);
}
