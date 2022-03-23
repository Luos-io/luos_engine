/******************************************************************************
 * @file motor
 * @brief FOC motor driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
using namespace std;
#include <Arduino.h>
#include "motor.h"
#include <SimpleFOC.h>
#include "SPI.h"
#include "wiring_private.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define GEAR_RATE 10.0

/*******************************************************************************
 * Variables
 ******************************************************************************/

StepperDriver4PWM driver = StepperDriver4PWM(5, 6, 9, 10);
StepperMotor motor       = StepperMotor(50);

// angular prosition command
float reduction       = GEAR_RATE;
float angle_command   = 0.0f;
float angle_read      = 0.0f;
asserv_pid_t coef_pid = {0.2f, 20, 0};
float speed_limit     = 20.0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Motor_MsgHandler(service_t *service, msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Motor_Init(void)
{

    driver.pwm_frequency = NOT_SET;

    driver.voltage_power_supply = 16;
    // limit the maximal dc voltage the driver can set
    // as a protection measure for the low-resistance motors
    // this value is fixed on startup
    driver.voltage_limit = 12;
    driver.init();
    // link the motor and the driver
    motor.linkDriver(&driver);

    // limiting motor movements
    // limit the voltage to be set to the motor
    // start very low for high resistance motors
    // currnet = resistance*voltage, so try to be well under 1Amp
    motor.voltage_limit = 6.0; // [V]
    // limit/set the velocity of the transition in between
    // target angles
    motor.velocity_limit = 20; // [rad/s] cca 50rpm
    // open loop control config
    motor.controller = MotionControlType::angle_openloop;

    // init motor hardware
    motor.init();

    // initialize service
    revision_t revision = {1, 0, 0};
    Luos_CreateService(Motor_MsgHandler, SERVO_MOTOR_TYPE, "FOC_motor", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Motor_Loop(void)
{

    // Motion control function
    // velocity, position or voltage (defined in motor.controller)
    // this function can be run at much lower frequency than loopFOC() function
    // You can also use motor.move() and set the motor.target in the code
    motor.move(AngularOD_PositionTo_rad(angle_command) * reduction);
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Motor_MsgHandler(service_t *service, msg_t *msg)
{
    switch (msg->header.cmd)
    {
        case ANGULAR_POSITION:
            AngularOD_PositionFromMsg((angular_position_t *)&angle_command, msg);
            break;

        case GET_CMD:
            // Report management
            msg_t pub_msg;
            pub_msg.header.target_mode = msg->header.target_mode;
            pub_msg.header.target      = msg->header.source;

            AngularOD_PositionToMsg((angular_position_t *)&angle_read, &pub_msg);
            Luos_SendMsg(service, &pub_msg);
            break;

        case REDUCTION:
            // set the motor reduction
            memcpy((void *)&reduction, msg->data, sizeof(float));
            break;

        case PID:
            // only position control is enable, we can save PID for positioning
            PidOD_PidFromMsg(&coef_pid, msg);
            break;

        case ANGULAR_SPEED_LIMIT:
            // set the motor angular speed limit
            AngularOD_SpeedFromMsg((angular_speed_t *)&speed_limit, msg);
            break;

        default:
            break;
    }
}
