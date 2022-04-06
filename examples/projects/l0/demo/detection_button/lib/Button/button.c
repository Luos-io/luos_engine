/******************************************************************************
 * @file button
 * @brief driver example a simple button
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "button.h"
#include "ll_button.h"
#include "profile_state.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
profile_state_t button;
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Button_Init(void)
{
    // low level initialization
    ll_button_init();
    // service initialization
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Profile configuration
    button.access = READ_ONLY_ACCESS;
    // Service creation following state profile
    ProfileState_CreateService(&button, 0, "button", revision);
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Button_Loop(void)
{
    ll_button_read(&button.state);
}