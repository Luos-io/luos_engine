/******************************************************************************
 * @file low-level gpio
 * @brief driver example a simple gpio_dev
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef LL_GPIO_H
#define LL_GPIO_H

#include "stm32f0xx_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// Pin configuration
#define P9_Pin       GPIO_PIN_1
#define P9_GPIO_Port GPIOA
#define P8_Pin       GPIO_PIN_0
#define P8_GPIO_Port GPIOB
#define P7_Pin       GPIO_PIN_1
#define P7_GPIO_Port GPIOB
#define P6_Pin       GPIO_PIN_10
#define P6_GPIO_Port GPIOB
#define P5_Pin       GPIO_PIN_11
#define P5_GPIO_Port GPIOB
#define P4_Pin       GPIO_PIN_3
#define P4_GPIO_Port GPIOB
#define P3_Pin       GPIO_PIN_4
#define P3_GPIO_Port GPIOB
#define P2_Pin       GPIO_PIN_5
#define P2_GPIO_Port GPIOB
#define P1_Pin       GPIO_PIN_0
#define P1_GPIO_Port GPIOA

enum
{
    P2,
    P3,
    P4,
    P5,
    P6,
    GPIO_NB
} gpio_enum_t;

enum
{
    P1,
    P7,
    P8,
    P9,
    ANALOG_NB
} analog_enum_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void ll_analog_init(void);
void ll_digital_init(void);

uint8_t ll_analog_read(void *);
uint8_t ll_digital_read(void *);

uint8_t ll_digital_write(void *);

#endif /* LL_GPIO_H */