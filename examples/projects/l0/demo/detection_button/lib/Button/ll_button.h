/******************************************************************************
 * @file low-level button
 * @brief driver example a simple button
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef LL_BUTTON_H
#define LL_BUTTON_H

#include "stdbool.h"
#include "stdio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BTN_Pin       GPIO_PIN_0
#define BTN_GPIO_Port GPIOB
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void ll_button_init(void);
uint8_t ll_button_read(bool *);

#endif /* LL_BUTTON_H */
