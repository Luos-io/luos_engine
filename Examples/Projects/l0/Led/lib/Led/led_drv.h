/******************************************************************************
 * @file led_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef LED_DRV_H
#define LED_DRV_H

#include "luos.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_system.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
// TIM2 Definitions
#define LED_TIM2_CLK()    __HAL_RCC_TIM2_CLK_ENABLE();
#define LED_TIM2_INSTANCE TIM2
#define LED_TIM2_CHANNEL  TIM_CHANNEL_1

#define LED_TIM2_GPIO_CLK() __HAL_RCC_GPIOA_CLK_ENABLE()
#define LED_TIM2_PORT       GPIOA
#define LED_TIM2_CH_PIN     GPIO_PIN_0
#define LED_AF_TIM2         GPIO_AF2_TIM2

// TIM3 Definitions
#define LED_TIM3_CLK()    __HAL_RCC_TIM3_CLK_ENABLE();
#define LED_TIM3_INSTANCE TIM3
#define LED_TIM3_CHANNEL1 TIM_CHANNEL_1
#define LED_TIM3_CHANNEL2 TIM_CHANNEL_2

#define LED_TIM3_GPIO_CLK() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED_TIM3_PORT       GPIOB
#define LED_TIM3_CH1_PIN    GPIO_PIN_4
#define LED_TIM3_CH2_PIN    GPIO_PIN_5
#define LED_AF_TIM3         GPIO_AF1_TIM3

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void LedDrv_Init();
void LedDrv_Write(color_t *rgb);

#endif /* LED_DRV_H */