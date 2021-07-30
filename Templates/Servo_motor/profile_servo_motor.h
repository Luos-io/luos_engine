/******************************************************************************
 * @file Profile servo motor
 * @brief servo motor profile
 * WARING : This h file should be only included by profile_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_CLASS_SERVO_MOTOR_H_
#define TEMPLATE_CLASS_SERVO_MOTOR_H_

#include "struct_servo_motor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void ProfileServoMotor_Handler(service_t *service, msg_t *msg, profile_servo_motor_t *profile_servo_motor);

#endif /* TEMPLATE_CLASS_SERVO_MOTOR_H_ */
