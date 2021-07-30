/******************************************************************************
 * @file Profile 
 * @brief service profile
 * WARING : This h file should be only included by user code or profile_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PROFILE_CORE_H
#define PROFILE_CORE_H

#include <stdbool.h>
#include "luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    void (*Init)(HANDLER *);
    void (*Handler)(service_t *, msg_t *);
    SERVICE_CB Callback;
} profile_ops_t;

typedef struct
{
    luos_type_t type;
    HANDLER *profile_data;
    profile_ops_t profile_ops;
} profile_core_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
profile_core_t *Luos_GetProfileFromService(service_t *);
service_t *Luos_LaunchProfile(profile_core_t *, const char *, revision_t);

#endif /* PROFILE_CORE_H_ */