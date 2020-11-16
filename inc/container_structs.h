/******************************************************************************
 * @file containers structure
 * @brief describs all the
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __CONTAINER_STRUCT_H
#define __CONTAINER_STRUCT_H

#include <stdint.h>
#include "luos.h"
#include "luos_od.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* This structure is used to manage containers timed auto update
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) timed_update_t
{
    uint32_t last_update;
    uint16_t time_ms;
    uint16_t target;
} timed_update_t;

/* This structure is used to manage containers
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) container_t
{
    ll_container_t *ll_container;
    // Callback
    void (*cont_cb)(struct container_t *container, msg_t *msg);
    // Variables
    uint8_t default_alias[MAX_ALIAS_SIZE]; /*!< container default alias. */
    uint8_t alias[MAX_ALIAS_SIZE];         /*!< container alias. */
    timed_update_t auto_refresh;        /*!< container auto refresh context. */
    uint8_t firm_version[20];              /*!< container firmware version. */
} container_t;

typedef void (*CONT_CB)(container_t *container, msg_t *msg);

/*
 * Control modes
 */
typedef enum
{
    PLAY,
    PAUSE,
    STOP
} control_type_t;

/*
 * Luos unic ID => ARM serial number
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        uint32_t uuid[3];
        uint8_t unmap[3 * sizeof(uint32_t)]; /*!< Uncmaped form. */
    };
} luos_uuid_t;

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

/*
 * controle
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            // control modes
            uint8_t mode_control : 2;
            uint8_t mode_rec : 1;
        };
        uint8_t unmap;
    };
} control_mode_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /*__CONTAINER_STRUCT_H */
