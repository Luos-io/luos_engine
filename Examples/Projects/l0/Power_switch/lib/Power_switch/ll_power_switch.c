/******************************************************************************
 * @file low-level Power switch
 * @brief driver example a simple Power switch
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "ll_power_switch.h"
#include "profile_state.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void ll_power_switch_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, SWITCH_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin   = SWITCH_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
};

/******************************************************************************
 * @brief read digital GPIO
 * @param None
 * @return None
 ******************************************************************************/
uint8_t ll_power_switch_write(bool *state)
{
    HAL_GPIO_WritePin(GPIOA, SWITCH_Pin, *state);

    return true;
};
