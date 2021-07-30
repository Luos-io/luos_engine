#include "template_voltage.h"
#include "profile_voltage.h"

/******************************************************************************
 * @brief Msg Handler call backed by Luos when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void TemplateVoltage_MsgHandler(service_t *service, msg_t *msg)
{
    template_voltage_t *voltage_template = (template_voltage_t *)service->template_context;
    ProfileVoltage_Handler(service, msg, &voltage_template->profile);
    if (voltage_template->self != 0)
    {
        voltage_template->self(service, msg);
    }
}
/******************************************************************************
 * @brief Service creation following the template
 * @param service_cb is an optional user callback called on every massage for this service
 * @param voltage_template template object pointer
 * @param alias for the service string (15 caracters max).
 * @param revision FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return None
 ******************************************************************************/
service_t *TemplateVoltage_CreateService(SERVICE_CB service_cb, template_voltage_t *voltage_template, const char *alias, revision_t revision)
{
    voltage_template->self    = service_cb;
    service_t *service        = Luos_CreateService(TemplateVoltage_MsgHandler, VOLTAGE_TYPE, alias, revision);
    service->template_context = (void *)voltage_template;
    service->access           = voltage_template->profile.access;
    return service;
}
/******************************************************************************
 * @brief function converting Luos messages innto data and reverse.
 * @param service the target service
 * @param msg the received message
 * @param voltage_profile the profile struct to update
 * @return None
 ******************************************************************************/
void ProfileVoltage_Handler(service_t *service, msg_t *msg, profile_voltage_t *voltage_profile)
{
    if ((msg->header.cmd == GET_CMD) && ((voltage_profile->access == READ_WRITE_ACCESS) || (voltage_profile->access == READ_ONLY_ACCESS)))
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
    if ((msg->header.cmd == VOLTAGE) && ((voltage_profile->access == READ_WRITE_ACCESS) || (voltage_profile->access == WRITE_ONLY_ACCESS)))
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
    if (msg->header.cmd == TIME)
    {
        // save time in ms
        TimeOD_TimeFromMsg((time_luos_t *)&voltage_profile->sampling_period, msg);
        return;
    }
}
