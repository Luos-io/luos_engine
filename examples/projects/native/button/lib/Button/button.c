/******************************************************************************
 * @file button
 * @brief driver example a simple button
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
#include <fcntl.h>
#include "button.h"
#include "profile_state.h"
#include <stdlib.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef _WIN32
    #define get_character() getch()
#else
    #define get_character() getchar()
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
profile_state_t button;

const char button_ON[768] = "         ______________________________________________\n"
                            "        /                                             /;\n"
                            "       /    ________________        \\ | /            //\n"
                            "      /    /               /;        .-.            //\n"
                            "     /    /     MCU       //        |`-'|          //\n"
                            "    /    /               //        [ `-' )        //\n"
                            "   /    /______________ //          `---'        //\n"
                            "  /     '---------------'                       //\n"
                            " /                                             //\n"
                            "/_____________________________________________//\n"
                            "'---------------------------------------------'\n";

const char button_OFF[768] = "         ______________________________________________\n"
                             "        /                                             /;\n"
                             "       /    ________________         .-.             //\n"
                             "      /    /               /;       |`-'|           //\n"
                             "     /    /     MCU       //        |   |          //\n"
                             "    /    /               //        [ `-' )        //\n"
                             "   /    /______________ //          `---'        //\n"
                             "  /     '---------------'                       //\n"
                             " /                                             //\n"
                             "/_____________________________________________//\n"
                             "'---------------------------------------------'\n";

/*******************************************************************************
 * Function
 ******************************************************************************/

void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    // Assume POSIX
    int systemRet;
    systemRet = system("clear");
#endif
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Button_Init(void)
{
    // service initialization
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Profile configuration
    button.access = READ_ONLY_ACCESS;
    // Service creation following state profile
    ProfileState_CreateService(&button, 0, "button", revision);
    // set_fl(STDOUT_FILENO, O_NONBLOCK);
    clear_screen();
    printf("Button service running. Press Q to quit.\n\n\tUse the SPACE button to swap the button state.\n");
    printf("%s", button_OFF);
}
#ifndef _WIN32
int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Button_Loop(void)
{
    static bool stop_loop = false;
    int c;
    if ((kbhit()) && (!stop_loop))
    {
        c = get_character();
        if (c == ' ')
        {
            clear_screen();
            printf("Button service is running. Press Q to quit.\n\n\tUse the SPACE button to swap the button state.\n");
            button.state = !button.state;
            if (button.state)
            {
                printf("%s", button_ON);
            }
            else
            {
                printf("%s", button_OFF);
            }
        }
        else if ((c == 'q') || (c == 'Q'))
        {
            clear_screen();
            printf("\n\n--- Quit button service ---\n(press \"CTRL + C\" to close program)\n\n");
            stop_loop = true;
        }
    }
}
