/******************************************************************************
 * @file servo
 * @brief driver example a simple servo motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "servo_drv.h"

#include "servo.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static service_t *service_servo[SERVONUMBER];
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Servo_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Servo_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    Servo_DRVInit();

    service_servo[0] = Luos_CreateService(Servo_MsgHandler, ANGLE_TYPE, "servo1", revision);
    service_servo[1] = Luos_CreateService(Servo_MsgHandler, ANGLE_TYPE, "servo2", revision);
    service_servo[2] = Luos_CreateService(Servo_MsgHandler, ANGLE_TYPE, "servo3", revision);
    service_servo[3] = Luos_CreateService(Servo_MsgHandler, ANGLE_TYPE, "servo4", revision);
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
    uint8_t i = 0;
    servo_t servo;
    if (msg->header.cmd == ANGULAR_POSITION)
    {
        // set the motor position
        AngularOD_PositionFromMsg((angular_position_t *)&servo.angle, msg);
        for (i = 0; i <= SERVONUMBER; i++)
        {
            if ((int)service == (int)service_servo[i])
            {
                Servo_DRVSetPosition(servo.angle, i);
                break;
            }
        }
    }
    else if (msg->header.cmd == PARAMETERS)
    {
        // set the servo parameters
        memcpy((void *)servo.param.unmap, msg->data, sizeof(servo_parameters_t));
        for (i = 0; i <= SERVONUMBER; i++)
        {
            if ((int)service == (int)service_servo[i])
            {
                Servo_DRVParameter(servo.param, i);
                break;
            }
        }
    }
}
