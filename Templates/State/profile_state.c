#include "template_state.h"
#include "profile_state.h"

/******************************************************************************
 * @brief Msg Handler call backed by Luos when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void TemplateState_MsgHandler(service_t *service, msg_t *msg)
{
    template_state_t *state_template = (template_state_t *)service->template_context;
    ProfileState_Handler(service, msg, &state_template->profile);
    if (state_template->self != 0)
    {
        state_template->self(service, msg);
    }
}
/******************************************************************************
 * @brief Service creation following the template
 * @param service_cb is an optional user callback called on every massage for this service
 * @param state_struct template object pointer
 * @param alias for the service string (15 caracters max).
 * @param revision FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return None
 ******************************************************************************/
service_t *TemplateState_CreateService(SERVICE_CB service_cb, template_state_t *state_template, const char *alias, revision_t revision)
{
    state_template->self      = service_cb;
    service_t *service        = Luos_CreateService(TemplateState_MsgHandler, STATE_TYPE, alias, revision);
    service->template_context = (void *)state_template;
    service->access           = state_template->profile.access;
    return service;
}
/******************************************************************************
 * @brief function converting Luos messages innto data and reverse.
 * @param service the target service
 * @param msg the received message
 * @param state_struct the data struct to update
 * @return None
 ******************************************************************************/
void ProfileState_Handler(service_t *service, msg_t *msg, profile_state_t *state_profile)
{
    if ((msg->header.cmd == GET_CMD) && ((state_profile->access == READ_WRITE_ACCESS) || (state_profile->access == READ_ONLY_ACCESS)))
    {
        // fill the message infos
        msg_t pub_msg;
        pub_msg.header.cmd         = IO_STATE;
        pub_msg.header.target_mode = msg->header.target_mode;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(char);
        pub_msg.data[0]            = state_profile->state;
        Luos_SendMsg(service, &pub_msg);
    }
    if ((msg->header.cmd == IO_STATE) && ((state_profile->access == READ_WRITE_ACCESS) || (state_profile->access == WRITE_ONLY_ACCESS)))
    {
        state_profile->state = msg->data[0];
    }
}
