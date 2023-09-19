/******************************************************************************
 * @file load
 * @brief driver example a simple load
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "load.h"
#include "HX711.h"
#include "string.h"
#include "stdbool.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
force_t load;
bool new_data_ready = false;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Load_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Load_Init(void)
{
    load                = ForceOD_ForceFrom_N(0.0f);
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    hx711_Init();
    Luos_CreateService(Load_MsgHandler, LOAD_TYPE, "load", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Load_Loop(void)
{
    if (hx711_ReadValue(&load) == SUCCEED)
    {
        new_data_ready = true;
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Load_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        if (new_data_ready)
        {
            msg_t pub_msg;
            // fill the message infos
            pub_msg.header.target_mode = SERVICEID;
            pub_msg.header.target      = msg->header.source;
            ForceOD_ForceToMsg((force_t *)&load, &pub_msg);
            Luos_SendMsg(service, &pub_msg);
            new_data_ready = false;
        }
        return;
    }
    if (msg->header.cmd == REINIT)
    {
        hx711_tare(DEFAULT_TARE_TIME);
        return;
    }
    if (msg->header.cmd == RESOLUTION)
    {
        // put this value in scale
        float value = 0.0;
        memcpy(&value, msg->data, sizeof(value));
        hx711_set_scale(value);
        return;
    }
    if (msg->header.cmd == OFFSET)
    {
        force_t value = ForceOD_ForceFrom_N(0.0);
        ForceOD_ForceFromMsg(&value, msg);
        hx711_set_offset((long)(ForceOD_ForceTo_N(value) * hx711_get_scale()));
        return;
    }
}
