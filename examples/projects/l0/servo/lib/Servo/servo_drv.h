/******************************************************************************
 * @file servo
 * @brief driver example a simple servo motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef SERVO_DRV_H
#define SERVO_DRV_H

#include "stm32f0xx_hal.h"
#include "luos_engine.h"
#include "robus.h"
#include "luos_hal.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * Servo
 */
typedef struct
{
    union
    {
        struct __attribute__((__packed__))
        {
            angular_position_t max_angle;
            float min_pulse_time;
            float max_pulse_time;
        };
        unsigned char unmap[3 * sizeof(float)];
    };
} servo_parameters_t;

typedef struct
{
    TIM_TypeDef *Timer;
    uint32_t Channel;
} servo_control_t;

typedef struct
{
    angular_position_t angle;
    servo_parameters_t param;
    servo_control_t control;
} servo_t;

// HW definition
#define SERVONUMBER 4

#define DEFAULT_PRESACALER 60

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
    } while (0U)

#define S1_PIN     GPIO_PIN_0
#define S1_PORT    GPIOA
#define S1_AF      GPIO_AF2_TIM2
#define S1_TIMER   TIM2
#define S1_CHANNEL LL_TIM_CHANNEL_CH1

#define S2_PIN     GPIO_PIN_3
#define S2_PORT    GPIOB
#define S2_AF      GPIO_AF2_TIM2
#define S2_TIMER   TIM2
#define S2_CHANNEL LL_TIM_CHANNEL_CH2

#define S3_PIN     GPIO_PIN_10
#define S3_PORT    GPIOB
#define S3_AF      GPIO_AF2_TIM2
#define S3_TIMER   TIM2
#define S3_CHANNEL LL_TIM_CHANNEL_CH3

#define S4_PIN     GPIO_PIN_11
#define S4_PORT    GPIOB
#define S4_AF      GPIO_AF2_TIM2
#define S4_TIMER   TIM2
#define S4_CHANNEL LL_TIM_CHANNEL_CH4
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Servo_DRVInit(void);
uint8_t Servo_DRVSetPosition(angular_position_t angle, uint8_t motor_id);
uint8_t Servo_DRVParameter(servo_parameters_t param, uint8_t MotorId);

#endif /* SERVO_DRV_H */
