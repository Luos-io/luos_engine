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
#define GEAR_RATE 10.0

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

// initialize simple FOC drivers
SPIClass SPI_FOC(&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);
MagneticSensorSPI sensor = MagneticSensorSPI(AS5047_SPI, 10);

BLDCMotor motor       = BLDCMotor(14);
BLDCDriver3PWM driver = BLDCDriver3PWM(9, 5, 6, 8);

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
    servo_motor.position_pid.p = 0.2f;
    servo_motor.position_pid.i = 20.0;
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

    // initialize sensor
    sensor.init(&SPI_FOC);

    pinPeripheral(11, PIO_SERCOM);
    pinPeripheral(12, PIO_SERCOM);
    pinPeripheral(13, PIO_SERCOM);

    // initialize driver
    driver.voltage_power_supply = 12;
    driver.init();

    // initialize motor
    motor.linkSensor(&sensor);
    motor.linkDriver(&driver);

    // choose FOC modulation (optional)
    motor.foc_modulation = FOCModulationType::SpaceVectorPWM;

    // set motion control loop to be used
    motor.controller = MotionControlType::angle;

    // velocity PI controller parameters
    motor.PID_velocity.P = servo_motor.position_pid.p;
    motor.PID_velocity.I = servo_motor.position_pid.i;
    motor.PID_velocity.D = servo_motor.position_pid.d;
    // maximal voltage to be set to the motor
    motor.voltage_limit = 2.0;

    // velocity low pass filtering time constant
    // the lower the less filtered
    motor.LPF_velocity.Tf = 0.01f;

    // angle P controller
    motor.P_angle.P = 20;
    // maximal velocity of the position control
    motor.velocity_limit = servo_motor.limit_angular_speed[MAXI];

    // init motor hardware
    motor.init();
    // align sensor and start FOC
    motor.initFOC();

    // initialize service
    revision_t revision = {1, 0, 0};
    ProfileServo_CreateService(&servo_motor, Motor_MsgHandler, "FOC_motor", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Motor_Loop(void)
{
    if ((servo_motor.mode.mode_angular_position == true) || (servo_motor.mode.mode_linear_position == true))
    {
        motor.controller = MotionControlType::angle;
    }

    if (servo_motor.mode.mode_angular_speed == true)
    {
        motor.controller = MotionControlType::velocity;
    }

    // main FOC algorithm function
    // the faster you run this function the better
    // Arduino UNO loop  ~1kHz
    // Bluepill loop ~10kHz
    motor.loopFOC();

    // Motion control function
    // velocity, position or voltage (defined in motor.controller)
    // this function can be run at much lower frequency than loopFOC() function
    // You can also use motor.move() and set the motor.target in the code
    if (motor.controller == MotionControlType::angle)
    {
        motor.move(AngularOD_PositionTo_rad(servo_motor.target_angular_position) * servo_motor.motor_reduction);
    }

    if (motor.controller == MotionControlType::velocity)
    {
        motor.move(AngularOD_PositionTo_rad(servo_motor.target_angular_speed) * servo_motor.motor_reduction);
    }

    // update sensor position
    servo_motor.angular_position = AngularOD_PositionFrom_rad(sensor.getAngle()) / servo_motor.motor_reduction;
    servo_motor.linear_position  = (servo_motor.angular_position * 3.141592653589793 * servo_motor.wheel_diameter) / 360.0;
    servo_motor.angular_speed    = AngularOD_PositionFrom_rad(sensor.getVelocity()) / servo_motor.motor_reduction;
    servo_motor.linear_speed     = (servo_motor.angular_speed * 3.141592653589793 * servo_motor.wheel_diameter) / 360.0;

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
