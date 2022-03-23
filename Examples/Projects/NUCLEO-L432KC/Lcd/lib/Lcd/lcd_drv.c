/******************************************************************************
 * @file lcd_drv
 * @brief This is a driver example for an LCD display. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/L432KC-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#include "lcd_drv.h"
#include "stm32l4xx_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t display_function;
uint8_t display_control;
uint8_t display_mode;

/*******************************************************************************
 * Functions
 ******************************************************************************/
void LcdDrv_DelayMs(int time);
void LcdDrv_PulseEnable(void);
void LcdDrv_HWInit(void);
void LcdDrv_Write(uint8_t value);
void LcdDrv_SendCommand(uint8_t value);

/******************************************************************************
 * @brief Driver init must be call in service init
 * @param None
 * @return None
 ******************************************************************************/
void LcdDrv_Init(void)
{
    data_pins[0]      = DATA1_Pin;
    data_gpio_port[0] = (int)DATA1_GPIO_Port;

    data_pins[1]      = DATA2_Pin;
    data_gpio_port[1] = (int)DATA2_GPIO_Port;

    data_pins[2]      = DATA3_Pin;
    data_gpio_port[2] = (int)DATA3_GPIO_Port;

    data_pins[3]      = DATA4_Pin;
    data_gpio_port[3] = (int)DATA4_GPIO_Port;

    if (!FOURBITMODE)
    {
        data_pins[4]      = DATA5_Pin;
        data_gpio_port[4] = (int)DATA5_GPIO_Port;

        data_pins[5]      = DATA6_Pin;
        data_gpio_port[5] = (int)DATA6_GPIO_Port;

        data_pins[6]      = DATA7_Pin;
        data_gpio_port[6] = (int)DATA7_GPIO_Port;

        data_pins[7]      = DATA8_Pin;
        data_gpio_port[7] = (int)DATA8_GPIO_Port;

        display_function = (N_ROWS > 1) ? LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS : LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
    }
    else
    {
        display_function = (N_ROWS > 1) ? LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS : LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    }

    LcdDrv_HWInit();
}

/******************************************************************************
 * @brief Hardware init must be call in driver init
 * @param None
 * @return None
 ******************************************************************************/
void LcdDrv_HWInit(void)
{
    LcdDrv_DelayMs(50);
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    LCD_PORTS_CLK_ENABLE();

    GPIO_InitStruct.Pin   = EN_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(EN_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = RS_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RS_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);

    if (RW_Pin != 255)
    {
        GPIO_InitStruct.Pin = RW_Pin;
        HAL_GPIO_Init(RW_GPIO_Port, &GPIO_InitStruct);

        HAL_GPIO_WritePin(RW_GPIO_Port, RW_Pin, GPIO_PIN_RESET);
    }

    for (int i = 0; i < (8 - 4 * FOURBITMODE); i++)
    {
        GPIO_InitStruct.Pin = data_pins[i];
        HAL_GPIO_Init(data_gpio_port[i], &GPIO_InitStruct);
    }

    // this is according to the Hitachi HD44780 datasheet
    // figure 24, pg 46
    if (!(display_function & LCD_8BITMODE))
    {
        // we start in 8bit mode, try to set 4 bit mode
        LcdDrv_Write(0x03);
        LcdDrv_DelayMs(3); // wait min 4.1ms

        // second try
        LcdDrv_Write(0x03);
        LcdDrv_DelayMs(3); // wait min 4.1ms

        // third go!
        LcdDrv_Write(0x03);

        // finally, set to 4-bit interface
        LcdDrv_Write(0x02);
    }
    else
    {
        LcdDrv_SendCommand(LCD_FUNCTIONSET | display_function);
        LcdDrv_DelayMs(3);

        LcdDrv_SendCommand(LCD_FUNCTIONSET | display_function);

        LcdDrv_SendCommand(LCD_FUNCTIONSET | display_function);
    }

    // finally, set # lines, font size, etc.
    LcdDrv_SendCommand(LCD_FUNCTIONSET | display_function);

    // turn the display on with no cursor or blinking default
    display_control = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    LcdDrv_SendCommand(LCD_DISPLAYCONTROL | display_function);

    // clear it off
    LcdDrv_SendCommand(LCD_CLEARDISPLAY);

    // Initialize to default text direction (for romance languages)
    display_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    LcdDrv_SendCommand(LCD_ENTRYMODESET | display_mode);

    LcdDrv_SetParameter(LCD_DISPLAY, 1);
}

/******************************************************************************
 * @brief To display text on lcd screen
 * @param text
 * @param length
 * @return None
 ******************************************************************************/
void LcdDrv_Print(char *text, int length)
{
    LcdDrv_SendCommand(LCD_CLEARDISPLAY);
    uint8_t letter;

    for (int i = 0; i < length; i++)
    {
        letter = (uint8_t)text[i];
        HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_SET);

        if (FOURBITMODE)
            LcdDrv_Write(letter >> 4);

        LcdDrv_Write(letter);
        HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);

        LcdDrv_DelayMs(1);
    }
}

/******************************************************************************
 * @brief Must be call to set parameters
 * @param param
 * @param value
 * @return None
 ******************************************************************************/
void LcdDrv_SetParameter(uint8_t param, uint8_t value)
{
    switch (param)
    {
        case LCD_DISPLAY:
            if (value)
                display_control |= LCD_DISPLAYON;
            else
                display_control &= ~LCD_DISPLAYON;

            LcdDrv_SendCommand(LCD_DISPLAYCONTROL | display_control);
            break;

        case LCD_CURSOR:
            if (value)
                display_control |= LCD_CURSORON;
            else
                display_control &= ~LCD_CURSORON;

            LcdDrv_SendCommand(LCD_DISPLAYCONTROL | display_control);
            break;

        case LCD_BLINK:
            if (value)
                display_control |= LCD_BLINKON;
            else
                display_control &= ~LCD_BLINKON;

            LcdDrv_SendCommand(LCD_DISPLAYCONTROL | display_control);
            break;

        case LCD_AUTOSCROLL:
            if (value)
                display_mode |= LCD_ENTRYSHIFTINCREMENT;
            else
                display_mode &= ~LCD_ENTRYSHIFTINCREMENT;

            LcdDrv_SendCommand(LCD_ENTRYMODESET | display_mode);
            break;

        case LCD_SCROLL_DISPLAY_RIGHT:
            (value) ? LcdDrv_SendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT) : LcdDrv_SendCommand(LCD_CLEARDISPLAY);
            LcdDrv_DelayMs(LCD_SCROLL_DELAY);
            break;

        case LCD_SCROLL_DISPLAY_LEFT:
            (value) ? LcdDrv_SendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT) : LcdDrv_SendCommand(LCD_CLEARDISPLAY);
            LcdDrv_DelayMs(LCD_SCROLL_DELAY);
            break;

        case LCD_RIGHT_TO_LEFT:
            if (value)
                display_mode &= ~LCD_ENTRYLEFT;
            else
                display_mode |= LCD_ENTRYLEFT;

            LcdDrv_SendCommand(LCD_ENTRYMODESET | display_mode);
            break;

        default:
            break;
    }
}

/******************************************************************************
 * @brief Write command or data with automatic 4/8 bit selection
 * @param value to send
 * @return None
 ******************************************************************************/
void LcdDrv_Write(uint8_t value)
{
    if (RW_Pin != 255)
        HAL_GPIO_WritePin(RW_GPIO_Port, RW_Pin, GPIO_PIN_RESET);

    for (int i = 0; i < (8 - 4 * FOURBITMODE); i++)
    {
        if (value >> i & 0x01)
            HAL_GPIO_WritePin(data_gpio_port[i], data_pins[i], GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(data_gpio_port[i], data_pins[i], GPIO_PIN_RESET);
    }

    LcdDrv_PulseEnable();
}

/******************************************************************************
 * @brief Pulse on enable pin
 * @param None
 * @return None
 ******************************************************************************/
void LcdDrv_PulseEnable(void)
{
    LcdDrv_DelayMs(1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);

    LcdDrv_DelayMs(1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);

    LcdDrv_DelayMs(3);
}

/******************************************************************************
 * @brief Send command to the lcd with automatic 4/8 bit selection
 * @param value
 * @return None
 ******************************************************************************/
void LcdDrv_SendCommand(uint8_t value)
{
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    if (FOURBITMODE)
        LcdDrv_Write(value >> 4);
    LcdDrv_Write(value);
}

/******************************************************************************
 * @brief Delay in milliseconds
 * @param time
 * @return None
 ******************************************************************************/
void LcdDrv_DelayMs(int delay)
{
    volatile int t = RobusHAL_GetSystick();
    while ((RobusHAL_GetSystick() - t) < delay)
        ;
}