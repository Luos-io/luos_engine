/******************************************************************************
 * @file lcd
 * @brief This is a driver example for an LCD display. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/L432KC-Luos-Biometric-Security-System
 * @author MarieBidouille
 * @version 0.0.0
 ******************************************************************************/
#include "lcd.h"
#include "lcd_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    LCD_TYPE = LUOS_LAST_TYPE
};

/*******************************************************************************
 * Variables
 ******************************************************************************/
lcd_mode_t mode;
static service_t *service_lcd;
uint8_t i = 0;
/*******************************************************************************
 * Functions
 ******************************************************************************/
void Lcd_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief Service init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Lcd_Init(void)
{
    revision_t revision = {{0, 0, 0}};
    service_lcd         = Luos_CreateService(Lcd_MsgHandler, LCD_TYPE, "lcd", revision);

    LcdDrv_Init();

    mode.mode_display        = 1;
    mode.mode_cursor         = 0;
    mode.mode_blink          = 0;
    mode.mode_autoscroll     = 0;
    mode.mode_scroll_display = 0;
    mode.mode_right_to_left  = 0;
}

/******************************************************************************
 * @brief Service loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Lcd_Loop(void)
{
    if (mode.mode_scroll_display)
    {
        if (mode.mode_right_to_left)
        {
            LcdDrv_SetParameter(LCD_SCROLL_DISPLAY_LEFT, 1);
        }
        else
        {
            LcdDrv_SetParameter(LCD_SCROLL_DISPLAY_RIGHT, 1);
        }
    }
}

/******************************************************************************
 * @brief Handle messages
 * @param service
 * @param msg
 * @return None
 ******************************************************************************/
void Lcd_MsgHandler(service_t *service, const msg_t *msg)
{
    switch (msg->header.cmd)
    {
        case TEXT:
        {
            char text[msg->header.size];
            memcpy(&text, msg->data, msg->header.size);
            LcdDrv_Print(text, msg->header.size);
        }
        break;
        case REINIT:
        {
            LcdDrv_Init();
            mode.mode_display        = 1;
            mode.mode_cursor         = 0;
            mode.mode_blink          = 0;
            mode.mode_autoscroll     = 0;
            mode.mode_scroll_display = 0;
            mode.mode_right_to_left  = 0;
        }
        break;
        case PARAMETERS:
        {
            memcpy(&mode, msg->data, sizeof(lcd_mode_t));

            LcdDrv_SetParameter(LCD_DISPLAY, mode.mode_display);
            LcdDrv_SetParameter(LCD_CURSOR, mode.mode_cursor);
            LcdDrv_SetParameter(LCD_BLINK, mode.mode_blink);
            LcdDrv_SetParameter(LCD_AUTOSCROLL, mode.mode_autoscroll);
            LcdDrv_SetParameter(LCD_RIGHT_TO_LEFT, mode.mode_right_to_left);
        }
        break;
        default:
        {
            return;
        }
        break;
    }
}
