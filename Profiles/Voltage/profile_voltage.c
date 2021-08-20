#include "profile_voltage.h"

/******************************************************************************
 * @brief function converting Luos messages into data and reverse.
 * @param service the target service
 * @param msg the received message
 * @return None
 ******************************************************************************/
void ProfileVoltage_Handler(service_t *service, msg_t *msg)
{
    profile_core_t *profile            = ProfileCore_GetFromService(service);
    profile_voltage_t *voltage_profile = (profile_voltage_t *)profile->profile_data;

    switch (msg->header.cmd)
    {
        case GET_CMD:
            if ((voltage_profile->access == READ_WRITE_ACCESS) || (voltage_profile->access == READ_ONLY_ACCESS))
            {
                // fill the message infos
                msg_t pub_msg;
                pub_msg.header.target_mode = msg->header.target_mode;
                pub_msg.header.target      = msg->header.source;
                if (voltage_profile->control.rec)
                {
                    LUOS_ASSERT(voltage_profile->signal.data_ptr != 0);
                    // send back a record stream
                    pub_msg.header.cmd = VOLTAGE;
                    Luos_SendStreaming(service, &pub_msg, &voltage_profile->signal);
                }
                else
                {
                    ElectricOD_VoltageToMsg(&voltage_profile->voltage, &pub_msg);
                    Luos_SendMsg(service, &pub_msg);
                }
            }
            break;
        case VOLTAGE:
            if ((voltage_profile->access == READ_WRITE_ACCESS) || (voltage_profile->access == WRITE_ONLY_ACCESS))
            {
                if (msg->header.size == sizeof(voltage_t))
                {
                    // Get the voltage
                    ElectricOD_VoltageFromMsg(&voltage_profile->voltage, msg);
                }
                else
                {
                    LUOS_ASSERT(voltage_profile->signal.data_ptr != 0);
                    // this is a signal, save it into ring buffer.
                    Luos_ReceiveStreaming(service, msg, &voltage_profile->signal);
                    // values will be converted one by one during trajectory management.
                }
            }
            break;
        case TIME:
        {
            // save time in ms
            TimeOD_TimeFromMsg((time_luos_t *)&voltage_profile->sampling_period, msg);
        }
        break;
        default:
        {
            return;
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
 * @param profile_mode HEAD / CONNECT
 * @param profile data structure
 * @return None
 ******************************************************************************/
void ProfileVoltage_link(uint8_t profile_mode, profile_voltage_t *profile_voltage)
{
    profile_core_t *profile      = ProfileCore_GetNew(profile_mode);
    profile->type                = VOLTAGE_TYPE;
    profile->profile_data        = (void *)profile_voltage;
    profile->profile_ops.Init    = 0;
    profile->profile_ops.Handler = ProfileVoltage_Handler;
}

/******************************************************************************
 * @brief Create a service with a linked profile
 * @param profile data structure
 * @param callback used by the service
 * @param alias 
 * @param revision 
 * @return service pointer
 ******************************************************************************/
service_t *ProfileVoltage_CreateService(profile_voltage_t *profile_voltage, SERVICE_CB callback, const char *alias, revision_t revision)
{
    // link head profile
    ProfileVoltage_link(HEAD_PROFILE, profile_voltage);

    // Start profile
    return ProfileCore_StartService(callback, alias, revision);
}