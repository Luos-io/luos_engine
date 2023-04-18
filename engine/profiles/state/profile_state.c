/******************************************************************************
 * @file profile_state
 * @brief state profile
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "profile_state.h"

/******************************************************************************
 * @brief Function converting Luos messages into data and reverse.
 * @param service : Service the target service
 * @param msg : The received message
 * @return None
 ******************************************************************************/
void ProfileState_Handler(service_t *service, const msg_t *msg)
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

                // send message on the network
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
 * @param profile : Profile data structure
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
 * @param profile : Profile data structure
 * @param callback : Callback used by the service
 * @param alias : Alias for the service (15 caracters max string)
 * @param revision : FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return Service pointer
 ******************************************************************************/
service_t *ProfileState_CreateService(profile_state_t *profile_state, SERVICE_CB callback, const char *alias, revision_t revision)
{
    // link head profile
    ProfileState_link(HEAD_PROFILE, profile_state);

    // Start profile
    return ProfileCore_StartService(callback, alias, revision);
}
