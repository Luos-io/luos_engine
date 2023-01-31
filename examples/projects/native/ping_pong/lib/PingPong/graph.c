#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

const char table[3][768]
    = {"         _____________________________________________\n"
       "        /                      /                      /;\n"
       "       /                      /                      //\n"
       "      /                      /                      //\n"
       "     /                      /                      //\n"
       "    /                      /                      //\n"
       "   /                      /                      //\n"
       "  /                      /                      //\n"
       " /                      /                      //\n"
       "/______________________/______________________//\n"
       "'---------------------------------------------'\n",
       "         _____________________________________________\n"
       "        /                      /                      /;\n"
       "       /                      /                      //\n"
       "      /                      /                      //\n"
       "     /      ,---.           /                      //\n"
       "    /      [     )         /                      //\n"
       "   /        `---'         /                      //\n"
       "  /                      /                      //\n"
       " /                      /                      //\n"
       "/______________________/______________________//\n"
       "'---------------------------------------------'\n",
       "         _____________________________________________\n"
       "        /                      /                      /;\n"
       "       /                      /                      //\n"
       "      /                      /                      //\n"
       "     /                      /     ,---.            //\n"
       "    /                      /     [     )          //\n"
       "   /                      /       `---'          //\n"
       "  /                      /                      //\n"
       " /                      /                      //\n"
       "/______________________/______________________//\n"
       "'---------------------------------------------'\n"};

SCREEN view        = start_view;
bool need_update   = false;
ball_t *ball_state = NULL;

/* msleep(): Sleep for the requested number of milliseconds. */
static int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec  = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

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

void create_ball(ball_t *ballpt)
{
    ball_state = ballpt;
}

void *Graph_LoopThread(void *vargp)
{
    while (1)
    {
        view();
        msleep(10);
    }
    return NULL;
}

void set_screen_to(SCREEN screen)
{
    view        = screen;
    need_update = true;
}

//******************* screen view *****************

void start_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("%s", table[EMPTY]);
        printf("THE LUOS PING PONG GAME\n\tPress SPACE BAR to start!\n");
    }
}

void alone_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("%s", table[EMPTY]);
        printf("You are alone, go get a friend!\n");
        printf("THE LUOS PING PONG GAME\n\tPress SPACE BAR to start!\n");
    }
}

void service_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("%s\n", table[EMPTY]);
        printf("Press SPACE BAR to serve\n");
    }
}

void gameOver_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("GAME_OVER\n");
        printf("Press SPACE BAR to restart\a\n");
    }
}

void game_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("%s\n", table[*ball_state]);
        printf("Game is running\a\n");
    }
}

void wait_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("%s\n", table[EMPTY]);
        printf("Game is starting, please wait...\n");
    }
}