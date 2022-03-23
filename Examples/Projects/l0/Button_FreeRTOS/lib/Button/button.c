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
service_t *app;
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
    app = ProfileState_CreateService(&button, 0, "button", revision);
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Button_Loop(void)
{
    ll_button_read(&button.state);
    search_result_t result;
    // Get the ID of our LED from the routing table
    RTFilter_Alias(RTFilter_Reset(&result), "led_service");
    if (result.result_nbr > 0)
    {
        // Now send a message
        msg_t led_msg;
        led_msg.header.target      = result.result_table[0]->id;
        led_msg.header.cmd         = IO_STATE;
        led_msg.header.target_mode = IDACK;
        led_msg.header.size        = sizeof(char);
        led_msg.data[0]            = !(uint8_t)button.state;
        Luos_SendMsg(app, &led_msg);
    }
}