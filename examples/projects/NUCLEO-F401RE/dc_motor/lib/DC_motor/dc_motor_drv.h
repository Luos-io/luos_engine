/******************************************************************************
 * @file dc_motor
 * @brief driver example a simple dc_motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef DC_MOTOR_DRV_H
#define DC_MOTOR_DRV_H

#include "luos_engine.h"
#include "robus.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
enum
{
    MOTOR_DC_1,
    MOTOR_DC_2,
    MOTOR_NUMBER,
};

#define PWM_PIN_CLK()                 \
    do                                \
    {                                 \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0U)

#define PWM_TIMER_CLK()              \
    do                               \
    {                                \
        __HAL_RCC_TIM2_CLK_ENABLE(); \
        __HAL_RCC_TIM3_CLK_ENABLE(); \
    } while (0U)

#define PWM_PERIOD 5000 - 1

#define SLEEP_PIN  GPIO_PIN_0
#define SLEEP_PORT GPIOB

#define PWM_1_PIN  GPIO_PIN_0
#define PWM_1_PORT GPIOA
#define PWM_1_AF   GPIO_AF1_TIM2

#define PWM_2_PIN  GPIO_PIN_3
#define PWM_2_PORT GPIOB
#define PWM_2_AF   GPIO_AF1_TIM2

#define PWM_1_TIMER   TIM2
#define PWM_1_CHANNEL LL_TIM_CHANNEL_CH1

#define PWM_2_TIMER   TIM2
#define PWM_2_CHANNEL LL_TIM_CHANNEL_CH2

#define PWM_3_PIN  GPIO_PIN_4
#define PWM_3_PORT GPIOB
#define PWM_3_AF   GPIO_AF2_TIM3

#define PWM_4_PIN  GPIO_PIN_5
#define PWM_4_PORT GPIOB
#define PWM_4_AF   GPIO_AF2_TIM3

#define PWM_3_TIMER   TIM3
#define PWM_3_CHANNEL LL_TIM_CHANNEL_CH1

#define PWM_4_TIMER   TIM3
#define PWM_4_CHANNEL LL_TIM_CHANNEL_CH2
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void DRV_DCMotorInit(void);
uint8_t DRV_DCMotorSetPower(uint8_t Motor, ratio_t power);
void DRV_DCMotorEnable(uint8_t enable);

#endif /* DC_MOTOR_DRV_H */
