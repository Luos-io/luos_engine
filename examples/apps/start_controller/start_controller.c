/******************************************************************************
 * @file start controller
 * @brief application example a start controller
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "start_controller.h"
#include "main.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LIGHT_INTENSITY  255
#define UPDATE_PERIOD_MS 10

typedef enum
{
    ALARM_CONTROLLER_APP = LUOS_LAST_TYPE,
    START_CONTROLLER_APP
} alarm_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *app;
volatile control_t control_app;
uint8_t lock           = 1;
uint8_t last_btn_state = 0;
uint8_t state_switch   = 0;
uint8_t end_detection  = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void StartController_MsgHandler(service_t *service, msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void StartController_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Create App
    app                 = Luos_CreateService(StartController_MsgHandler, START_CONTROLLER_APP, "start_control", revision);
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
void StartController_Loop(void)
{
    static uint32_t switch_date    = 0;
    static uint8_t animation_state = 0;
    search_result_t result;
    // ********** hot plug management ************
    // Check if we have done the first init or if service Id have changed
    if (Luos_IsDetected())
    {
        // Make services configurations
        if (end_detection)
        {
            // By default this app running
            control_app.flux = PLAY;
            RTFilter_Alias(RTFilter_Reset(&result), "lock");
            if (result.result_nbr > 0)
            {

                msg_t msg;
                msg.header.target      = result.result_table[0]->id;
                msg.header.target_mode = SERVICEIDACK;
                // Setup auto update each UPDATE_PERIOD_MS on button
                // This value is resetted on all service at each detection
                // It's important to setting it each time.
                time_luos_t time = TimeOD_TimeFrom_ms(UPDATE_PERIOD_MS);
                TimeOD_TimeToMsg(&time, &msg);
                msg.header.cmd = UPDATE_PUB;
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
                end_detection = 0;
                return;
            }
        }
    }
    else
    {
        return;
    }
    // ********** non blocking button management ************
    if (state_switch & (control_app.flux == PLAY) & (animation_state == 0))
    {
        msg_t msg;
        msg.header.target_mode = SERVICEIDACK;
        // Share the lock state with the alarm_control app
        RTFilter_Alias(RTFilter_Reset(&result), "alarm_control");
        if (result.result_nbr > 0)
        {
            // we have an alarm_controller App control it
            control_t alarm_control;
            if (lock)
            {
                // Bike is locked, alarm need to run.
                alarm_control.flux = PLAY;
            }
            else
            {
                // Bike is unlocked alarm should be sutted down.
                alarm_control.flux = STOP;
            }
            // send message
            msg.header.target = result.result_table[0]->id;
            ControlOD_ControlToMsg(&alarm_control, &msg);
            while (Luos_SendMsg(app, &msg) != SUCCEED)
            {
                Luos_Loop();
            }
        }
        // The button state switch, change the led consequently
        state_switch = 0;
        RTFilter_Type(RTFilter_Reset(&result), COLOR_TYPE);
        if (result.result_nbr > 0)
        {
            // we have an alarm, we can set its color
            color_t color;
            color.r = 0;
            color.g = 0;
            color.b = 0;
            if (!lock)
            {
                color.g = LIGHT_INTENSITY;
            }
            else
            {
                color.b = LIGHT_INTENSITY;
            }
            msg.header.target = result.result_table[0]->id;
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
            msg.header.target = result.result_table[0]->id;
            msg.header.size   = sizeof(uint8_t);
            msg.header.cmd    = IO_STATE;
            // turn the horn on/off
            msg.data[0] = 1;
            while (Luos_SendMsg(app, &msg) != SUCCEED)
            {
                Luos_Loop();
            }
        }
        // try to reach a buzzer and drive it to make a happy sound
        if (!lock)
        {
            RTFilter_Alias(RTFilter_Reset(&result), "buzzer_mod");
            if (result.result_nbr > 0)
            {
                msg.header.target = result.result_table[0]->id;
                msg.header.cmd    = IO_STATE;
                msg.header.size   = 1;
                msg.data[0]       = 1;
                while (Luos_SendMsg(app, &msg) != SUCCEED)
                {
                    Luos_Loop();
                }
            }
        }
        // Save switch date
        switch_date = Luos_GetSystick();
        animation_state++;
    }
    // This part is a start stop animation using available services
    if (((Luos_GetSystick() - switch_date) > 100) & (animation_state == 1))
    {
        // 100ms after button turn of light and horn
        msg_t msg;
        msg.header.target_mode = SERVICEIDACK;
        RTFilter_Alias(RTFilter_Reset(&result), "horn");
        if (result.result_nbr > 0)
        {
            // we get a horn
            msg.header.target = result.result_table[0]->id;
            msg.header.size   = sizeof(uint8_t);
            msg.header.cmd    = IO_STATE;
            // turn the horn on/off
            msg.data[0] = 0;
            while (Luos_SendMsg(app, &msg) != SUCCEED)
            {
                Luos_Loop();
            }
        }
        animation_state++;
    }
    if (((Luos_GetSystick() - switch_date) > 600) & (animation_state == 2))
    {
        // 600ms after switch turn light depending on the curent lock state
        msg_t msg;
        msg.header.target_mode = SERVICEIDACK;
        RTFilter_Type(RTFilter_Reset(&result), COLOR_TYPE);
        if (result.result_nbr > 0)
        {
            // we have an alarm, we can set its color
            color_t color;
            if (lock)
            {
                color.r = 0;
                color.g = 0;
                color.b = 0;
            }
            else
            {
                color.r = LIGHT_INTENSITY;
                color.g = LIGHT_INTENSITY;
                color.b = LIGHT_INTENSITY;
            }
            msg.header.target = result.result_table[0]->id;
            IlluminanceOD_ColorToMsg(&color, &msg);
            while (Luos_SendMsg(app, &msg) != SUCCEED)
            {
                Luos_Loop();
            }
        }
        animation_state = 0;
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void StartController_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        if (control_app.flux == PLAY)
        {
            search_result_t result;
            RTFilter_ID(RTFilter_Reset(&result), msg->header.source);
            if (result.result_table[0]->type == STATE_TYPE)
            {
                // this is the button reply we have filter it to manage monostability
                if ((!last_btn_state) & (last_btn_state != msg->data[0]))
                {
                    lock = (!lock);
                    state_switch++;
                }
            }
            else
            {
                // this is an already filtered information
                if ((lock != msg->data[0]))
                {
                    lock = msg->data[0];
                    state_switch++;
                }
            }
            last_btn_state = msg->data[0];
        }
        return;
    }
    if (msg->header.cmd == CONTROL)
    {
        control_app.unmap = msg->data[0];
        return;
    }
    if (msg->header.cmd == END_DETECTION)
    {
        end_detection = 1;
    }
}
