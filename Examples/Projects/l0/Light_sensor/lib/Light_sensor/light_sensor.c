/******************************************************************************
 * @file light sensor
 * @brief driver example a simple light sensor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "main.h"
#include "light_sensor.h"
#include "string.h"
#include "timestamp.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
illuminance_t lux = 0.0;
timestamp_token_t lux_timestamp;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void LightSensor_MsgHandler(service_t *service, msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void LightSensor_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // **************** communication driver init *************
    LightSensorDrv_Init();
    // ******************* service creation *******************
    Luos_CreateService(LightSensor_MsgHandler, LIGHT_TYPE, "light_sensor", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void LightSensor_Loop(void)
{
    // read and store the value that captured by the sensor
    lux = LightSensorDrv_Read();
    Timestamp_Tag(&lux_timestamp, &lux);
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 *        Light sensor receives GET_CMD messages so it need to send its value
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void LightSensor_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        msg_t pub_msg;
        // fill the message infos
        pub_msg.header.target_mode = ID;
        pub_msg.header.target      = msg->header.source;
        // transform the illuminance value to message format using Luos Object Dictionary
        IlluminanceOD_IlluminanceToMsg((illuminance_t *)&lux, &pub_msg);
        // find if this value is timestamped
        if (Timestamp_GetToken(&lux))
        {
            Timestamp_EncodeMsg(&pub_msg, &lux);
        }
        // send the illuminance that the sensor captured
        Luos_SendMsg(service, &pub_msg);
        return;
    }
}
