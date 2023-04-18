/******************************************************************************
 * @file led
 * @brief driver example a simple led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "led.h"
#include "driver/gpio.h"

#ifndef CONFIG_IDF_TARGET_ESP32
    #include "led_strip.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef CONFIG_IDF_TARGET_ESP32
    #define LED_GPIO (GPIO_NUM_18)
#else
    #define LED_GPIO (GPIO_NUM_8)
static led_strip_t *pStrip_a;
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t Led_last_state = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Led_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
#ifdef CONFIG_IDF_TARGET_ESP32
    gpio_config_t LedConfig;

    gpio_reset_pin(LED_GPIO);
    LedConfig.intr_type    = GPIO_INTR_DISABLE;
    LedConfig.mode         = GPIO_MODE_OUTPUT;
    LedConfig.pin_bit_mask = (1ULL << LED_GPIO);
    LedConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    LedConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
    gpio_config(&LedConfig);
    Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led", revision);
#else
    pStrip_a = led_strip_init(0, LED_GPIO, 1);
    pStrip_a->set_pixel(pStrip_a, 0, 0, 0, 0);
    pStrip_a->refresh(pStrip_a, 50);
    Luos_CreateService(Led_MsgHandler, COLOR_TYPE, "led_rgb", revision);
#endif
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
static void Led_MsgHandler(service_t *service, const msg_t *msg)
{
#ifdef CONFIG_IDF_TARGET_ESP32
    if (msg->header.cmd == IO_STATE)
    {
        if (msg->data[0] != Led_last_state)
        {
            Led_last_state = msg->data[0];
            gpio_set_level(LED_GPIO, Led_last_state);
        }
    }
#else
    color_t rgb;
    if (msg->header.cmd == COLOR)
    {
        IlluminanceOD_ColorFromMsg((color_t *)&rgb, msg);
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        pStrip_a->set_pixel(pStrip_a, 0, rgb.r, rgb.g, rgb.b);
        /* Refresh the strip to send data */
        pStrip_a->refresh(pStrip_a, 50);
    }
#endif
}
