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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
profile_state_t button;

/*******************************************************************************
 * Function
 ******************************************************************************/

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
    printf("Button service running.\n\tUse the SPACE button to swap the button state.\n");
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
    if (kbhit())
    {
        if (getchar() == ' ')
        {
            printf("\nButton state : ");
            button.state = !button.state;
            if (button.state)
                printf("True");
            else
                printf("False");
        }
    }
}