/******************************************************************************
 * @file led
 * @brief driver example a simple led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include "led.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED_PIN 18
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
    revision_t revision;
    revision.major = 1;
    revision.minor = 0;
    revision.build = 0;
    pinMode(LED_PIN, OUTPUT);
    Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led", revision);
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
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        if (msg->data[0] == 0)
        {
            digitalWrite(LED_PIN, false);
        }
        else
        {
            digitalWrite(LED_PIN, true);
        }
    }
}
