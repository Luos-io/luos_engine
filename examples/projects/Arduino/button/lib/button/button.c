/******************************************************************************
 * @file button
 * @brief driver example a simple button
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include "button.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BTN_PIN 8
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Button_MsgHandler(service_t *service, const msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Button_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    pinMode(BTN_PIN, INPUT);
    Luos_CreateService(Button_MsgHandler, STATE_TYPE, "button", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Button_Loop(void)
{
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Button_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        // fill the message infos
        msg_t pub_msg;
        pub_msg.header.cmd         = IO_STATE;
        pub_msg.header.target_mode = SERVICEID;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(char);
        pub_msg.data[0]            = digitalRead(BTN_PIN);
        time_luos_t timestamp      = Luos_Timestamp();

        Luos_SendTimestampMsg(service, &pub_msg, timestamp);
        return;
    }
}
