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
typedef struct profile_ops
{
    void (*Init)(void *);
    void (*Handler)(service_t *, msg_t *);
    SERVICE_CB Callback;
} profile_ops_t;

typedef struct profile_core
{
    // connect an another profile
    struct profile_core *connect;
    // profile structure
    luos_type_t type;
    void *profile_data;
    profile_ops_t profile_ops;
} profile_core_t;

#define CONNECT_AVAILABLE 0

enum
{
    HEAD_PROFILE,
    CONNECT_PROFILE
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
profile_core_t *ProfileCore_GetFromService(service_t *);
profile_core_t *ProfileCore_GetNew(bool);
void ProfileCore_OverrideConnectHandler(void);
service_t *ProfileCore_StartService(SERVICE_CB, const char *, revision_t);

#endif /* PROFILE_CORE_H_ */