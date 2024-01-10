/******************************************************************************
 * @file Led
 * @brief driver example a simple Led
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
#endif
#include "led.h"
#include <stdlib.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t Led_last_state = 0;
const char led_ON[768] = "                                   \\ | /\n"
                         "         ___________________________.-.________________\n"
                         "        /                        - |***| -            /;\n"
                         "       /    ________________     - |***| -           //\n"
                         "      /    /               /;     [` - ')           //\n"
                         "     /    /     MCU       //       `---'           //\n"
                         "    /    /               //         | |           //\n"
                         "   /    /______________ //                       //\n"
                         "  /     '---------------'                       //\n"
                         " /                                             //\n"
                         "/_____________________________________________//\n"
                         "`---------------------------------------------'\n";

const char led_OFF[768] = "\n"
                          "         ___________________________.-.________________\n"
                          "        /                          |   |              /;\n"
                          "       /    ________________       |   |             //\n"
                          "      /    /               /;     [` - ')           //\n"
                          "     /    /     MCU       //       `---'           //\n"
                          "    /    /               //         | |           //\n"
                          "   /    /______________ //                       //\n"
                          "  /     '---------------'                       //\n"
                          " /                                             //\n"
                          "/_____________________________________________//\n"
                          "`---------------------------------------------'\n";

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg);

void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    // Assume POSIX
    system("clear");
#endif
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Led_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led", revision);
    clear_screen();
    printf("LED service running.\n\n");
    printf(led_OFF);
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Led_Loop(void) {}

/******************************************************************************
 * @brief Msg manager callback when a msg receive for this service
 * @param service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Led_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        if (msg->data[0] != Led_last_state)
        {
            Led_last_state = msg->data[0];
            clear_screen();
            printf("LED service running.\n\n");
            if (Led_last_state == LED_OFF)
            {
                printf(led_OFF);
            }
            else if (Led_last_state == LED_ON)
            {
                printf(led_ON);
            }
            else
            {
                printf("[UNKWNOWN STATE] LED can only be ON or OFF...\n");
            }
        }
        else if (msg->data[0] == LED_ON)
        {

            clear_screen();
            printf("LED service running. => LED is already ON\n\n");
            printf(led_ON);
        }
        else if (msg->data[0] == LED_OFF)
        {
            clear_screen();
            printf("LED service running. => LED is already OFF\n\n");
            printf(led_OFF);
        }
    }
}
