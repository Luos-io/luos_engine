/******************************************************************************
 * @file
 * MDL_APPDetection.c
 * @author Luos
 * @version 1.0.0
 ******************************************************************************/
#include "Detection.h"
#include <Luos.h>
#include "luos_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
container_t *app;

typedef enum
{
    MDL_APPDETECTION = LUOS_LAST_TYPE
} App_type_t;

int8_t id_button;
int8_t id_led;
uint8_t button_last_state = 0;
/*******************************************************************************
 * Functions
 ******************************************************************************/
static void Detection_MsgHandler(container_t *container, msg_t *msg);
static void Detection_LedState(msg_t *msg, uint8_t state);
/*******************************************************************************
 * Code
 ******************************************************************************/

/******************************************************************************
 * @brief
 *   User setup function.
 * @Param
 *
 * @Return
 *
 ******************************************************************************/
void Detection_Init(void)
{
    revision_t revision = {.unmap = REV};
    uint32_t time = LuosHAL_GetSystick();
    app = Luos_CreateContainer(Detection_MsgHandler, MDL_APPDETECTION, "App_Detection", revision);
    while((Luos_GetSystick() - time) < 2);
    // Detect all containers of your network and create a routing_table
    RoutingTB_DetectContainers(app);

    id_button = RoutingTB_IDFromAlias("button_mod");
    id_led = RoutingTB_IDFromAlias("rgb_led_mod1");

    /*button Configuration*/
    if (id_button > 0)
    {
        msg_t msg;

        msg.header.target = id_button;
        msg.header.target_mode = IDACK;

        // Setup auto update each 10ms on button
        time_luos_t time = TimeOD_TimeFrom_ms(10);
        TimeOD_TimeToMsg(&time, &msg);
        msg.header.cmd = UPDATE_PUB;
        Luos_SendMsg(app, &msg);
    }
}
/******************************************************************************
 * @brief
 *   User setup function.
 * @Param
 *
 * @Return
 *
 ******************************************************************************/
void Detection_Loop(void)
{
}
/******************************************************************************
 * @brief
 *   User setup function.
 * @Param
 *
 * @Return
 *
 *****************************************************************************/
static void Detection_MsgHandler(container_t *container, msg_t *msg)
{
    msg_t led_msg;

    if (msg->header.cmd == IO_STATE)
    {
        if (msg->data[0] != button_last_state)
        {
            button_last_state = msg->data[0];
            if (id_led > 0)
            {
                led_msg.header.cmd = IO_STATE;
                led_msg.header.target_mode = IDACK;
                led_msg.header.target = id_led;
                led_msg.header.size = sizeof(char);
                Detection_LedState(&led_msg, button_last_state);
                Luos_SendMsg(container, &led_msg);
            }
        }
    }
}

/******************************************************************************
 * @brief
 *   User setup function.
 * @Param
 *
 * @Return
 *
 *****************************************************************************/
static void Detection_LedState(msg_t *msg, uint8_t state)
{
    color_t color;

    if (state == 0)
    {
        color.r = 0;
        color.g = 0;
        color.b = 0;
    }
    else
    {
        color.r = 255;
        color.g = 0;
        color.b = 0;
    }
    IlluminanceOD_ColorToMsg(&color, msg);
}
