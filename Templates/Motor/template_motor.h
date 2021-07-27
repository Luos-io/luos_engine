/******************************************************************************
 * @file Template motor
 * @brief motor service template
 * WARING : This h file should be only included by user code or profile_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_TEMPLATE_MOTOR_H_
#define TEMPLATE_TEMPLATE_MOTOR_H_

#include "luos.h"
#include "struct_motor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * motor object structure
 */
typedef struct
{
    SERVICE_CB self;
    profile_motor_t profile;
} template_motor_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

service_t *TemplateMotor_CreateService(SERVICE_CB service_cb, template_motor_t *var, const char *alias, revision_t revision);

#endif /* TEMPLATE_TEMPLATE_MOTOR_H_ */
