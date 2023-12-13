/******************************************************************************
 * @file distance
 * @brief driver example a simple distance
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "vl53l0x_drv.h"

#include "distance.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ADDRESS_DEFAULT 0b0101001

/*******************************************************************************
 * Variables
 ******************************************************************************/

linear_position_t dist;
bool new_data_ready = false;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Distance_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Distance_Init(void)
{
    dist                = LinearOD_PositionFrom_mm(-1);
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    vl53l0x_DRVInit();
    Luos_CreateService(Distance_MsgHandler, DISTANCE_TYPE, "distance", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Distance_Loop(void)
{
    if (vl53l0x_DrvRead(&dist) == SUCCEED)
    {
        new_data_ready = true;
    }
}
/******************************************************************************
 * @brief Msg handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Distance_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        msg_t pub_msg;
        // dist measurement ok
        pub_msg.header.target_mode = SERVICEID;
        pub_msg.header.target      = msg->header.source;
        LinearOD_PositionToMsg(&dist, &pub_msg);
        new_data_ready = false;
        Luos_SendMsg(service, &pub_msg);
    }
}
