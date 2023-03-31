/******************************************************************************
 * @file servo_drv
 * @brief This is a simple driver for a servo motor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/Arduino-Luos-Biometric-Security-System
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include <Servo.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "servo_drv.h"

#ifdef __cplusplus
}
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
Servo myservo;
servo_motor_t servo;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void ServoDrv_Init(void)
{
    myservo.attach(SERVO_PIN);

    servo_parameters_t param;

    param.max_angle      = AngularOD_PositionFrom_deg(180.0);
    param.max_pulse_time = 1.5 / 1000;
    param.min_pulse_time = 0.5 / 1000;

    servo.param = param;
    servo.angle = AngularOD_PositionFrom_deg(0.0);
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
uint8_t ServoDrv_SetPosition(angular_position_t angle)
{
    servo.angle = angle;
    // limit angle value
    if (AngularOD_PositionTo_deg(servo.angle) < 0.0)
    {
        servo.angle = AngularOD_PositionFrom_deg(0.0);
    }
    else if (AngularOD_PositionTo_deg(servo.angle) > AngularOD_PositionTo_deg(servo.param.max_angle))
    {
        servo.angle = servo.param.max_angle;
    }

    myservo.write(AngularOD_PositionTo_deg(servo.angle));

    return SUCCEED;
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
uint8_t ServoDrv_Parameter(servo_parameters_t param)
{
    servo.param.max_angle      = param.max_angle;
    servo.param.min_pulse_time = param.min_pulse_time;
    servo.param.max_pulse_time = param.max_pulse_time;
    return SUCCEED;
}