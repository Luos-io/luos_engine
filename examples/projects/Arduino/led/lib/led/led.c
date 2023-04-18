/******************************************************************************
 * @file Led
 * @brief driver example a simple Led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "led.h"
#include "Arduino.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Led_Init(void)
{
    pinMode(LED_BUILTIN, OUTPUT);

    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led_mod", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Led_Loop(void)
{
}
/******************************************************************************
 * @brief Msg manager callback when a msg receive for this service
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        if (msg->data[0] == 0)
        {
            digitalWrite(LED_BUILTIN, false);
        }
        else
        {
            digitalWrite(LED_BUILTIN, true);
        }
    }
}
