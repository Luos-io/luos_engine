/******************************************************************************
 * @file ping_pong
 * @brief simple ping_pong Game
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
#include "ping_pong.h"
#include "profile_state.h"
#include <stdlib.h>
#include <time.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef _WIN32
    #define get_character() getch()
#else
    #define get_character() getchar()
#endif

#define BALL_TIMEOUT 2000

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *player;
#define PLAYER_TYPE LUOS_LAST_TYPE
bool get_service = false;

typedef enum
{
    EMPTY,
    LEFT,
    RIGHT
} ball_t;

#define BALL_POS_CMD LUOS_LAST_STD_CMD

ball_t ball = EMPTY;

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
bool initialized = false;

void Player_MsgHandler(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == BALL_POS_CMD)
    {
        ball = (ball_t)msg->data[0];
    }
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PingPong_Init(void)
{
    // service initialization
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    // Service creation

    player = Luos_CreateService(Player_MsgHandler, PLAYER_TYPE, "Player", revision);
    // set_fl(STDOUT_FILENO, O_NONBLOCK);
    clear_screen();
    printf("%s", table[EMPTY]);
    printf("THE LUOS PING PONG GAME\n\tPress SPACE BAR to start!\n");
    srand(time(NULL)); // Initialization, should only be called once.
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
/* Returns an integer in the range [0, n).
 *
 * Uses rand(), and so is affected-by/affects the same seed.
 */
int randint(int n)
{
    if ((n - 1) == RAND_MAX)
    {
        return rand();
    }
    else
    {

        // Chop off all of the values that would cause skew...
        int end
            = RAND_MAX / n; // truncate skew
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end)
            ;

        return r % n;
    }
}
void send_random()
{
    // Randomize ball position.
    uint8_t r = (rand() % 2) + 1; // Returns a pseudo-random integer between 0 and RAND_MAX.
    // Randomize target
    search_result_t target_list;
    RTFilter_Reset(&target_list);
    printf("found services:%d\n", target_list.result_nbr);
    RTFilter_Type(&target_list, PLAYER_TYPE);
    int target;
    do
    {
        target = randint(target_list.result_nbr);
    } while (target_list.result_table[target]->id == player->ll_service->id);

    // Our target is OK, Send the message
    msg_t msg;
    msg.header.target      = target_list.result_table[target]->id;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(r);
    msg.header.cmd         = BALL_POS_CMD;
    msg.data[0]            = r;
    Luos_SendMsg(player, &msg);
}

void game_service()
{
    clear_screen();
    ball = EMPTY;
    printf("%s\n", table[ball]);
    printf("Press SPACE BAR to serve\n");
    char c;
    while (Luos_IsNodeDetected())
    {
        if ((kbhit()))
        {
            c = get_character();
            if (c == ' ')
            {
                send_random();
                return;
            }
        }
    }
}

void game_over()
{
    char c;
    clear_screen();
    printf("GAME_OVER\n");
    printf("Press SPACE BAR to restart\n");
    while (Luos_IsNodeDetected())
    {
        if ((kbhit()))
        {
            c = get_character();
            if (c == ' ')
            {
                send_random();
                break;
            }
        }
    }
    game_service();
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PingPong_Loop(void)
{
    static ball_t last_ball = EMPTY;
    uint32_t empty_date;
    char c;
    if (Luos_IsNodeDetected())
    {
        initialized = true;
        // Game running
        if (last_ball != ball)
        {
            last_ball = ball;
            clear_screen();
            printf("%s\n", table[ball]);
            printf("Game is running\n");
        }
        if (get_service)
        {
            game_service();
            get_service = false;
        }
        else
        {
            switch (ball)
            {
                case EMPTY:
                    empty_date = Luos_GetSystick();
                    break;
                case LEFT:
                case RIGHT:
                    if ((kbhit()))
                    {
                        c = get_character();
                        if (c == 'a')
                        {
                            if (ball == LEFT)
                            {
                                ball = EMPTY;
                                send_random();
                            }
                            else
                            {
                                game_over();
                            }
                        }
                        if (c == 'z')
                        {
                            if (ball == RIGHT)
                            {
                                ball = EMPTY;
                                send_random();
                            }
                            else
                            {
                                game_over();
                            }
                        }
                    }
                    if (Luos_GetSystick() - empty_date > BALL_TIMEOUT)
                    {
                        game_over();
                    }
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        // Game is not running
        if (!initialized) //& space
        {
            if ((kbhit()))
            {
                c = get_character();
                if (c == ' ')
                {
                    Luos_Detect(player);
                    initialized = true;
                    get_service = true;
                }
            }
        }
        else
        {
            clear_screen();
            printf("%s\n", table[EMPTY]);
            printf("Game is starting, please wait...\n");
        }
    }
}
