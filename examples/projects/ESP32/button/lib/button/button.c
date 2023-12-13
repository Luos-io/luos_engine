/******************************************************************************
 * @file button
 * @brief driver example a simple button
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "button.h"
#include "driver/gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef CONFIG_IDF_TARGET_ESP32
    #define BTN_GPIO (GPIO_NUM_18)
#else
    #define BTN_GPIO (GPIO_NUM_10)
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Button_MsgHandler(service_t *service, const msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Button_Init(void)
{
    gpio_config_t btnConfig;

    gpio_reset_pin(BTN_GPIO);
    btnConfig.intr_type    = GPIO_INTR_DISABLE;
    btnConfig.mode         = GPIO_MODE_INPUT;
    btnConfig.pin_bit_mask = (1ULL << BTN_GPIO);
    btnConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    btnConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
    gpio_config(&btnConfig);

    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Button_MsgHandler, STATE_TYPE, "button", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Button_Loop(void)
{
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Button_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        // fill the message infos
        msg_t pub_msg;
        pub_msg.header.cmd         = IO_STATE;
        pub_msg.header.target_mode = msg->header.target_mode;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(char);
        pub_msg.data[0]            = gpio_get_level(BTN_GPIO);
        Luos_SendMsg(service, &pub_msg);
        return;
    }
}
