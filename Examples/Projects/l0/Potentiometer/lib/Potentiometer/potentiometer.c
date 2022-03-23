/******************************************************************************
 * @file potentiometer
 * @brief driver example a simple potentiometer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "main.h"
#include "potentiometer.h"
#include "timestamp.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
angular_position_t angle = 0.0;
timestamp_token_t angle_timestamp;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Potentiometer_MsgHandler(service_t *service, msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Potentiometer_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // potentiometer driver init
    PotentiometerDrv_Init();
    // ******************* service creation *******************
    Luos_CreateService(Potentiometer_MsgHandler, ANGLE_TYPE, "potentiometer", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Potentiometer_Loop(void)
{
    // read and save the angular position value
    angle = PotentiometerDrv_Read();
    Timestamp_Tag(&angle_timestamp, &angle);
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 *
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Potentiometer_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        msg_t pub_msg;
        // fill the message infos
        pub_msg.header.target_mode = ID;
        // the message destination is the service that asked information
        pub_msg.header.target = msg->header.source;
        // convert the position to message data using Luos Object Dictionary
        AngularOD_PositionToMsg((angular_position_t *)&angle, &pub_msg);
        // find if this value is timestamped
        if (Timestamp_GetToken(&angle))
        {
            Timestamp_EncodeMsg(&pub_msg, &angle);
        }
        // send the angular position information
        Luos_SendMsg(service, &pub_msg);
        return;
    }
}
