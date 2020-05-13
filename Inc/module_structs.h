#ifndef __MODULE_STRUCT_H
#define __MODULE_STRUCT_H

#include <stdint.h>
#include "luos.h"
#include "luos_od.h"

#define MIN 0
#define MAX 1



/**
 * \struct timed_update_t
 * \brief timed update informations Structure
 *
 * This structure is used to manage modules timed auto update 
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) timed_update_t
{
    unsigned long last_update;
    uint16_t time_ms;
    unsigned short target;
} timed_update_t;

/**
 * \struct module_t
 * \brief Module Structure
 *
 * This structure is used to manage modules
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) module_t
{
    vm_t *vm;
    // Callback
    void (*mod_cb)(struct module_t *module, msg_t *msg);
    // Variables
    unsigned char message_available;    /*!< signal a new message available */
    msg_t *msg_stack[MSG_BUFFER_SIZE];  /*!< msg ready to be read */
    unsigned char rt;                   /*!< is this module a real time one? */
    char default_alias[MAX_ALIAS_SIZE]; /*!< Module default alias. */
    char alias[MAX_ALIAS_SIZE];         /*!< Module alias. */
    timed_update_t auto_refresh;        /*!< Module auto refresh context. */
    char firm_version[20];        /*!< Module firmware version. */
} module_t;

typedef void (*MOD_CB)(module_t *module, msg_t *msg);

// This file contain struct of messages exchanged between gate and modules

// Luos unic ID => ARM serial number
typedef struct __attribute__((__packed__))
{
    union {
        uint32_t uuid[3];
        uint8_t unmap[3 * sizeof(uint32_t)]; /*!< Uncmaped form. */
    };
} luos_uuid_t;

// Handy struct
typedef struct __attribute__((__packed__))
{
    union {
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

// Imu report struct
typedef struct __attribute__((__packed__))
{
    union {
        struct __attribute__((__packed__))
        {
            unsigned short accell : 1;
            unsigned short gyro : 1;
            unsigned short quat : 1;
            unsigned short compass : 1;
            unsigned short euler : 1;
            unsigned short rot_mat : 1;
            unsigned short pedo : 1;
            unsigned short linear_accel : 1;
            unsigned short gravity_vector : 1;
            unsigned short heading : 1;
        };
        unsigned char unmap[2];
    };
} imu_report_t;

// Pid
typedef struct __attribute__((__packed__))
{
    union {
        struct __attribute__((__packed__))
        {
            float p;
            float i;
            float d;
        };
        unsigned char unmap[3 * sizeof(float)];
    };
} asserv_pid_t;

// Servo
typedef struct
{
    union {
        struct __attribute__((__packed__))
        {
            angular_position_t max_angle;
            float min_pulse_time;
            float max_pulse_time;
        };
        unsigned char unmap[3 * sizeof(float)];
    };
} servo_parameters_t;

// Motor
typedef struct __attribute__((__packed__))
{
    union {
        struct __attribute__((__packed__))
        {
            // target modes
            uint16_t mode_compliant : 1;
            uint16_t mode_ratio : 1;
            uint16_t mode_torque : 1;
            uint16_t mode_angular_speed : 1;
            uint16_t mode_angular_position : 1;
            uint16_t mode_linear_speed : 1;
            uint16_t mode_linear_position : 1;

            // report modes
            uint16_t angular_position : 1;
            uint16_t angular_speed : 1;
            uint16_t linear_position : 1;
            uint16_t linear_speed : 1;
            uint16_t current : 1;
        };
        uint8_t unmap[2];
    };
} motor_mode_t;

// Control modes
typedef enum
{
    PLAY,
    PAUSE,
    STOP
} control_type_t;

typedef struct __attribute__((__packed__))
{
    union {
        struct __attribute__((__packed__))
        {
            // control modes
            uint8_t mode_control : 2;
            uint8_t mode_rec : 1;
        };
        uint8_t unmap;
    };
} control_mode_t;

typedef struct __attribute__((__packed__))
{

    // targets
    motor_mode_t mode;
    angular_position_t target_angular_position;
    angular_speed_t target_angular_speed;
    ratio_t target_ratio;

    // limits
    angular_position_t limit_angular_position[2];
    ratio_t limit_ratio;
    current_t limit_current;

    // measures
    angular_position_t angular_position;
    angular_speed_t angular_speed;
    linear_position_t linear_position;
    linear_speed_t linear_speed;
    current_t current;

    //configs
    float motor_reduction;
    float resolution;
    linear_position_t wheel_diameter;
} motor_config_t;

#endif /*__ __L0_H */
