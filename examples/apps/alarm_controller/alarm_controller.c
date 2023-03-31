/******************************************************************************
 * @file alarm controler
 * @brief application example an alarm controler
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "alarm_controller.h"
#include "main.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *app;
volatile control_t control_app;
uint8_t blink_state   = 0;
uint8_t end_detection = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void AlarmController_MsgHandler(service_t *service, msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void AlarmController_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Create App
    app                 = Luos_CreateService(AlarmController_MsgHandler, ALARM_CONTROLLER_APP, "alarm_control", revision);
    uint32_t init_timer = Luos_GetSystick();
    while (Luos_GetSystick() - init_timer < INIT_TIME)
        ;
    Luos_Detect(app);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void AlarmController_Loop(void)
{
    static uint8_t blink       = 0;
    static uint8_t blink_nb    = BLINK_NUMBER * 2;
    static uint32_t last_blink = 0;
    search_result_t result;

    // ********** hot plug management ************
    // Check if we have done the first init or if service Id have changed
    if (Luos_IsDetected())
    {
        if (end_detection)
        {
            // By default this app running
            control_app.flux = PLAY;
            // Make services configurations
            // try to find a Fader app and set light transition time just to be fancy
            // we will use the first that we find
            RTFilter_Type(RTFilter_Reset(&result), COLOR_TYPE);
            if (result.result_nbr > 0)
            {
                msg_t msg;
                msg.header.target      = result.result_table[0]->id;
                msg.header.target_mode = SERVICEIDACK;
                time_luos_t time       = TimeOD_TimeFrom_s(0.5f);
                TimeOD_TimeToMsg(&time, &msg);
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
            }
            // try to find an IMU and set parameters to disable quaternion and send back Gyro acceleration and euler.
            imu_report_t report;
            report.gyro  = 1;
            report.euler = 1;
            report.quat  = 0;

            RTFilter_Type(RTFilter_Reset(&result), IMU_TYPE);
            if (result.result_nbr > 0)
            {
                msg_t msg;
                msg.header.cmd         = PARAMETERS;
                msg.header.size        = sizeof(imu_report_t);
                msg.header.target      = result.result_table[0]->id;
                msg.header.target_mode = SERVICEIDACK;
                memcpy(msg.data, &report, sizeof(imu_report_t));
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }

                // Setup auto update each UPDATE_PERIOD_MS on imu
                // This value is resetted on all service at each detection
                // It's important to setting it each time.
                time_luos_t time = TimeOD_TimeFrom_ms(UPDATE_PERIOD_MS);
                TimeOD_TimeToMsg(&time, &msg);
                msg.header.cmd = UPDATE_PUB;
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
            }
            end_detection = 0;
            return;
        }
    }
    else
    {
        return;
    }
    // ********** non blocking blink ************
    if (control_app.flux == PLAY)
    {
        if (blink_state)
        {
            blink_state = 0;
            blink_nb    = 0;
            blink       = 0;
            // try to reach a buzzer and drive the first found if there is
            RTFilter_Alias(RTFilter_Reset(&result), "buzzer_mod");
            if (result.result_nbr > 0)
            {
                msg_t msg;
                msg.header.target      = result.result_table[0]->id;
                msg.header.target_mode = SERVICEIDACK;
                msg.header.cmd         = IO_STATE;
                msg.header.size        = 1;
                msg.data[0]            = 0;
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
            }
        }
        if (blink_nb < (BLINK_NUMBER * 2))
        {
            if ((Luos_GetSystick() - last_blink) >= 500)
            {
                blink_nb++;
                RTFilter_Type(RTFilter_Reset(&result), COLOR_TYPE);
                if (result.result_nbr > 0)
                {
                    // we get a led alarm, set color
                    color_t color;
                    color.r = 0;
                    color.g = 0;
                    color.b = 0;
                    if (!blink)
                    {
                        // turn led red
                        color.r = LIGHT_INTENSITY;
                    }
                    msg_t msg;
                    msg.header.target      = result.result_table[0]->id;
                    msg.header.target_mode = SERVICEIDACK;
                    IlluminanceOD_ColorToMsg(&color, &msg);
                    while (Luos_SendMsg(app, &msg) != SUCCEED)
                    {
                        Luos_Loop();
                    }
                }
                RTFilter_Alias(RTFilter_Reset(&result), "horn");
                if (result.result_nbr > 0)
                {
                    // we get a horn
                    uint8_t horn = 0;
                    if (!blink)
                    {
                        // turn the horn on
                        horn = 1;
                    }
                    msg_t msg;
                    msg.header.target      = result.result_table[0]->id;
                    msg.header.target_mode = SERVICEIDACK;
                    msg.header.size        = sizeof(uint8_t);
                    msg.header.cmd         = IO_STATE;
                    msg.data[0]            = horn;
                    while (Luos_SendMsg(app, &msg) != SUCCEED)
                    {
                        Luos_Loop();
                    }
                }
                blink      = (!blink);
                last_blink = Luos_GetSystick();
            }
        }
    }
    else
    {
        if (blink_nb != BLINK_NUMBER * 2)
        {
            // reset alarm state
            blink_nb    = BLINK_NUMBER * 2;
            blink_state = 0;
            blink       = 0;
        }
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void AlarmController_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == GYRO_3D)
    {
        // this is imu informations
        if (control_app.flux == PLAY)
        {
            float value[3];
            memcpy(value, msg->data, msg->header.size);
            if ((value[0] > MOVEMENT_SENSIBILITY) || (value[1] > MOVEMENT_SENSIBILITY) || (value[2] > MOVEMENT_SENSIBILITY))
            {
                blink_state = 1;
            }
        }
        return;
    }
    if (msg->header.cmd == CONTROL)
    {
        ControlOD_ControlFromMsg((control_t *)&control_app, msg);
        return;
    }
    if (msg->header.cmd == END_DETECTION)
    {
        end_detection = 1;
    }
}
