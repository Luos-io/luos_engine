/******************************************************************************
 * @file controller_motor
 * @brief driver example a simple controller motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "controller_motor.h"
#include "ll_motor.h"
#include "profile_servo_motor.h"

#include "stdbool.h"
#include "math.h"
#include <float.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ASSERV_PERIOD        1
#define SPEED_PERIOD         50
#define SPEED_NB_INTEGRATION SPEED_PERIOD / ASSERV_PERIOD
#define SAMPLING_PERIOD_MS   10.0
#define BUFFER_SIZE          1000

/*******************************************************************************
 * Variables
 ******************************************************************************/
profile_servo_motor_t servo_motor;

float errSpeedSum            = 0.0;
float motion_target_position = 0.0;

// Position Asserv things
volatile float errAngleSum  = 0.0;
volatile float lastErrAngle = 0.0;

// Speed Asserv things
volatile float lastErrSpeed = 0.0;

// Trajectory management (can be position or speed)
volatile float trajectory_buf[BUFFER_SIZE];
volatile angular_position_t last_position;

// measurement management (can be position or speed)
volatile float measurement_buf[BUFFER_SIZE];

// Speed calculation values
char speed_bootstrap = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void ControllerMotor_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void ControllerMotor_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    ll_motor_init();
    ll_motor_enable(MOTOR_DISABLE);

    // ************** motor driver configuration *****************
    motor_config_t ll_motor_parameters = {
        .motor_reduction = &servo_motor.motor_reduction,
        .resolution      = &servo_motor.resolution,
        .wheel_diameter  = &servo_motor.wheel_diameter,
        .limit_ratio     = &servo_motor.motor.limit_ratio,
        .limit_current   = &servo_motor.motor.limit_current};
    ll_motor_config(ll_motor_parameters);

    // ********** Control & Trajectory configuration *************
    // motor mode by default
    servo_motor.sampling_period            = TimeOD_TimeFrom_ms(SAMPLING_PERIOD_MS);
    servo_motor.mode.mode_compliant        = 1;
    servo_motor.mode.current               = 0;
    servo_motor.mode.mode_power            = 1;
    servo_motor.mode.mode_angular_position = 0;
    servo_motor.mode.mode_angular_speed    = 0;
    servo_motor.mode.mode_linear_position  = 0;
    servo_motor.mode.mode_linear_speed     = 0;
    servo_motor.mode.angular_position      = 1;
    servo_motor.mode.angular_speed         = 0;
    servo_motor.mode.linear_position       = 0;
    servo_motor.mode.linear_speed          = 0;

    // Position limits default values
    servo_motor.limit_angular_position[MINI] = AngularOD_PositionFrom_deg(-FLT_MAX);
    servo_motor.limit_angular_position[MAXI] = AngularOD_PositionFrom_deg(FLT_MAX);

    // Position PID default values
    servo_motor.position_pid.p = 4.0;
    servo_motor.position_pid.i = 0.02;
    servo_motor.position_pid.d = 100.0;

    // Speed PID default values
    servo_motor.speed_pid.p = 0.1;
    servo_motor.speed_pid.i = 0.1;
    servo_motor.speed_pid.d = 0.0;

    // Control mode default values
    servo_motor.control.unmap = 0; // PLAY and no REC

    // Init streaming channels
    servo_motor.trajectory  = Streaming_CreateChannel((float *)trajectory_buf, BUFFER_SIZE, sizeof(float));
    servo_motor.measurement = Streaming_CreateChannel((float *)measurement_buf, BUFFER_SIZE, sizeof(float));

    last_position = AngularOD_PositionFrom_deg(0.0);

    // ************** Service creation *****************
    ProfileServo_CreateService(&servo_motor, ControllerMotor_MsgHandler, "servo_motor", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void ControllerMotor_Loop(void)
{
    // Time management
    static uint32_t last_asserv_systick = 0;
    uint32_t timestamp                  = Luos_GetSystick();
    uint32_t deltatime                  = timestamp - last_asserv_systick;

    // Speed measurement
    static angular_position_t last_angular_positions[SPEED_NB_INTEGRATION];

    // ************* Values computation *************
    // angular_posistion => degree
    servo_motor.angular_position = AngularOD_PositionFrom_deg(ll_motor_GetAngularPosition() + AngularOD_PositionTo_deg(servo_motor.angular_position));
    // linear_distance => m
    servo_motor.linear_position = LinearOD_PositionFrom_m(ll_motor_GetLinearPosition(AngularOD_PositionTo_deg(servo_motor.angular_position)));
    // current => A
    servo_motor.motor.current = ElectricOD_CurrentFrom_A(ll_motor_GetCurrent());

    if (deltatime >= ASSERV_PERIOD)
    {
        last_asserv_systick = timestamp;
        // angular_speed => degree/seconds
        // add the position value into unfiltered speed measurement
        for (int nbr = 0; nbr < (SPEED_NB_INTEGRATION - 1); nbr++)
        {
            // Check if this is the first measurement. If it is init the table.
            if (!speed_bootstrap)
            {
                last_angular_positions[nbr] = servo_motor.angular_position;
            }
            else
            {
                last_angular_positions[nbr] = last_angular_positions[nbr + 1];
            }
        }
        speed_bootstrap                                  = 1;
        last_angular_positions[SPEED_NB_INTEGRATION - 1] = servo_motor.angular_position;
        servo_motor.angular_speed                        = AngularOD_SpeedFrom_deg_s((AngularOD_PositionTo_deg(last_angular_positions[SPEED_NB_INTEGRATION - 1]) - AngularOD_PositionTo_deg(last_angular_positions[0])) * 1000.0 / SPEED_PERIOD);
        // linear_speed => m/seconds
        servo_motor.linear_speed = LinearOD_SpeedFrom_m_s((AngularOD_SpeedTo_deg_s(servo_motor.angular_speed) / 360.0) * M_PI * LinearOD_PositionTo_m(servo_motor.wheel_diameter));
        // ************* Limit clamping *************
        if (motion_target_position < AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MINI]))
        {
            motion_target_position = AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MINI]);
        }
        if (motion_target_position > AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MAXI]))
        {
            motion_target_position = AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MAXI]);
        }

        if (servo_motor.mode.mode_compliant)
        {
            // Motor is compliant, only manage motor limits
            if (AngularOD_PositionTo_deg(servo_motor.angular_position) < AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MINI]))
            {
                // re-enable motor to avoid bypassing motors limits
                ll_motor_enable(MOTOR_ENABLE);
                ll_motor_Command(servo_motor.mode.mode_compliant, 100.0 * (AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MINI]) - AngularOD_PositionTo_deg(servo_motor.angular_position)));
            }
            else if (AngularOD_PositionTo_deg(servo_motor.angular_position) > AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MAXI]))
            {
                ll_motor_enable(MOTOR_ENABLE);
                ll_motor_Command(servo_motor.mode.mode_compliant, -100.0 * (AngularOD_PositionTo_deg(servo_motor.angular_position) - AngularOD_PositionTo_deg(servo_motor.limit_angular_position[MAXI])));
            }
            else
            {
                ll_motor_enable(MOTOR_DISABLE);
            }
        }
        else if (servo_motor.mode.mode_power)
        {
            ll_motor_Command(servo_motor.mode.mode_compliant, RatioOD_RatioTo_Percent(servo_motor.motor.power));
        }
        else
        {
            // ************* position asserv *************
            // Target Position is managed by the motion planning interrupt (systick interrupt)
            float errAngle   = 0.0;
            float dErrAngle  = 0.0;
            float anglePower = 0.0;
            if (servo_motor.mode.mode_angular_position || servo_motor.mode.mode_linear_position)
            {
                errAngle  = motion_target_position - AngularOD_PositionTo_deg(servo_motor.angular_position);
                dErrAngle = (errAngle - lastErrAngle) / deltatime;
                errAngleSum += (errAngle * (float)deltatime);
                // Integral clamping
                if (errAngleSum < -100.0)
                    errAngleSum = -100.0;
                if (errAngleSum > 100.0)
                    errAngleSum = 100;
                anglePower   = (errAngle * servo_motor.position_pid.p) + (errAngleSum * servo_motor.position_pid.i) + (dErrAngle * servo_motor.position_pid.d); // raw PID command
                lastErrAngle = errAngle;
            }
            // ************* speed asserv *************
            float errSpeed   = 0.0;
            float dErrSpeed  = 0.0;
            float speedPower = 0.0;
            if (servo_motor.mode.mode_angular_speed || servo_motor.mode.mode_linear_speed)
            {
                errSpeed  = AngularOD_SpeedTo_deg_s(servo_motor.target_angular_speed) - AngularOD_SpeedTo_deg_s(servo_motor.angular_speed);
                dErrSpeed = (errSpeed - lastErrSpeed) / deltatime;
                errSpeedSum += (errSpeed * (float)deltatime);
                if (errSpeedSum < -100.0)
                    errSpeedSum = -100.0;
                if (errSpeedSum > 100.0)
                    errSpeedSum = 100;
                speedPower   = ((errSpeed * servo_motor.speed_pid.p) + (errSpeedSum * servo_motor.speed_pid.i) + (dErrSpeed * servo_motor.speed_pid.d)); // raw PID command
                lastErrSpeed = errSpeed;
            }
            // ************* command merge *************
            if (!(servo_motor.mode.mode_angular_position || servo_motor.mode.mode_linear_position) && (servo_motor.mode.mode_angular_speed || servo_motor.mode.mode_linear_speed))
            {
                // Speed control only
                ll_motor_Command(servo_motor.mode.mode_compliant, speedPower);
            }
            else
            {
                // we use position control by default
                ll_motor_Command(servo_motor.mode.mode_compliant, anglePower);
            }
        }
    }
}
/******************************************************************************
 * @brief Msg manager call by luos when service created a msg receive
 * @param Service send msg
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void ControllerMotor_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        return;
    }
    if (msg->header.cmd == PARAMETERS)
    {
        ll_motor_enable(servo_motor.mode.mode_compliant == 0);
        if (servo_motor.mode.mode_compliant == 0)
        {
            Luos_SetIrqState(false);
            last_position = servo_motor.angular_position;
            errAngleSum   = 0.0;
            lastErrAngle  = 0.0;
            Luos_SetIrqState(true);
        }
        return;
    }
    if (msg->header.cmd == REINIT)
    {
        // reinit asserv calculation
        errAngleSum     = 0.0;
        lastErrAngle    = 0.0;
        last_position   = AngularOD_PositionFrom_deg(0.0);
        speed_bootstrap = 0;
        return;
    }
    if ((msg->header.cmd == ANGULAR_POSITION) || (msg->header.cmd == LINEAR_POSITION))
    {
        if ((servo_motor.mode.mode_angular_position | servo_motor.mode.mode_angular_position) && (msg->header.size == sizeof(angular_position_t)))
        {
            // set the motor target angular position
            Luos_SetIrqState(false);
            last_position = servo_motor.angular_position;
            Luos_SetIrqState(true);
        }
        return;
    }
    if ((msg->header.cmd == ANGULAR_SPEED) || (msg->header.cmd == LINEAR_SPEED))
    {
        // set the motor target angular position
        if ((servo_motor.mode.mode_angular_speed) | (servo_motor.mode.mode_linear_speed))
        {
            // reset the integral factor for speed
            errSpeedSum = 0.0;
        }
        return;
    }
}

/******************************************************************************
 * @brief Callback called each millisecond to handle trajectory
 * @param void
 * @return None
 ******************************************************************************/
void HAL_SYSTICK_Motor_Callback(void)
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
            servo_motor.target_angular_position = AngularOD_PositionFrom_deg((LinearOD_PositionTo_m(linear_position_tmp) * 360.0) / (3.141592653589793 * LinearOD_PositionTo_m(servo_motor.wheel_diameter)));
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
