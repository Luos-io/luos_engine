/******************************************************************************
 * @file led
 * @brief driver example a simple led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "led.h"
#include "driver/gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef CONFIG_IDF_TARGET_ESP32
    #define LED_GPIO (GPIO_NUM_18)
#else
    #define LED_GPIO (GPIO_NUM_10)
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t Led_last_state = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Led_Init(void)
{
    gpio_config_t LedConfig;

    gpio_reset_pin(LED_GPIO);
    LedConfig.intr_type    = GPIO_INTR_DISABLE;
    LedConfig.mode         = GPIO_MODE_OUTPUT;
    LedConfig.pin_bit_mask = (1ULL << LED_GPIO);
    LedConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    LedConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
    gpio_config(&LedConfig);

    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Led_Loop(void)
{
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        if (msg->data[0] != Led_last_state)
        {
            Led_last_state = msg->data[0];
            gpio_set_level(LED_GPIO, Led_last_state);
        }
    }
}
