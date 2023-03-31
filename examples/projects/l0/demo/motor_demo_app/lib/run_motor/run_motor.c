/******************************************************************************
 * @file start controller
 * @brief application example a start controller
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "run_motor.h"
#include "profile_servo_motor.h"
#include "od_ratio.h"
#include "product_config.h"

#include "math.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STARTUP_DELAY_MS 100

#define REFRESH_POSITION_MOTOR  100
#define REFRESH_DIRECTION_MOTOR 100 // in milliseconds

#define MAX_ANGLE 90

#define NB_POINT_IN_TRAJECTORY 200
#define TRAJECTORY_PERIOD      2.0 // in seconds
#define SAMPLING_PERIOD        (float)(TRAJECTORY_PERIOD / NB_POINT_IN_TRAJECTORY)
/*******************************************************************************
 * Variables
 ******************************************************************************/
static service_t *app;
static uint8_t current_motor_target = NO_MOTOR;
static uint8_t next_motor_target    = NO_MOTOR;
uint32_t command_refresh            = 0;
uint16_t led_app_id                 = 0;
uint16_t motor_table[3];
int motor_found = 0;

// Trajectory management
uint32_t trajectory_refresh = 0;
float trajectory[NB_POINT_IN_TRAJECTORY];

/*******************************************************************************
 * Function
 ******************************************************************************/
static void RunMotor_EventHandler(service_t *service, msg_t *msg);
static void motor_init(uint8_t motor_target);
static void motor_SendTrajectory(uint8_t motor_target);
static void motor_stream(uint8_t motor_target, control_type_t control);
static void sort_motors(void);
static void compute_trajectory(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void RunMotor_Init(void)
{
    revision_t revision = {.major = 0, .minor = 1, .build = 1};
    // Create App
    app = Luos_CreateService(RunMotor_EventHandler, RUN_MOTOR, "run_motor", revision);

    command_refresh    = Luos_GetSystick();
    trajectory_refresh = Luos_GetSystick();

    compute_trajectory();
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void RunMotor_Loop(void)
{
    // Check if we the node is detected
    if (Luos_IsDetected())
    {
        // check if we need to change the selected motor
        // if new target has been received, update selected motor
        // and reset unselected motor to their default position
        if (next_motor_target != current_motor_target)
        {
            // send play command to selected motor
            if (current_motor_target != NO_MOTOR)
            {
                motor_stream(motor_table[current_motor_target - 1], PAUSE);
            }
            if (next_motor_target != NO_MOTOR)
            {
                motor_stream(motor_table[next_motor_target - 1], PLAY);
            }
            current_motor_target = next_motor_target;
        }

        // send trajectory data at a fixed period
        if ((Luos_GetSystick() - trajectory_refresh > (uint32_t)TRAJECTORY_PERIOD * 1000))
        {
            // reset command_refresh
            trajectory_refresh = Luos_GetSystick();

            // send trajectory to the current motor
            for (int i = 0; i < motor_found; i++)
            {
                motor_SendTrajectory(motor_table[i]);
            }
        }
    }
}

void RunMotor_EventHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == SET_CMD)
    {
        next_motor_target = msg->data[0];
    }
    else if (msg->header.cmd == END_DETECTION)
    {
        search_result_t result;
        // A detection just finished
        // Make services configurations
        // ask for the motor's position to move
        RTFilter_Type(RTFilter_Reset(&result), LEDSTRIP_POSITION_APP);
        if (result.result_nbr > 0)
        {
            // Setup auto update each UPDATE_PERIOD_MS on imu
            // This value is resetted on all service at each detection
            // It's important to setting it each time.
            msg_t update_msg;
            update_msg.header.target      = result.result_table[0]->id;
            update_msg.header.target_mode = SERVICEIDACK;
            time_luos_t time              = TimeOD_TimeFrom_ms(REFRESH_POSITION_MOTOR);
            TimeOD_TimeToMsg(&time, &update_msg);
            update_msg.header.cmd = UPDATE_PUB;
            while (Luos_SendMsg(app, &update_msg) != SUCCEED)
            {
                Luos_Loop();
            }
        }
        // init motor on the first run
        sort_motors();
        for (int i = 0; i < motor_found; i++)
        {
            motor_init(motor_table[i]);
        }
        next_motor_target = NO_MOTOR;
    }
}

void motor_init(uint8_t motor_target)
{
    msg_t msg;
    // Do not send motor configuration to dxl
    search_result_t result;
    RTFilter_ID(RTFilter_Reset(&result), motor_target);
    if (strstr(result.result_table[0]->alias, "dxl") == 0)
    {
        // Send sensor resolution
        float resolution       = 12.0;
        msg.header.target      = motor_target;
        msg.header.cmd         = RESOLUTION;
        msg.header.target_mode = SERVICEIDACK;
        msg.header.size        = sizeof(float);
        memcpy(&msg.data, &resolution, sizeof(float));
        while (Luos_SendMsg(app, &msg) != SUCCEED)
        {
            Luos_Loop();
        }
        // Send reduction ratio resolution
        float reduction        = 74.83;
        msg.header.target      = motor_target;
        msg.header.cmd         = REDUCTION;
        msg.header.target_mode = SERVICEIDACK;
        msg.header.size        = sizeof(float);
        memcpy(&msg.data, &reduction, sizeof(float));
        while (Luos_SendMsg(app, &msg) != SUCCEED)
        {
            Luos_Loop();
        }
        // Send PID
        asserv_pid_t pid_coef  = {.p = 28.0, .i = 0.1, .d = 100.0};
        msg.header.target      = motor_target;
        msg.header.cmd         = PID;
        msg.header.target_mode = SERVICEIDACK;
        msg.header.size        = sizeof(asserv_pid_t);
        memcpy(&msg.data, &pid_coef, sizeof(asserv_pid_t));
        while (Luos_SendMsg(app, &msg) != SUCCEED)
        {
            Luos_Loop();
        }
    }
    // Send parameters to the motor
    servo_motor_mode_t servo_mode = {
        .mode_compliant        = false,
        .mode_angular_speed    = false,
        .mode_angular_position = true,
        .angular_position      = true};

    msg.header.target      = motor_target;
    msg.header.cmd         = PARAMETERS;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(servo_motor_mode_t);
    memcpy(&msg.data, &servo_mode, sizeof(servo_motor_mode_t));
    while (Luos_SendMsg(app, &msg) != SUCCEED)
    {
        Luos_Loop();
    }
    // Send sampling frequency
    float sampling_freq    = SAMPLING_PERIOD;
    msg.header.target      = motor_target;
    msg.header.cmd         = TIME;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(float);
    memcpy(&msg.data, &sampling_freq, sizeof(float));
    while (Luos_SendMsg(app, &msg) != SUCCEED)
    {
        Luos_Loop();
    }

    // stop streaming
    motor_stream(motor_target, PAUSE);

    // send trajectory data
    motor_SendTrajectory(motor_target);
}

void motor_SendTrajectory(uint8_t motor_target)
{
    if (motor_target != 0)
    {
        msg_t msg;
        // send data
        msg.header.target      = motor_target;
        msg.header.cmd         = ANGULAR_POSITION;
        msg.header.target_mode = SERVICEIDACK;
        msg.header.size        = NB_POINT_IN_TRAJECTORY * sizeof(angular_position_t);
        Luos_SendData(app, &msg, trajectory, NB_POINT_IN_TRAJECTORY * sizeof(angular_position_t));
    }
}

void motor_stream(uint8_t motor_target, control_type_t control_type)
{
    msg_t msg;
    // stop streaming
    control_t control      = {.flux = control_type};
    msg.header.target      = motor_target;
    msg.header.cmd         = CONTROL;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = NB_POINT_IN_TRAJECTORY * sizeof(angular_position_t);
    ControlOD_ControlToMsg(&control, &msg);
    while (Luos_SendMsg(app, &msg) == FAILED)
    {
        Luos_Loop();
    }
}

static void sort_motors(void)
{
    search_result_t result;
    // Parse routing table to find motors
    RTFilter_Type(RTFilter_Reset(&result), SERVO_MOTOR_TYPE);
    for (uint8_t i = 0; i < result.result_nbr; i++)
    {
        motor_table[i] = result.result_table[i]->id;
    }
    motor_found = result.result_nbr;
}

void compute_trajectory(void)
{
    uint32_t index  = 0;
    float angle_deg = 0;
    float quantum   = 360.0 / NB_POINT_IN_TRAJECTORY;

    for (index = 0; index < NB_POINT_IN_TRAJECTORY; index++)
    {
        // compute linear trajectory
        angle_deg = index * quantum;
        // compute sinus trajectory
        trajectory[index] = sin(M_PI / 180 * angle_deg) * MAX_ANGLE;
    }
}