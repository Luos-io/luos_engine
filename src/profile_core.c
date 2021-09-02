#include "profile_core.h"

// static initialization of profile table
profile_core_t profile_table[MAX_PROFILE_NUMBER];
uint16_t available_index    = 0;
uint16_t head_profile_index = 0;

// connect override flags
static bool override_connect_handler = false;

/******************************************************************************
 * @brief get profile_context from service
 * @param service
 * @return profile_context pointer
 ******************************************************************************/
profile_core_t *ProfileCore_GetFromService(service_t *service)
{
    return (profile_core_t *)service->profile_context;
}

/******************************************************************************
 * @brief find a profile which can hold a new connect
 * @param profile_mode HEAD / CONNECT
 * @return profile pointer to the dedicated clot
 ******************************************************************************/
profile_core_t *ProfileCore_GetNew(bool profile_mode)
{
    profile_core_t *new_profile = 0;

    // update head index if new head profile is linked
    if (profile_mode == HEAD_PROFILE)
        head_profile_index = available_index;

    // update available profile index
    LUOS_ASSERT(available_index <= (MAX_PROFILE_NUMBER - 1));
    available_index += 1;
    new_profile = &profile_table[available_index - 1];

    // find a connection for a profile
    if (profile_mode == CONNECT_PROFILE)
    {
        profile_core_t *connect_profile = &profile_table[head_profile_index];
        while (true)
        {
            if (connect_profile->connect == CONNECT_AVAILABLE)
                break;
            else
                connect_profile = connect_profile->connect;
        }
        connect_profile->connect = new_profile;
    }

    return new_profile;
}

/******************************************************************************
 * @brief Override connect handler / don't call connect handler
 * @param None
 * @return None
 ******************************************************************************/
void ProfileCore_OverrideConnectHandler(void)
{
    override_connect_handler = true;
}

/******************************************************************************
 * @brief Msg Handler call backed by Luos when a msg receive for this service
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void ProfileCore_Handler(service_t *service, msg_t *msg)
{
    profile_core_t *head_profile = ProfileCore_GetFromService(service);

    // reset flag
    override_connect_handler = false;

    // launch each handler of profile linked list
    // stop if an overrided function is detected
    profile_core_t *current_profile = head_profile;
    while ((current_profile != 0) && (!override_connect_handler))
    {
        // launch profile handler
        if (current_profile->profile_ops.Handler != 0)
            current_profile->profile_ops.Handler(service, msg);
        // find connect profile
        current_profile = current_profile->connect;
        // switch service profile context
        service->profile_context = (void *)current_profile;
    }

    // restore head profile in service
    service->profile_context = (void *)head_profile;
}

/******************************************************************************
 * @brief service creation following the template
 * @param profile_core_t the profile handler to launch
 * @param alias for the service string (15 caracters max).
 * @param revision FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return None
 ******************************************************************************/
service_t *ProfileCore_StartService(SERVICE_CB callback, const char *alias, revision_t revision)
{
    profile_core_t *head_profile = &profile_table[head_profile_index];

    // link service callback
    head_profile->profile_ops.Callback = callback;

    // launch profile init
    if (head_profile->profile_ops.Init != 0)
    {
        head_profile->profile_ops.Init((void *)head_profile);
    }

    service_t *service       = Luos_CreateService(ProfileCore_Handler, head_profile->type, alias, revision);
    service->profile_context = (void *)head_profile;

    return service;
}
