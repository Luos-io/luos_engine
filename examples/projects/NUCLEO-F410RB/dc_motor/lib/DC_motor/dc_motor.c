/******************************************************************************
 * @file dc_motor
 * @brief driver example a simple dc_motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "dc_motor.h"
#include "dc_motor_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *MotorDCservice1;
service_t *MotorDCservice2;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void MotorDC_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void MotorDC_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    DRV_DCMotorInit();
    MotorDCservice1 = Luos_CreateService(MotorDC_MsgHandler, MOTOR_TYPE, "DC_motor1", revision);
    MotorDCservice2 = Luos_CreateService(MotorDC_MsgHandler, MOTOR_TYPE, "DC_motor2", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void MotorDC_Loop(void)
{
}
/******************************************************************************
 * @brief Msg manager call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void MotorDC_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == RATIO)
    {
        // set the motor position
        ratio_t power;
        RatioOD_RatioFromMsg(&power, msg);
        if ((int)MotorDCservice1 == (int)service)
        {
            DRV_DCMotorSetPower(MOTOR_DC_1, power);
        }
        else if ((int)MotorDCservice2 == (int)service)
        {
            DRV_DCMotorSetPower(MOTOR_DC_2, power);
        }
    }
}
