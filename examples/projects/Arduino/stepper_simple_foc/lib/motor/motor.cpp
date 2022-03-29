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
#include <float.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define GEAR_RATE 1.0

#define ASSERV_PERIOD        1
#define SPEED_PERIOD         50
#define SPEED_NB_INTEGRATION SPEED_PERIOD / ASSERV_PERIOD
#define SAMPLING_PERIOD_MS   10.0
#define BUFFER_SIZE          1000

/*******************************************************************************
 * Variables
 ******************************************************************************/
// create a servo motor profile structure
profile_servo_motor_t servo_motor;

// Simple FOC driver and motor declaration
StepperDriver4PWM driver = StepperDriver4PWM(5, 6, 9, 10);
StepperMotor motor       = StepperMotor(50);

// Trajectory management (can be position or speed)
volatile float trajectory_buf[BUFFER_SIZE];
volatile angular_position_t last_position = 0.0;

// measurement management (can be position or speed)
volatile float measurement_buf[BUFFER_SIZE];

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
    // servo motor mode initialization
    servo_motor.mode.mode_compliant        = false;
    servo_motor.mode.current               = false;
    servo_motor.mode.mode_power            = false;
    servo_motor.mode.mode_angular_position = true;
    servo_motor.mode.mode_angular_speed    = false;
    servo_motor.mode.mode_linear_position  = false;
    servo_motor.mode.mode_linear_speed     = false;
    servo_motor.mode.angular_position      = true;
    servo_motor.mode.angular_speed         = false;
    servo_motor.mode.linear_position       = false;
    servo_motor.mode.linear_speed          = false;

    // measures
    servo_motor.angular_position = 0.0;
    servo_motor.angular_speed    = 0.0;
    servo_motor.linear_position  = 0.0;
    servo_motor.linear_speed     = 0.0;

    // target commands
    servo_motor.target_angular_position = 0.0;
    servo_motor.target_angular_speed    = 0.0;

    // Position limits
    servo_motor.limit_angular_position[MINI] = -FLT_MAX;
    servo_motor.limit_angular_position[MAXI] = FLT_MAX;
    servo_motor.limit_angular_speed[MINI]    = -FLT_MAX;
    servo_motor.limit_angular_speed[MAXI]    = 20.0;

    // Position PID default values
    servo_motor.position_pid.p = 0.0;
    servo_motor.position_pid.i = 0.0;
    servo_motor.position_pid.d = 0.0;

    // Speed PID default values
    servo_motor.speed_pid.p = 0.0;
    servo_motor.speed_pid.i = 0.0;
    servo_motor.speed_pid.d = 0.0;

    // motor parameters
    servo_motor.motor_reduction = GEAR_RATE;
    servo_motor.resolution      = 0.0;
    servo_motor.wheel_diameter  = 0.0;

    // Streaming control channels
    servo_motor.control.unmap   = 0; // PLAY and no REC
    servo_motor.sampling_period = TimeOD_TimeFrom_ms(SAMPLING_PERIOD_MS);
    servo_motor.trajectory      = Stream_CreateStreamingChannel((float *)trajectory_buf, BUFFER_SIZE, sizeof(float));
    servo_motor.measurement     = Stream_CreateStreamingChannel((float *)measurement_buf, BUFFER_SIZE, sizeof(float));

    // simple foc pwm driver initialization
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
    motor.velocity_limit = servo_motor.limit_angular_speed[MAXI]; // [rad/s] cca 50rpm
    // open loop control config
    motor.controller = MotionControlType::angle_openloop;

    // init motor hardware
    motor.init();

    // initialize service
    revision_t revision = {1, 0, 0};
    ProfileServo_CreateService(&servo_motor, Motor_MsgHandler, "Stepper_FOC", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Motor_Loop(void)
{
    // Set the motor mode
    // if linear mode, set angular mode, linear command will be converted
    // to an angular command by the motor profile
    if ((servo_motor.mode.mode_angular_position == true) || (servo_motor.mode.mode_linear_position == true))
    {
        motor.controller = MotionControlType::angle_openloop;
    }

    if (servo_motor.mode.mode_angular_speed == true)
    {
        motor.controller = MotionControlType::velocity_openloop;
    }

    // Motion control function
    // velocity, position or voltage (defined in motor.controller)
    // this function can be run at much lower frequency than loopFOC() function
    // You can also use motor.move() and set the motor.target in the code
    if (motor.controller == MotionControlType::angle_openloop)
    {
        motor.move(AngularOD_PositionTo_rad(servo_motor.target_angular_position) * servo_motor.motor_reduction);
    }

    if (motor.controller == MotionControlType::velocity_openloop)
    {
        motor.move(AngularOD_PositionTo_rad(servo_motor.target_angular_speed) * servo_motor.motor_reduction);
    }

    // measures
    servo_motor.angular_position = servo_motor.target_angular_position;
    servo_motor.linear_position  = (servo_motor.angular_position * 3.141592653589793 * servo_motor.wheel_diameter) / 360.0;

    // TODO linear and angular speed measures
    // use a timer to compute speed based on position ?

    // TODO streaming
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Motor_MsgHandler(service_t *service, msg_t *msg)
{
}
