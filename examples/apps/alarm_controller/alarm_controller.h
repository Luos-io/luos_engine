/******************************************************************************
 * @file alarm controler
 * @brief application example an alarm controler
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef ALARM_CONTROLLER_H
#define ALARM_CONTROLLER_H

#include "luos_engine.h"
#include "robus_network.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define UPDATE_PERIOD_MS 10
#define BLINK_NUMBER     3

#define LIGHT_INTENSITY      255
#define MOVEMENT_SENSIBILITY 20

// Imu report struct
typedef struct __attribute__((__packed__))
{
    union
    {
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

typedef enum
{
    ALARM_CONTROLLER_APP = LUOS_LAST_TYPE,
    START_CONTROLLER_APP
} alarm_apps_type_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void AlarmController_Init(void);
void AlarmController_Loop(void);

#endif /* ALARM_CONTROLLER_H */
