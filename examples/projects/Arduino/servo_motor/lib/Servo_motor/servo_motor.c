/******************************************************************************
 * @file servo_motor
 * @brief This is a simple driver for a servo motor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/Arduino-Luos-Biometric-Security-System
 * @version 0.0.0
 ******************************************************************************/
#include "servo_motor.h"
#include "servo_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static service_t *service_servo;

/*******************************************************************************
 * Functions
 ******************************************************************************/
static void Servo_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Servo_Init(void)
{
    ServoDrv_Init();

    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    service_servo = Luos_CreateService(Servo_MsgHandler, SERVO_MOTOR_TYPE, "servo", revision);
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Servo_Loop(void)
{
}

/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Servo_MsgHandler(service_t *service, const msg_t *msg)
{
    servo_motor_t servo;
    if (msg->header.cmd == ANGULAR_POSITION)
    {
        // set the motor position
        AngularOD_PositionFromMsg((angular_position_t *)&servo.angle, msg);
        ServoDrv_SetPosition(servo.angle);
    }
    else if (msg->header.cmd == PARAMETERS)
    {
        // set the servo parameters
        memcpy((void *)servo.param.unmap, msg->data, sizeof(servo_parameters_t));
        ServoDrv_Parameter(servo.param);
    }
}
