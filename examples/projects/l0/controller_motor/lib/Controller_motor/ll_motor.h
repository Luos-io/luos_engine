/******************************************************************************
 * @file low level motor functions
 * @brief driver example a simple controller motor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LL_MOTOR_H
#define _LL_MOTOR_H

#include "luos_engine.h"
#include "stdio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// Pin configuration
#define FB_Pin                 GPIO_PIN_0
#define FB_GPIO_Port           GPIOB
#define B_Pin                  GPIO_PIN_0
#define B_GPIO_Port            GPIOA
#define A_Pin                  GPIO_PIN_1
#define A_GPIO_Port            GPIOA
#define POWER_SENSOR_Pin       GPIO_PIN_2
#define POWER_SENSOR_GPIO_Port GPIOA
#define FB_Pin                 GPIO_PIN_0
#define FB_GPIO_Port           GPIOB
#define EN_Pin                 GPIO_PIN_1
#define EN_GPIO_Port           GPIOB
#define PWM2_Pin               GPIO_PIN_4
#define PWM2_GPIO_Port         GPIOB
#define PWM1_Pin               GPIO_PIN_5
#define PWM1_GPIO_Port         GPIOB
#define POSITION_TIMER         TIM2
#define COMMAND_TIMER          TIM3

enum
{
    MOTOR_DISABLE,
    MOTOR_ENABLE
};

typedef struct motor_config
{
    // motor configuration
    float *motor_reduction;
    float *resolution;
    linear_position_t *wheel_diameter;

    // hardware limits
    ratio_t *limit_ratio;
    current_t *limit_current;
} motor_config_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void ll_motor_init(void);
void ll_motor_enable(char);
void ll_motor_Command(uint16_t, float);
float ll_motor_GetCurrent(void);
float ll_motor_GetAngularPosition(void);
float ll_motor_GetLinearPosition(float);
void ll_motor_config(motor_config_t);

#endif /* _LL_MOTOR_H */
