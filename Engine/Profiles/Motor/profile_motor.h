/******************************************************************************
 * @file Profile motor
 * @brief motor object managing a true false API
 * WARING : This h file should be only included by profile_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PROFILE_MOTOR_H_
#define PROFILE_MOTOR_H_

#include "profile_core.h"

#define MINI 0
#define MAXI 1

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * motor mode structure
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            // drive modes
            uint8_t mode_compliant : 1;

            // measures modes
            uint8_t temperature : 1;
            uint8_t current : 1;
        };
        uint8_t unmap[1];
    };
} motor_mode_t;
/*
 * motor data structure
 */
typedef struct
{
    // drive
    motor_mode_t mode;
    ratio_t power;

    // limits
    ratio_t limit_ratio;
    current_t limit_current;
    temperature_t limit_temperature;

    // measures
    temperature_t temperature;
    current_t current;
} profile_motor_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void ProfileMotor_link(uint8_t, profile_motor_t *);
service_t *ProfileMotor_CreateService(profile_motor_t *, SERVICE_CB, const char *, revision_t);

#endif /* PROFILE_MOTOR_H_ */
