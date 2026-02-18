/******************************************************************************
 * @file laser
 * @brief driver example a laser
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef LASER_H
#define LASER_H

#include "luos_engine.h"
#include "main.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PWM_PIN_CLK()                 \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0U)

#define PWM_TIMER_CLK()               \
    do                                \
    {                                 \
        __HAL_RCC_TIM15_CLK_ENABLE(); \
    } while (0U)

#define PWM_PERIOD 5000 - 1

#define LASER_PWM_PIN  GPIO_PIN_15
#define LASER_PWM_PORT GPIOB
#define LASER_PWM_AF   GPIO_AF14_TIM15

#ifndef LASER_PWM_TIMER
    #define LASER_PWM_TIMER TIM15
#endif
#define LASER_PWM_CHANNEL TIM_CHANNEL_2

#ifndef DEFAULT_SAMPLE_FREQUENCY
    #define DEFAULT_SAMPLE_FREQUENCY 10000.0
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Laser_Init(void);
void Laser_Loop(void);

#endif /* LASER_H */
