/******************************************************************************
 * @file Template state
 * @brief state container template
 * WARING : This h file should be only included by user code or profile_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_TEMPLATE_STATE_H_
#define TEMPLATE_TEMPLATE_STATE_H_

#include <stdbool.h>
#include "luos.h"
#include "struct_state.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * State object
 */
typedef struct
{
    CONT_CB self;
    profile_state_t profile;
} template_state_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

container_t *TemplateState_CreateContainer(CONT_CB cont_cb, template_state_t *var, const char *alias, revision_t revision);

#endif /* TEMPLATE_TEMPLATE_STATE_H_ */
