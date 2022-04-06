/******************************************************************************
 * @file lcd_drv
 * @brief This is a driver example for an LCD display. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/STM32F0-Luos-Biometric-Security-System
 * @author mariebidouille
 * @version 0.0.0
 ******************************************************************************/
#ifndef LCD_DRV_H
#define LCD_DRV_H

#include "luos_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LCD_PORTS_CLK_ENABLE()        \
    do                                \
    {                                 \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0U)

// Essential pins
#define RS_Pin       GPIO_PIN_1
#define RS_GPIO_Port GPIOA

#define EN_Pin       GPIO_PIN_0
#define EN_GPIO_Port GPIOB

#define DATA1_Pin       GPIO_PIN_3
#define DATA1_GPIO_Port GPIOB

#define DATA2_Pin       GPIO_PIN_4
#define DATA2_GPIO_Port GPIOB

#define DATA3_Pin       GPIO_PIN_5
#define DATA3_GPIO_Port GPIOB

#define DATA4_Pin       GPIO_PIN_0
#define DATA4_GPIO_Port GPIOA

// 1 for 4 data pins, 0 for 8 data pins
#define FOURBITMODE 1

// Additionnal pins
#define DATA5_Pin       GPIO_PIN_3
#define DATA5_GPIO_Port GPIOB

#define DATA6_Pin       GPIO_PIN_4
#define DATA6_GPIO_Port GPIOB

#define DATA7_Pin       GPIO_PIN_5
#define DATA7_GPIO_Port GPIOB

#define DATA8_Pin       GPIO_PIN_0
#define DATA8_GPIO_Port GPIOA

#define RW_Pin       255
#define RW_GPIO_Port 0

// Lcd configuration
#define N_ROWS        2
#define N_COLS        16
#define MAX_TEXT_SIZE 2 * N_COLS

#define LCD_SCROLL_DELAY 100

// Commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// Flags for display entry mode
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Flags for display on/off control
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON   0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON    0x01
#define LCD_BLINKOFF   0x00

// Flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT   0x04
#define LCD_MOVELEFT    0x00

// Flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE    0x08
#define LCD_1LINE    0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS  0x00

/*******************************************************************************
 * Variables
 ******************************************************************************/
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

uint16_t data_pins[8];
uint32_t data_gpio_port[8];

/*******************************************************************************
 * Functions
 ******************************************************************************/
void LcdDrv_Init(void);
void LcdDrv_Print(char *text, int length);
void LcdDrv_SetParameter(uint8_t mode, uint8_t value);

#endif /* LCD_DRV_H */