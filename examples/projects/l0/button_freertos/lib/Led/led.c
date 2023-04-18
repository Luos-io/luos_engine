/******************************************************************************
 * @file Led
 * @brief driver example a simple Led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "led.h"
#include <math.h>
#include "string.h"
#include "led_drv.h"

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
 * @brief package init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Led_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Initialization of Led Driver
    LedDrv_Init();
    // Create Led service
    Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led_service", revision);
}
/******************************************************************************
 * @brief package loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Led_Loop(void)
{
}
/******************************************************************************
 * @brief Msg manager callback when a msg receive for the led service
 *        Led can receive messages commands of type COLOR
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        // get state value from message
        uint8_t state = msg->data[0];
        // Send the state value to the led
        LedDrv_Write(state);
    }
}
