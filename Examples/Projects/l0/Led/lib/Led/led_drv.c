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
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/*******************************************************************************
 * Function
 ******************************************************************************/

void LedTIM2_Init(void);
void LedTIM3_Init(void);

/******************************************************************************
 * @brief Communication init must be called in Led_Init
 * @param None
 * @return None
 ******************************************************************************/
void LedDrv_Init(void)
{
    LedTIM2_Init();
    LedTIM3_Init();

    HAL_TIM_PWM_Start(&htim3, LED_TIM3_CHANNEL1);
    HAL_TIM_PWM_Start(&htim3, LED_TIM3_CHANNEL2);
    HAL_TIM_PWM_Start(&htim2, LED_TIM2_CHANNEL);
}

/******************************************************************************
 * @brief TIM2 Init
 * @param None
 * @return None
 ******************************************************************************/
void LedTIM2_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC          = {0};
    GPIO_InitTypeDef GPIO_InitStruct      = {0};

    LED_TIM2_CLK();

    htim2.Instance               = LED_TIM2_INSTANCE;
    htim2.Init.Prescaler         = 0;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 2550 - 1;
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim2);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, LED_TIM2_CHANNEL);

    LED_TIM2_GPIO_CLK();
    /**TIM2 GPIO Configuration
    PA0     ------> TIM2_CH1
    */
    GPIO_InitStruct.Pin       = LED_TIM2_CH_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = LED_AF_TIM2;
    HAL_GPIO_Init(LED_TIM2_PORT, &GPIO_InitStruct);
}
/******************************************************************************
 * @brief TIM3 Init
 * @param None
 * @return None
 ******************************************************************************/
void LedTIM3_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC          = {0};
    GPIO_InitTypeDef GPIO_InitStruct      = {0};

    LED_TIM3_CLK();

    htim3.Instance               = LED_TIM3_INSTANCE;
    htim3.Init.Prescaler         = 0;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 2550 - 1;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim3);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, LED_TIM3_CHANNEL1);

    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, LED_TIM3_CHANNEL2);

    LED_TIM3_GPIO_CLK();
    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1
    PB5     ------> TIM3_CH2
    */
    GPIO_InitStruct.Pin       = LED_TIM3_CH1_PIN | LED_TIM3_CH2_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = LED_AF_TIM3;
    HAL_GPIO_Init(LED_TIM3_PORT, &GPIO_InitStruct);
}
/******************************************************************************
 * @brief Led driver write function puts the color to the led
 * @param rgb value
 * @return error value (succeed/failed)
 ******************************************************************************/
void LedDrv_Write(color_t *rgb)
{
    LED_TIM3_INSTANCE->CCR1 = (uint16_t)rgb->r * 10;
    LED_TIM3_INSTANCE->CCR2 = (uint16_t)rgb->g * 10;
    LED_TIM2_INSTANCE->CCR1 = (uint16_t)rgb->b * 10;
}