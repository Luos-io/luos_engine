/******************************************************************************
 * @file Profile state
 * @brief state object managing a true false API
 * WARING : This h file should be only included by profile_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PROFILE_STATE_H_
#define PROFILE_STATE_H_

#include "profile_core.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * State data
 */
typedef struct
{
    access_t access;
    bool state;
} profile_state_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void ProfileState_link(uint8_t profile_mode, profile_state_t *profile_state);
service_t *ProfileState_CreateService(profile_state_t *, SERVICE_CB, const char *, revision_t);

#endif /* PROFILE_STATE_H_ */
