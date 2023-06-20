/******************************************************************************
 * @file lcd
 * @brief This is a driver example for an LCD display. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/STM32F0-Luos-Biometric-Security-System
 * @author mariebidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef LCD_H
#define LCD_H

#include "luos_engine.h"
#include "robus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t mode_display : 1;
            uint8_t mode_cursor : 1;
            uint8_t mode_blink : 1;
            uint8_t mode_autoscroll : 1;
            uint8_t mode_scroll_display : 1;
            uint8_t mode_right_to_left : 1;
        };
        uint8_t unmap[2];
    };
} lcd_mode_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
void Lcd_Init(void);
void Lcd_Loop(void);

#endif /* LCD_H */
