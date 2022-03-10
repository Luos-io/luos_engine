/*
 * convert.h
 *
 *  Created on: 20 juil. 2018
 *      Author: nicolasrabault
 */

#ifndef CONVERT_H_
#define CONVERT_H_

#include <json_mnger.h>
#include "cJSON.h"
#include "container_structs.h"
#include "luos.h"

/*
 * Pid
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            float p;
            float i;
            float d;
        };
        unsigned char unmap[3 * sizeof(float)];
    };
} asserv_pid_t;

/*
 * Servo
 */
typedef struct
{
    union
    {
        struct __attribute__((__packed__))
        {
            angular_position_t max_angle;
            float min_pulse_time;
            float max_pulse_time;
        };
        unsigned char unmap[3 * sizeof(float)];
    };
} servo_parameters_t;

void json_to_msg(container_t *container, uint16_t id, luos_type_t type, cJSON *jobj, msg_t *msg, char *data);
void msg_to_json(msg_t *msg, char *json);
void routing_table_to_json(char *json);
void exclude_container_to_json(int id, char *json);

#endif /* CONVERT_H_ */
