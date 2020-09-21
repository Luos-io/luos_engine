/******************************************************************************
 * @file modules structure
 * @brief describs all the
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __MODULE_STRUCT_H
#define __MODULE_STRUCT_H

#include <stdint.h>
#include "luos.h"
#include "luos_od.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* This structure is used to manage modules timed auto update
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) timed_update_t
{
    unsigned long last_update;
    uint16_t time_ms;
    unsigned short target;
} timed_update_t;

/* This structure is used to manage modules
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) module_t
{
    vm_t *vm;
    // Callback
    void (*mod_cb)(struct module_t *module, msg_t *msg);
    // Variables
    unsigned char rt;                   /*!< is this module a real time one? */
    char default_alias[MAX_ALIAS_SIZE]; /*!< Module default alias. */
    char alias[MAX_ALIAS_SIZE];         /*!< Module alias. */
    timed_update_t auto_refresh;        /*!< Module auto refresh context. */
    char firm_version[20];              /*!< Module firmware version. */
} module_t;

typedef void (*MOD_CB)(module_t *module, msg_t *msg);

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
 * Handy struct TODO : remove it
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t index;
            uint8_t middle;
            uint8_t ring;
            uint8_t pinky;
            uint8_t thumb;
        };
        uint8_t unmap[(5 * sizeof(uint8_t))]; /*!< Uncmaped form. */
    };
} handy_t;

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

#endif /*__ __L0_H */
