/******************************************************************************
 * @file low-level button
 * @brief driver example a simple button
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "ll_button.h"
#include "stm32f0xx_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief initialize button hardware
 * @param None
 * @return None
 ******************************************************************************/
void ll_button_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // configure gpio clock
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin : button pin */
    GPIO_InitStruct.Pin  = BTN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);
}

/******************************************************************************
 * @brief read the button state
 * @param None
 * @return button state
 ******************************************************************************/
uint8_t ll_button_read(bool *state)
{
    *state = (bool)HAL_GPIO_ReadPin(BTN_GPIO_Port, BTN_Pin);

    return true;
}