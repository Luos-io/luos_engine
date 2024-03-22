/******************************************************************************
 * @file profile_motor
 * @brief Motor profile
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "profile_motor.h"
#include "math.h"

/******************************************************************************
 * @brief Function converting Luos messages into data and reverse.
 * @param service : The target service
 * @param msg : The received message
 * @return None
 ******************************************************************************/
void ProfileMotor_Handler(service_t *service, const msg_t *msg)
{
    profile_core_t *profile        = ProfileCore_GetFromService(service);
    profile_motor_t *profile_motor = (profile_motor_t *)profile->profile_data;

    switch (msg->header.cmd)
    {
        case GET_CMD:
        {
            msg_t pub_msg;
            pub_msg.header.target_mode = msg->header.target_mode;
            pub_msg.header.target      = msg->header.source;
            if (profile_motor->mode.current)
            {
                ElectricOD_CurrentToMsg(&profile_motor->current, &pub_msg);
                Luos_SendMsg(service, &pub_msg);
            }
            if (profile_motor->mode.temperature)
            {
                TemperatureOD_TemperatureToMsg(&profile_motor->temperature, &pub_msg);
                Luos_SendMsg(service, &pub_msg);
            }
        }
        break;
        case RATIO:
        {
            // get the motor power ratio
            RatioOD_RatioFromMsg(&profile_motor->power, msg);
            if (RatioOD_RatioTo_Percent(profile_motor->power) > 100.0)
                profile_motor->power = RatioOD_RatioFrom_Percent(100.0);
            if (RatioOD_RatioTo_Percent(profile_motor->power) < -100.0)
                profile_motor->power = RatioOD_RatioFrom_Percent(-100.0);
        }
        break;
        case RATIO_LIMIT:
        {
            // set the motor power ratio limit
            RatioOD_RatioFromMsg(&profile_motor->limit_ratio, msg);
            profile_motor->limit_ratio = RatioOD_RatioFrom_Percent(fabs(RatioOD_RatioTo_Percent(profile_motor->limit_ratio)));
            if (RatioOD_RatioTo_Percent(profile_motor->limit_ratio) > 100.0)
                profile_motor->limit_ratio = RatioOD_RatioFrom_Percent(100.0);
        }
        break;
        case CURRENT_LIMIT:
        {
            // set the motor current limit
            ElectricOD_CurrentFromMsg((current_t *)&profile_motor->limit_current, msg);
            profile_motor->limit_current = ElectricOD_CurrentFrom_A(fabs(ElectricOD_CurrentTo_A(profile_motor->limit_current)));
        }
        break;
        case TEMPERATURE_LIMIT:
        {
            // set the motor power ratio limit
            TemperatureOD_TemperatureFromMsg(&profile_motor->limit_temperature, msg);
            profile_motor->limit_temperature = TemperatureOD_TemperatureFrom_deg_c(fabs(TemperatureOD_TemperatureTo_deg_c(profile_motor->limit_temperature)));
        }
        break;
        case PARAMETERS:
        {
            // check the message size
            // fill the message infos
            memcpy((void *)&profile_motor->mode, msg->data, sizeof(motor_mode_t));
        }
        break;
        default:
        {
        }
        break;
    }

    if ((profile->profile_ops.Callback != 0))
    {
        profile->profile_ops.Callback(service, msg);
    }
}

/******************************************************************************
 * @brief Link profile to the general profile handler
 * @param profile_mode : HEAD / CONNECT
 * @param profile : profile data structure
 * @return None
 ******************************************************************************/
void ProfileMotor_link(uint8_t profile_mode, profile_motor_t *profile_motor)
{
    profile_core_t *profile      = ProfileCore_GetNew(profile_mode);
    profile->type                = MOTOR_TYPE;
    profile->profile_data        = (void *)profile_motor;
    profile->profile_ops.Init    = 0;
    profile->profile_ops.Handler = ProfileMotor_Handler;
}

/******************************************************************************
 * @brief Create a service with a linked profile
 * @param profile : Profile data structure
 * @param callback : Callback used by the service
 * @param alias : Alias for the service (15 caracters max string)
 * @param revision : FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return service pointer
 ******************************************************************************/
service_t *ProfileMotor_CreateService(profile_motor_t *profile_motor, SERVICE_CB callback, const char *alias, revision_t revision)
{
    // link head profile
    ProfileMotor_link(HEAD_PROFILE, profile_motor);

    // Start service with the linked profile
    return ProfileCore_StartService(callback, alias, revision);
}

/******************************************************************************
 * @brief Get the profile data from a service pointer
 * @param service : service pointer
 * @return profile_motor_t pointer
 ******************************************************************************/
profile_motor_t *ProfileMotor_GetFromService(service_t *service)
{
    profile_core_t *profile = ProfileCore_GetFromService(service);
    if (profile == 0)
    {
        return 0;
    }
    if (profile->type != MOTOR_TYPE)
    {
        return 0;
    }
    return (profile_motor_t *)profile->profile_data;
}
