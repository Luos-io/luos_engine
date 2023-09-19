/******************************************************************************
 * @file Led
 * @brief driver example a simple Led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "led_fader.h"
#include <math.h>
#include "string.h"
#include "ledfader_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// led fader service new custom type added in the end of the types list
typedef enum
{
    LED_FADER_TYPE = LUOS_LAST_TYPE

} led_fader_type_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile color_t last_rgb;
color_t target_rgb;
time_luos_t time;
uint32_t init_time     = 0;
static int elapsed_ms  = 0;
volatile float coef[3] = {0.0};

/*******************************************************************************
 * Function
 ******************************************************************************/
static void LedFader_MsgHandler(service_t *service, const msg_t *msg);
/******************************************************************************
 * @brief package init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void LedFader_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // time initialization/ transform time using the Object Dictionary for time
    time = TimeOD_TimeFrom_ms(0.0);
    // create the Luos service for Led Fader
    Luos_CreateService(LedFader_MsgHandler, COLOR_TYPE, "alarm", revision);
    // initialize the Led driver
    LedFaderDrv_Init();
}
/******************************************************************************
 * @brief package loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void LedFader_Loop(void)
{
    color_t rgb;
    // calculate how much time is the led fading
    elapsed_ms = Luos_GetSystick() - init_time;
    if (TimeOD_TimeTo_s(time) != 0)
    {
        if ((float)elapsed_ms > TimeOD_TimeTo_ms(time))
        {
            // we finished the transition
            LedFaderDrv_Write((color_t *)&target_rgb);
            memcpy((color_t *)&last_rgb, (color_t *)&target_rgb, sizeof(color_t));
        }
        else
        {
            // calculate the new led color and send it
            rgb.r = (int)(coef[0] * (float)elapsed_ms + (float)last_rgb.r);
            rgb.g = (int)(coef[1] * (float)elapsed_ms + (float)last_rgb.g);
            rgb.b = (int)(coef[2] * (float)elapsed_ms + (float)last_rgb.b);
            // light the led
            LedFaderDrv_Write((color_t *)&rgb);
        }
    }
    else
    {
        LedFaderDrv_Write((color_t *)&target_rgb);
    }
}

/******************************************************************************
 * @brief Msg manager callback when a msg receive for the led fader service
 *        the Led Fader can receive msg commands of type Color and Time.
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
void LedFader_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == COLOR)
    {
        // change led target color
        memcpy((color_t *)&last_rgb, (color_t *)&target_rgb, sizeof(color_t));
        // Get the color value from the received msg data using Luos Object Dictionary
        IlluminanceOD_ColorFromMsg((color_t *)&target_rgb, msg);
        if (TimeOD_TimeTo_ms(time) > 0.0)
        {
            elapsed_ms = 0;
            coef[0]    = (float)(target_rgb.r - last_rgb.r) / TimeOD_TimeTo_ms(time);
            coef[1]    = (float)(target_rgb.g - last_rgb.g) / TimeOD_TimeTo_ms(time);
            coef[2]    = (float)(target_rgb.b - last_rgb.b) / TimeOD_TimeTo_ms(time);
        }
        // keep the time we are going to light the led
        init_time = Luos_GetSystick();
        return;
    }
    else if (msg->header.cmd == TIME)
    {
        // save new transition time
        // Get the time value from the received msg data using Luos Object Dictionary
        TimeOD_TimeFromMsg((time_luos_t *)&time, msg);
        return;
    }
}
