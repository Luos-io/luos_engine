/******************************************************************************
 * @file Template servo motor
 * @brief servo motor container template
 * WARING : This h file should be only included by user code or profile_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_TEMPLATE_SERVO_MOTOR_H_
#define TEMPLATE_TEMPLATE_SERVO_MOTOR_H_

#include "luos.h"
#include "struct_servo_motor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * motor object structure
 */
typedef struct
{
    CONT_CB self;
    profile_servo_motor_t profile;
} template_servo_motor_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

container_t *TemplateServoMotor_CreateContainer(CONT_CB cont_cb, template_servo_motor_t *var, const char *alias, revision_t revision);

#endif /* TEMPLATE_TEMPLATE_SERVO_MOTOR_H_ */
