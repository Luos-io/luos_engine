/******************************************************************************
 * @file start controller
 * @brief application example a start controller
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "motor_copy.h"
#include "profile_servo_motor.h"
#include "od_ratio.h"
#include "product_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STARTUP_DELAY_MS       50
#define REFRESH_POSITION_MOTOR 10

#define DEFAULT_ANGULAR_SPEED 180 // 180°/s
/*******************************************************************************
 * Variables
 ******************************************************************************/
static service_t *app;
uint32_t position_refresh = 0;
uint8_t position          = NO_MOTOR;
uint16_t motor_table[3];
bool end_detection = false;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void MotorCopy_EventHandler(service_t *service, msg_t *msg);
static void Motor_init(uint16_t id);
static void motor_set(uint8_t motor_target, angular_position_t position);
static void sort_motors(void);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void MotorCopy_Init(void)
{
    revision_t revision = {.major = 0, .minor = 1, .build = 1};
    // Create App
    app = Luos_CreateService(MotorCopy_EventHandler, COPY_MOTOR, "motor_copy", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void MotorCopy_Loop(void)
{

    // Check if we have done the first init or if service Id have changed
    if (Luos_IsDetected())
    {
        if (end_detection)
        {
            search_result_t result;
            sort_motors();
            RTFilter_Type(RTFilter_Reset(&result), LEDSTRIP_POSITION_APP);
            if (result.result_nbr > 0)
            {
                msg_t msg;
                uint8_t ledstrip_id = result.result_table[0]->id;
                // Switch the LEDSTRIP_POSITION_APP to copy mode
                msg.header.target_mode = SERVICEIDACK;
                msg.header.target      = ledstrip_id;
                msg.header.cmd         = PARAMETERS;
                msg.header.size        = 1;
                msg.data[0]            = MOTOR_COPY_DISPLAY;
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
                RTFilter_Alias(RTFilter_Reset(&result), "dxl");
                if (result.result_nbr > 0)
                {
                    // Setup auto update each UPDATE_PERIOD_MS on dxl
                    // This value is resetted on all service at each detection
                    // It's important to setting it each time.
                    msg.header.target      = result.result_table[0]->id;
                    msg.header.target_mode = SERVICEIDACK;
                    time_luos_t time       = TimeOD_TimeFrom_ms(REFRESH_POSITION_MOTOR);
                    TimeOD_TimeToMsg(&time, &msg);
                    msg.header.cmd = UPDATE_PUB;
                    while (Luos_SendMsg(app, &msg) != SUCCEED)
                    {
                        Luos_Loop();
                    }
                    // Setup dxl
                    servo_motor_mode_t servo_mode = {
                        .mode_compliant = true,
                        // control POWER / ANGULAR POSITION
                        .mode_angular_speed    = false,
                        .mode_angular_position = true,
                        .angular_position      = true};

                    msg.header.target      = result.result_table[0]->id;
                    msg.header.cmd         = PARAMETERS;
                    msg.header.target_mode = SERVICEIDACK;
                    msg.header.size        = sizeof(servo_motor_mode_t);
                    memcpy(&msg.data, &servo_mode, sizeof(servo_motor_mode_t));
                    while (Luos_SendMsg(app, &msg) != SUCCEED)
                    {
                        Luos_Loop();
                    }
                }
                // Compute the position of the dxl
                msg.header.target      = ledstrip_id;
                msg.header.cmd         = SET_CMD;
                msg.header.target_mode = SERVICEIDACK;
                msg.header.size        = 1;
                msg.data[0]            = position;
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
                // find the other motors and configure them
                RTFilter_Alias(RTFilter_Reset(&result), "servo");
                for (uint8_t i = 0; i < result.result_nbr; i++)
                {
                    Motor_init(result.result_table[i]->id);
                }
                end_detection = false;
            }
        }
    }
}

void MotorCopy_EventHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == ANGULAR_POSITION)
    {
        search_result_t result;
        angular_position_t target;
        AngularOD_PositionFromMsg(&target, msg);
        RTFilter_Alias(RTFilter_Reset(&result), "servo");
        for (uint8_t i = 0; i < result.result_nbr; i++)
        {
            motor_set(result.result_table[i]->id, target);
        }
    }
    else if (msg->header.cmd == END_DETECTION)
    {
        end_detection = true;
    }
}

void Motor_init(uint16_t id)
{
    // send a command to the motor
    servo_motor_mode_t servo_mode = {
        .mode_compliant = false,
        // control POWER / ANGULAR POSITION
        .mode_angular_speed    = false,
        .mode_angular_position = true,
        .angular_position      = false};

    msg_t msg;
    msg.header.target      = id;
    msg.header.cmd         = PARAMETERS;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(servo_motor_mode_t);
    memcpy(&msg.data, &servo_mode, sizeof(servo_motor_mode_t));
    while (Luos_SendMsg(app, &msg) != SUCCEED)
    {
        Luos_Loop();
    }

    float resolution       = 12.0;
    msg.header.target      = id;
    msg.header.cmd         = RESOLUTION;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(float);
    memcpy(&msg.data, &resolution, sizeof(float));
    while (Luos_SendMsg(app, &msg) != SUCCEED)
    {
        Luos_Loop();
    }

    float reduction        = 74.83;
    msg.header.target      = id;
    msg.header.cmd         = REDUCTION;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(float);
    memcpy(&msg.data, &reduction, sizeof(float));
    while (Luos_SendMsg(app, &msg) != SUCCEED)
    {
        Luos_Loop();
    }

    asserv_pid_t pid_coef  = {.p = 28.0, .i = 0.1, .d = 100.0};
    msg.header.target      = id;
    msg.header.cmd         = PID;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(asserv_pid_t);
    memcpy(&msg.data, &pid_coef, sizeof(asserv_pid_t));
    while (Luos_SendMsg(app, &msg) != SUCCEED)
    {
        Luos_Loop();
    }
}

void motor_set(uint8_t motor_target, angular_position_t position)
{
    // send
    msg_t msg;
    msg.header.target      = motor_target;
    msg.header.cmd         = ANGULAR_POSITION;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(angular_position_t);
    memcpy(&msg.data, &position, sizeof(angular_position_t));
    while (Luos_SendMsg(app, &msg) == FAILED)
    {
        Luos_Loop();
    }
}

static void sort_motors(void)
{
    search_result_t result;
    // Parse routing table to find motors
    int motor_found = 0;
    position        = NO_MOTOR;
    RTFilter_Type(RTFilter_Reset(&result), SERVO_MOTOR_TYPE);
    for (motor_found = 0; motor_found < result.result_nbr; motor_found++)
    {
        motor_table[motor_found] = result.result_table[motor_found]->id;
    }
    motor_found = result.result_nbr;
    for (uint8_t i = 0; i < result.result_nbr; i++)
    {
        if (strstr(result.result_table[i]->alias, "dxl") != 0)
        {
            position = i + 1;
            break;
        }
    }
}