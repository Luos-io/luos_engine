#include "profile_core.h"

/******************************************************************************
 * @brief get profile_context from service
 * @param service
 * @return profile_context pointer
 ******************************************************************************/
profile_core_t *Luos_GetProfileFromService(service_t *container)
{
    return (profile_core_t *)container->profile_context;
}

/******************************************************************************
 * @brief Msg Handler call backed by Luos when a msg receive for this service
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Luos_ProfileHandler(service_t *service, msg_t *msg)
{
    profile_core_t *profile = Luos_GetProfileFromService(service);

    // auto-update profile data
    profile->profile_ops.Handler(service, msg);

    // call the profile callback when the service receive a message
    if (profile->profile_ops.Callback != 0)
    {
        profile->profile_ops.Callback(service, msg);
    }
}

/******************************************************************************
 * @brief service creation following the template
 * @param profile_core_t the profile handler to launch
 * @param alias for the service string (15 caracters max).
 * @param revision FW for the service (tab[MajorVersion,MinorVersion,Patch])
 * @return None
 ******************************************************************************/
service_t *Luos_LaunchProfile(profile_core_t *profile, const char *alias, revision_t revision)
{
    // call the profile init function if needed
    if (profile->profile_ops.Init != 0)
    {
        profile->profile_ops.Init((HANDLER *)profile);
    }

    service_t *service       = Luos_CreateService(Luos_ProfileHandler, profile->type, alias, revision);
    service->profile_context = (HANDLER *)profile;

    return service;
}