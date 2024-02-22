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

#include <cheerp/clientlib.h>

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

using namespace client;

static bool isBrowser;
[[cheerp::genericjs]] HTMLElement *ledDisplay;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void
Led_MsgHandler(service_t *service, const msg_t *msg);

void clear_screen(void)
{
    // clear the console
    printf("\x1B[2J\x1B[H");
}

[[cheerp::genericjs]] void define_browser()
{
    __asm__("typeof %1 !== 'undefined'" : "=r"(isBrowser) : "r"(&client::window));
}

[[cheerp::genericjs]] void browser_init()
{
    client::HTMLElement *body = client::document.get_body();
    HTMLElement *board        = document.createElement("div");
    board->setAttribute("style", "width: 200px; height: 100px; background-color: #449944; display: flex; border-radius: 5%; border: 2px solid #000000;");
    body->appendChild(board);
    HTMLElement *chip = document.createElement("div");
    chip->setAttribute("style", "margin: 25px; width: 50px; height: 50px; background-color: #101010; border-radius: 10%; color: #FFFFFF; text-align: center; line-height: 50px; font-size: 15px; font-weight: bold;");
    chip->appendChild(document.createTextNode("MCU"));
    board->appendChild(chip);
    ledDisplay = document.createElement("div");
    ledDisplay->setAttribute("style", "margin: 30px; width: 30px; height: 30px; background-color: #F0F0F0; border-radius: 50%");
    board->appendChild(ledDisplay);
}

[[cheerp::genericjs]] void browser_display(bool state)
{
    if (state)
    {
        ledDisplay->setAttribute("style", "margin: 30px; width: 30px; height: 30px; background-color: #00ff00; border-radius: 50%");
    }
    else
    {

        ledDisplay->setAttribute("style", "margin: 30px; width: 30px; height: 30px; background-color: #F0F0F0; border-radius: 50%");
    }
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
PUBLIC void Led_Init(void)
{
    define_browser();
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Led_MsgHandler, STATE_TYPE, "led", revision);
    clear_screen();
    printf("LED service running.\n\n");
    if (isBrowser)
    {
        browser_init();
        browser_display(false);
    }
    else
    {
        printf(led_OFF);
    }
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
PUBLIC void Led_Loop(void) {}

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
                if (isBrowser)
                {
                    browser_display(false);
                }
                else
                {
                    printf(led_OFF);
                }
            }
            else if (Led_last_state == LED_ON)
            {
                if (isBrowser)
                {
                    browser_display(true);
                }
                else
                {
                    printf(led_ON);
                }
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
