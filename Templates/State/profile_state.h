/******************************************************************************
 * @file Profile state
 * @brief state object managing a true false API
 * WARING : This h file should be only included by profile_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_CLASS_STATE_H_
#define TEMPLATE_CLASS_STATE_H_

#include "struct_state.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void ProfileState_Handler(container_t *container, msg_t *msg, profile_state_t *state_profile);

#endif /* TEMPLATE_CLASS_STATE_H_ */
