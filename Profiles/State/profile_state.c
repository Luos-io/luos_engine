#include "profile_state.h"

/******************************************************************************
 * @brief function converting Luos messages into data and reverse.
 * @param service the target service
 * @param msg the received message
 * @return None
 ******************************************************************************/
void ProfileState_Handler(service_t *service, msg_t *msg)
{
    profile_core_t *profile        = ProfileCore_GetFromService(service);
    profile_state_t *profile_state = (profile_state_t *)profile->profile_data;

    switch (msg->header.cmd)
    {
        case GET_CMD:
            if ((profile_state->access == READ_WRITE_ACCESS) || (profile_state->access == READ_ONLY_ACCESS))
            {
                // fill the message infos
                msg_t pub_msg;
                pub_msg.header.cmd         = IO_STATE;
                pub_msg.header.target_mode = msg->header.target_mode;
                pub_msg.header.target      = msg->header.source;
                pub_msg.header.size        = sizeof(bool);
                memcpy(&pub_msg.data, &profile_state->state, sizeof(bool));
                Luos_SendMsg(service, &pub_msg);
            }
            break;
        case IO_STATE:
            if ((profile_state->access == READ_WRITE_ACCESS) || (profile_state->access == WRITE_ONLY_ACCESS))
            {
                memcpy(&profile_state->state, &msg->data, sizeof(bool));
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
void ProfileState_link(uint8_t profile_mode, profile_state_t *profile_state)
{
    profile_core_t *profile      = ProfileCore_GetNew(profile_mode);
    profile->type                = STATE_TYPE;
    profile->profile_data        = (void *)profile_state;
    profile->profile_ops.Init    = 0;
    profile->profile_ops.Handler = ProfileState_Handler;
}

/******************************************************************************
 * @brief Create a service with a linked profile
 * @param profile data structure
 * @param callback used by the service
 * @param alias 
 * @param revision 
 * @return service pointer
 ******************************************************************************/
service_t *ProfileState_CreateService(profile_state_t *profile_state, SERVICE_CB callback, const char *alias, revision_t revision)
{
    // link head profile
    ProfileState_link(HEAD_PROFILE, profile_state);

    // Start profile
    return ProfileCore_StartService(callback, alias, revision);
}