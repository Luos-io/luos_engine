/******************************************************************************
 * @file Power switch
 * @brief driver example a simple Power switch
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "power_switch.h"
#include "ll_power_switch.h"

#include "profile_state.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
profile_state_t power_switch;
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PowerSwitch_Init(void)
{
    // hardware initialization
    ll_power_switch_init();

    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Profile configuration
    power_switch.access = WRITE_ONLY_ACCESS;

    ProfileState_CreateService(&power_switch, 0, "power_switch", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PowerSwitch_Loop(void)
{
    // write power switch state to the driver
    ll_power_switch_write(&power_switch.state);
}
