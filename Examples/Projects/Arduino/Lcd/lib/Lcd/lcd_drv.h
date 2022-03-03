/******************************************************************************
 * @file lcd_drv
 * @brief This is a driver example for an LCD display. It will work as is
 * but if you want to see it inside of a project, I am using it to create a 
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/Arduino-Luos-Biometric-Security-System
 * @version 0.0.0
 ******************************************************************************/
#ifndef LCD_DRV_H
#define LCD_DRV_H

#include "luos_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RS_PIN 13
#define EN_PIN 12

#define D0_PIN 11
#define D1_PIN 10
#define D2_PIN 9
#define D3_PIN 8

#define N_ROWS        2
#define N_COLS        16
#define MAX_TEXT_SIZE 2 * N_COLS

//scroll delay in ms
#define LCD_SCROLL_DELAY 100

enum
{
    LCD_DISPLAY,
    LCD_CURSOR,
    LCD_BLINK,
    LCD_AUTOSCROLL,
    LCD_SCROLL_DISPLAY_RIGHT,
    LCD_SCROLL_DISPLAY_LEFT,
    LCD_RIGHT_TO_LEFT
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/
void LcdDrv_Init(void);
void LcdDrv_Print(char *text, uint16_t size);
void LcdDrv_SetParameter(uint8_t mode, uint8_t value);

#endif /*LCD_DRV_H*/