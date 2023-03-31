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

#define TRAJECTORY_PERIOD_CALLBACK 1
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
angular_position_t last_position;
float motion_target_position = 0.0;

// measurement management (can be position or speed)
volatile float measurement_buf[BUFFER_SIZE];

// initialize a variable to save delay start
float tickstart = 0.0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Motor_MsgHandler(service_t *service, msg_t *msg);
void Motor_TrajectoryCallback(void);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Motor_Init(void)
{
    last_position = AngularOD_PositionFrom_deg(0.0);
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
    servo_motor.angular_position = AngularOD_PositionFrom_deg(0.0);
    servo_motor.angular_speed    = AngularOD_SpeedFrom_deg_s(0.0);
    servo_motor.linear_position  = LinearOD_PositionFrom_m(0.0);
    servo_motor.linear_speed     = LinearOD_SpeedFrom_m_s(0.0);

    // target commands
    servo_motor.target_angular_position = AngularOD_PositionFrom_deg(0.0);
    servo_motor.target_angular_speed    = AngularOD_SpeedFrom_deg_s(0.0);

    // Position limits
    servo_motor.limit_angular_position[MINI] = AngularOD_PositionFrom_deg(-FLT_MAX);
    servo_motor.limit_angular_position[MAXI] = AngularOD_PositionFrom_deg(FLT_MAX);
    servo_motor.limit_angular_speed[MINI]    = AngularOD_SpeedFrom_deg_s(-FLT_MAX);
    servo_motor.limit_angular_speed[MAXI]    = AngularOD_SpeedFrom_deg_s(20.0);

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
    servo_motor.wheel_diameter  = LinearOD_PositionFrom_m(0.0);

    // Streaming control channels
    servo_motor.control.unmap   = 0; // PLAY and no REC
    servo_motor.sampling_period = TimeOD_TimeFrom_ms(SAMPLING_PERIOD_MS);
    servo_motor.trajectory      = Streaming_CreateChannel((float *)trajectory_buf, BUFFER_SIZE, sizeof(float));
    servo_motor.measurement     = Streaming_CreateChannel((float *)measurement_buf, BUFFER_SIZE, sizeof(float));

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
    motor.velocity_limit = AngularOD_SpeedTo_deg_s(servo_motor.limit_angular_speed[MAXI]); // [rad/s] cca 50rpm
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
        motor.move(AngularOD_SpeedTo_rad_s(servo_motor.target_angular_speed) * servo_motor.motor_reduction);
    }

    // measures
    servo_motor.angular_position = servo_motor.target_angular_position;
    servo_motor.linear_position  = LinearOD_PositionFrom_m((AngularOD_PositionTo_deg(servo_motor.angular_position) * 3.141592653589793 * LinearOD_PositionTo_m(servo_motor.wheel_diameter)) / 360.0);
    servo_motor.angular_speed    = servo_motor.target_angular_speed;
    servo_motor.linear_speed     = LinearOD_SpeedFrom_m_s((AngularOD_SpeedTo_deg_s(servo_motor.angular_speed) * 3.141592653589793 * LinearOD_PositionTo_m(servo_motor.wheel_diameter)) / 360.0);

    // call streaming handler each millisecond
    if ((millis() - tickstart) > TRAJECTORY_PERIOD_CALLBACK)
    {
        // reset tickstart
        tickstart = millis();
        // call trajectory handler
        Motor_TrajectoryCallback();
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Motor_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == PARAMETERS)
    {
        if (servo_motor.mode.mode_compliant == 0)
        {
            last_position = servo_motor.angular_position;
        }
    }
    if (msg->header.cmd == REINIT)
    {
        // reinit asserv calculation
        last_position = AngularOD_PositionFrom_deg(0.0);
    }

    // here we shunt the trajectory mode
    if ((msg->header.cmd == ANGULAR_POSITION) || (msg->header.cmd == LINEAR_POSITION))
    {
        if ((servo_motor.mode.mode_angular_position | servo_motor.mode.mode_angular_position) && (msg->header.size == sizeof(angular_position_t)))
        {
            // set the motor target angular position
            last_position = servo_motor.angular_position;
        }
    }
}

/******************************************************************************
 * @brief Callback called each millisecond to handle trajectory
 * @param void
 * @return None
 ******************************************************************************/
void Motor_TrajectoryCallback(void)
{
    // ************* motion planning *************
    // ****** recorder management *********
    static uint32_t last_rec_systick = 0;
    if (servo_motor.control.rec && ((Luos_GetSystick() - last_rec_systick) >= TimeOD_TimeTo_ms(servo_motor.sampling_period)))
    {
        // We have to save a sample of current position
        Streaming_PutSample(&servo_motor.measurement, (angular_position_t *)&servo_motor.angular_position, 1);
        last_rec_systick = Luos_GetSystick();
    }
    // ****** trajectory management *********
    static uint32_t last_systick = 0;
    if (servo_motor.control.flux == STOP)
    {
        Streaming_ResetChannel(&servo_motor.trajectory);
    }
    if ((Streaming_GetAvailableSampleNB(&servo_motor.trajectory) > 0) && ((Luos_GetSystick() - last_systick) >= TimeOD_TimeTo_ms(servo_motor.sampling_period)) && (servo_motor.control.flux == PLAY))
    {
        if (servo_motor.mode.mode_linear_position == 1)
        {
            linear_position_t linear_position_tmp;
            Streaming_GetSample(&servo_motor.trajectory, &linear_position_tmp, 1);
            servo_motor.target_angular_position = AngularOD_PositionFrom_deg(LinearOD_PositionTo_m(linear_position_tmp) * 360.0 / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor.wheel_diameter)));
        }
        else
        {
            Streaming_GetSample(&servo_motor.trajectory, (angular_position_t *)&servo_motor.target_angular_position, 1);
        }
        last_systick = Luos_GetSystick();
    }
    // ****** Linear interpolation *********
    if ((servo_motor.mode.mode_angular_position || servo_motor.mode.mode_linear_position)
        && (servo_motor.mode.mode_angular_speed || servo_motor.mode.mode_linear_speed))
    {
        // speed control and position control are enabled
        // we need to move target position following target speed
        float increment = fabs(AngularOD_SpeedTo_deg_s(servo_motor.target_angular_speed) / 1000.0);

        // shunt the trajectory mode
        if (fabs(AngularOD_PositionTo_deg(servo_motor.target_angular_position) - AngularOD_PositionTo_deg(last_position)) <= increment)
        {
            // target_position is the final target position
            motion_target_position = AngularOD_PositionTo_deg(servo_motor.target_angular_position);
        }
        else if ((AngularOD_PositionTo_deg(servo_motor.target_angular_position) - AngularOD_PositionTo_deg(servo_motor.angular_position)) < 0.0)
        {
            motion_target_position = AngularOD_PositionTo_deg(last_position) - increment;
        }
        else
        {
            motion_target_position = AngularOD_PositionTo_deg(last_position) + increment;
        }
    }
    else
    {
        // target_position is the final target position
        motion_target_position = AngularOD_PositionTo_deg(servo_motor.target_angular_position);
    }
    last_position = AngularOD_PositionFrom_deg(motion_target_position);
}