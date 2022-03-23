/******************************************************************************
 * @file led_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "led_drv.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t led_state = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Communication init must be called in Led_Init
 * @param None
 * @return None
 ******************************************************************************/
void LedDrv_Init(void)
{
    // Led Initialization
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pins : RxEN_Pin */
    GPIO_InitStruct.Pin   = LED_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

/******************************************************************************
 * @brief Led driver write function puts the color to the led
 * @param rgb value
 * @return error value (succeed/failed)
 ******************************************************************************/
void LedDrv_Write(uint8_t value)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, value);
}