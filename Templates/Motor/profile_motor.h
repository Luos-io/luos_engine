/******************************************************************************
 * @file Profile motor
 * @brief motor object managing a true false API
 * WARING : This h file should be only included by profile_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_CLASS_MOTOR_H_
#define TEMPLATE_CLASS_MOTOR_H_

#include "struct_motor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void ProfileMotor_Handler(service_t *service, msg_t *msg, profile_motor_t *motor_profile);

#endif /* TEMPLATE_CLASS_MOTOR_H_ */
