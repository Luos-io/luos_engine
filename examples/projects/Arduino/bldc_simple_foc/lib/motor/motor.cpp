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

#define TRAJECTORY_PERIOD_CALLBACK 1
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
angular_position_t last_position;
float motion_target_position = 0.0;

// measurement management (can be position or speed)
volatile float measurement_buf[BUFFER_SIZE];

// initialize a variable to save delay start
float tickstart = 0.0;

// velocity computation
float velocity_angle_prev     = 0.0;
float velocity_full_rotations = 0.0;
float vel                     = 0.0;
float velocity_tickstart      = 0.0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Motor_MsgHandler(service_t *service, const msg_t *msg);
void Motor_TrajectoryCallback(void);
float Motor_getVelocity(void);
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
    servo_motor.wheel_diameter  = LinearOD_PositionFrom_m(0.0);

    // Streaming control channels
    servo_motor.control.unmap   = 0; // PLAY and no REC
    servo_motor.sampling_period = TimeOD_TimeFrom_ms(SAMPLING_PERIOD_MS);
    servo_motor.trajectory      = Streaming_CreateChannel((float *)trajectory_buf, BUFFER_SIZE, sizeof(float));
    servo_motor.measurement     = Streaming_CreateChannel((float *)measurement_buf, BUFFER_SIZE, sizeof(float));

    // initialize tick variable
    tickstart = millis();

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
    motor.velocity_limit = AngularOD_SpeedTo_deg_s(servo_motor.limit_angular_speed[MAXI]);

    // init motor hardware
    motor.init();
    // align sensor and start FOC
    motor.initFOC();

    // velocity initialization
    velocity_angle_prev = sensor.getMechanicalAngle();
    velocity_tickstart  = millis();

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
        motor.move(AngularOD_PositionTo_rad(AngularOD_PositionFrom_deg(motion_target_position * servo_motor.motor_reduction)));
    }

    if (motor.controller == MotionControlType::velocity)
    {
        motor.move(AngularOD_PositionTo_rad(AngularOD_PositionFrom_deg(AngularOD_SpeedTo_deg_s(servo_motor.target_angular_speed) * servo_motor.motor_reduction)));
    }

    // update sensor position
    servo_motor.angular_position = AngularOD_PositionFrom_rad(sensor.getAngle() / servo_motor.motor_reduction);
    servo_motor.linear_position  = LinearOD_PositionFrom_m((AngularOD_PositionTo_deg(servo_motor.angular_position) * 3.141592653589793 * LinearOD_PositionTo_m(servo_motor.wheel_diameter)) / 360.0);
    servo_motor.angular_speed    = AngularOD_SpeedFrom_rad_s(Motor_getVelocity() / servo_motor.motor_reduction);
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
static void Motor_MsgHandler(service_t *service, const msg_t *msg)
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
        float increment = (fabs(AngularOD_SpeedTo_deg_s(servo_motor.target_angular_speed)) / 1000.0);

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

/******************************************************************************
 * @brief Compute motor velocity
 * @param void
 * @return velocity
 ******************************************************************************/
float Motor_getVelocity(void)
{
    if ((millis() - velocity_tickstart) > 100)
    {
        velocity_tickstart = millis();
        // get full rotations count
        float full_rotations = (sensor.getAngle() - sensor.getMechanicalAngle()) / _2PI;
        // velocity calculation
        vel = ((float)(full_rotations - velocity_full_rotations) * _2PI + (sensor.getMechanicalAngle() - velocity_angle_prev)) / ((float)0.1);
        // save variables for future pass
        velocity_angle_prev     = sensor.getMechanicalAngle();
        velocity_full_rotations = full_rotations;
    }

    return vel;
}
