/******************************************************************************
 * @file distance
 * @brief driver example a simple distance
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "vl53l0x_drv.h"

#include "distance.h"
#include "timestamp.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ADDRESS_DEFAULT 0b0101001

/*******************************************************************************
 * Variables
 ******************************************************************************/

linear_position_t dist = -0.001;
bool new_data_ready    = false;
timestamp_token_t distance_timestamp;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Distance_MsgHandler(service_t *service, msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Distance_Init(void)
{
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
        Timestamp_Tag(&distance_timestamp, &dist);
    }
}
/******************************************************************************
 * @brief Msg handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Distance_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        if (new_data_ready)
        {
            msg_t pub_msg;
            // dist measurement ok
            pub_msg.header.target_mode = ID;
            pub_msg.header.target      = msg->header.source;
            LinearOD_PositionToMsg(&dist, &pub_msg);
            new_data_ready = false;
            if (Timestamp_GetToken(&dist))
            {
                Timestamp_EncodeMsg(&pub_msg, &dist);
            }
            Luos_SendMsg(service, &pub_msg);
        }
    }
}
