#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "luos_engine.h"
#include "robus.h"
#include "game_anim.h"
#include "scoring.h"

static void score_display(void);

const char start[768]
    = "                          ((((.       \n"
      "                     ,(((((((((((((   \n"
      "          (#########((((((((((((((((( \n"
      "        ###########(((((((((((((((((((\n"
      "      ,###########((((((((((((((((((((\n"
      "      #############(((((((((((((((((((\n"
      "      ,,*###########((((((((((((((((( \n"
      "       ,,,,###########,,,,,,,,,,,,.   \n"
      "      ***,,,,*#########  ,*****       \n"
      "   *******  .,,,###       ****.       \n"
      " ******            *//    ****,       \n"
      "  ***             *////   *****       \n"
      "                    ''      .         \n";

SCREEN view        = start_view;
bool need_update   = false;
ball_t *ball_state = NULL;

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
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
        printf("\tThe Luos ping pong world cup!\n");
        printf("%s", start);
        printf("Controls:\n");
        printf("\t - If the ball is going left press '←'\n");
        printf("\t - If the ball is going right press '→'\n");
        printf("\tPress SPACE BAR to start!\n");
    }
}

void alone_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("\tThe Luos ping pong world cup!\n");
        printf("%s", start);
        printf("You don't have any friends?\n\nYou should find some on the Luos discord :\n\thttps://discord.gg/luos\n\n");
        printf("Controls:\n");
        printf("\t - If the ball is going left press '←'\n");
        printf("\t - If the ball is going right press '→'\n");
        printf("\tPress SPACE BAR to start!\n");
    }
}

void service_view(void)
{
    if (need_update)
    {
        clear_screen();
        printf("%s\n", table[EMPTY][1]);
        score_display();
        printf("\n\tPress SPACE BAR to serve\n");
        need_update = false;
    }
}

void gameOver_view(void)
{
    if (need_update)
    {
        clear_screen();
        printf("GAME_OVER\n");
        score_display();
        printf("\n\tPress SPACE BAR to restart\a\n");
        need_update = false;
    }
}

void game_view(void)
{
    static int animation_frame = 0;
    if (need_update)
    {
        clear_screen();
        animation_frame = 0;
        printf("%s\n", table[*ball_state][animation_frame]);
        score_display();
        need_update = false;
    }
    if (animation_frame < GAME_ANIMATION_FRAME_NB)
    {
        clear_screen();
        printf("%s\n", table[*ball_state][animation_frame]);
        score_display();
        animation_frame++;
        msleep(10);
    }
}

void wait_view(void)
{
    if (need_update)
    {
        need_update = false;
        clear_screen();
        printf("\tThe Luos ping pong world cup!\n");
        printf("%s", start);
        printf("Game is starting, please wait...\n");
    }
}

void score_display(void)
{
    score_table_t *score = get_score();
    for (int i = 0; i < score->player_nb; i++)
    {
        printf("\t%d |%16s |\t %d\n", i + 1, score->scores[i].alias, score->scores[i].score);
    }
}

void score_view(void)
{
    if (need_update)
    {
        clear_screen();
        printf("%s\n", table[EMPTY][1]);
        score_display();
        printf("\n\tSomeone just lost...\n");
        need_update = false;
    }
}
