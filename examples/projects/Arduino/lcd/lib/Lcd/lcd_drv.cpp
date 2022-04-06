/******************************************************************************
 * @file lcd_drv
 * @brief This is a driver example for an LCD display. It will work as is
 * but if you want to see it inside of a project, I am using it to create a 
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/Arduino-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include <LiquidCrystal.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "lcd_drv.h"

#ifdef __cplusplus
}
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#ifdef RW_PIN
#ifdef D4_PIN &&D5_PIN &&D6_PIN &&D7_PIN
LiquidCrystal mydisplay(RS_PIN, RW_PIN, EN_PIN, D0_PIN, D1_PIN, D2_PIN, D3_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);
#else
LiquidCrystal mydisplay(RS_PIN, EN_PIN, D0_PIN, D1_PIN, D2_PIN, D3_PIN);
#endif
#else
#ifdef D7_PIN
LiquidCrystal mydisplay(RS_PIN, EN_PIN, D0_PIN, D1_PIN, D2_PIN, D3_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);
#else
LiquidCrystal mydisplay(RS_PIN, EN_PIN, D0_PIN, D1_PIN, D2_PIN, D3_PIN);
#endif
#endif

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief hardware init must be call in service init
 * @param None
 * @return None
 ******************************************************************************/
void LcdDrv_Init(void)
{
    mydisplay.begin(N_COLS, N_ROWS);
    LcdDrv_SetParameter(LCD_DISPLAY, true);

    for (int i = 1; i < LCD_RIGHT_TO_LEFT; i++)
    {
        LcdDrv_SetParameter(i, false);
    }
}

/******************************************************************************
 * @brief print to display text on screen
 * @param text
 * @param size
 * @return None
 ******************************************************************************/
void LcdDrv_Print(char *text, uint16_t size)
{
    mydisplay.clear();
    mydisplay.print(text);
}

/******************************************************************************
 * @brief command must be call to set parameters of lcd
 * @param mode
 * @param value
 * @return None
 ******************************************************************************/
void LcdDrv_SetParameter(uint8_t mode, uint8_t value)
{
    switch (mode)
    {
        case LCD_DISPLAY:
            (value) ? mydisplay.display() : mydisplay.noDisplay();
            break;

        case LCD_CURSOR:
            (value) ? mydisplay.cursor() : mydisplay.noCursor();
            break;

        case LCD_BLINK:
            (value) ? mydisplay.blink() : mydisplay.noBlink();
            break;

        case LCD_AUTOSCROLL:
            (value) ? mydisplay.autoscroll() : mydisplay.noAutoscroll();
            break;

        case LCD_SCROLL_DISPLAY_RIGHT:
            (value) ? mydisplay.scrollDisplayRight() : mydisplay.clear();
            delay(LCD_SCROLL_DELAY);
            break;

        case LCD_SCROLL_DISPLAY_LEFT:
            (value) ? mydisplay.scrollDisplayLeft() : mydisplay.clear();
            delay(LCD_SCROLL_DELAY);
            break;

        case LCD_RIGHT_TO_LEFT:
            (value) ? mydisplay.rightToLeft() : mydisplay.leftToRight();
            break;

        default:
            break;
    }
}
