/******************************************************************************
 * @file servo_drv
 * @brief This is a simple driver for a servo motor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/Arduino-Luos-Biometric-Security-System
 * @version 0.0.0
 ******************************************************************************/
#ifndef SERVO_DRV_H
#define SERVO_DRV_H

#include "luos_engine.h"
#include "robus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
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

typedef struct
{
    angular_position_t angle;
    servo_parameters_t param;
} servo_motor_t;

#define SERVO_PIN 15

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
void ServoDrv_Init(void);

uint8_t ServoDrv_SetPosition(angular_position_t angle);
uint8_t ServoDrv_Parameter(servo_parameters_t param);

#endif /* SERVO_DRV_H*/
