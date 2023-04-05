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
#include "graph.h"
#include <pthread.h>
#include "scoring.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef _WIN32
    #define get_character() _getch()
    #define LEFT_KEY        'K'
    #define RIGHT_KEY       'M'
#else
    #define get_character() getchar()
    #define LEFT_KEY        'D'
    #define RIGHT_KEY       'C'
#endif

#define BALL_TIMEOUT 700

static void game_start(void);
static void game_loading(void);
static void game_service(void);
static void game_over(void);
static void game_running(void);
static void game_alone(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *player;
#define PLAYER_TYPE LUOS_LAST_TYPE
bool get_service = false;

#define BALL_POS_CMD LUOS_LAST_STD_CMD

ball_t ball = EMPTY;
typedef void (*GAME_STATE)(void);
GAME_STATE game_state = NULL;
bool initialized      = false;
uint16_t last_id      = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/

void Player_MsgHandler(service_t *service, msg_t *msg)
{
    if ((msg->header.target_mode == TOPIC) & (msg->header.target = SCORE_TOPIC) & (msg->header.source != service->id))
    {
        score_update(msg);
        set_screen_to(score_view);
    }
    if (msg->header.cmd == BALL_POS_CMD)
    {
        ball    = (ball_t)msg->data[0];
        last_id = msg->header.source;
    }
    if (msg->header.cmd == END_DETECTION)
    {
        search_result_t target_list;
        RTFilter_Reset(&target_list);
        RTFilter_Type(&target_list, PLAYER_TYPE);
        score_init(player, &target_list);
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
    char str[30];
    printf("\n\t\tWelcome to \n\tThe Luos ping pong world cup!\n");
    printf("Enter your player name :");
    scanf("%s", str);

    player = Luos_CreateService(Player_MsgHandler, PLAYER_TYPE, str, revision);

    create_ball(&ball);
    set_screen_to(start_view);
    game_state = game_start;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, Graph_LoopThread, NULL);

    srand(time(NULL)); // Initialization, should only be called once.
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PingPong_Loop(void)
{
    // This is a function pointer that will louch the good function depending on the state of the game.
    game_state();
    msleep(10);
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

bool send_random()
{
    // This function send the ball to a random player at a random position.
    // Randomize ball position.
    uint8_t r = (rand() % 2) + 1; // Returns a pseudo-random integer between 0 and RAND_MAX.
    // Randomize target
    search_result_t target_list;
    RTFilter_Reset(&target_list);
    RTFilter_Type(&target_list, PLAYER_TYPE);
    if (target_list.result_nbr == 1)
    {
        return false;
    }
    int target;
    do
    {
        target = rand() % target_list.result_nbr;
    } while (target_list.result_table[target]->id == player->id);

    // Our target is OK, Send the message
    msg_t msg;
    msg.header.target      = target_list.result_table[target]->id;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = sizeof(r);
    msg.header.cmd         = BALL_POS_CMD;
    msg.data[0]            = r;
    Luos_SendMsg(player, &msg);
    return true;
}

void game_service()
{
    ball        = EMPTY;
    get_service = false;
    set_screen_to(service_view);
    char c;
    while (1)
    {
        if ((kbhit()))
        {
            c = get_character();
            if (c == ' ')
            {

                if (send_random())
                {
                    set_screen_to(game_view);
                    game_state = game_running;
                }
                else
                {
                    // You are probably alone
                    game_state = game_alone;
                }
                return;
            }
        }
        if (!Luos_IsDetected())
        {
            // Someone relaunch a detection
            game_state = game_loading;
            return;
        }
    }
}

void game_over()
{
    char c;
    score_increase(player, last_id);
    set_screen_to(gameOver_view);
    while (Luos_IsDetected())
    {
        if ((kbhit()))
        {
            c = get_character();
            if (c == ' ')
            {
                break;
            }
        }
    }
    game_service();
}

void game_start()
{
    char c;
    set_screen_to(start_view);
    // Game is not running
    while (!Luos_IsDetected())
    {
        // This is the initialization
        if ((kbhit()))
        {
            c = get_character();
            if (c == ' ')
            {
                // This player lanch the detection
                Luos_Detect(player);
                get_service = true;
                game_state  = game_loading;
                return;
            }
        }
    }
    // Someone did a detection.
    game_state = game_running;
}

void game_alone()
{
    static bool update_alone = true;
    char c;
    if (update_alone)
    {
        update_alone = false;
        set_screen_to(alone_view);
    }
    // Game is not running
    // This is the initialization
    if ((kbhit()))
    {
        c = get_character();
        if (c == ' ')
        {
            // This player lanch the detection
            Luos_Detect(player);
            get_service  = true;
            game_state   = game_loading;
            update_alone = true;
            while (Luos_IsDetected())
            {
            }
            return;
        }
    }
}

void game_loading()
{
    set_screen_to(wait_view);
    while (!Luos_IsDetected())
    {
    }
    if (get_service)
    {
        game_state = game_service;
    }
    else
    {
        game_state = game_running;
    }
}

void game_running()
{
    static ball_t last_ball = LEFT;
    static uint32_t empty_date;
    uint8_t c;
    if (!Luos_IsDetected())
    {
        // Someone relaunch a detection
        game_state = game_loading;
        return;
    }
    if (last_ball != ball)
    {
        // Update the screen to display
        last_ball = ball;
        set_screen_to(game_view);
    }
    // Game loop
    if (ball == EMPTY)
    {
        empty_date = Luos_GetSystick();
    }
    else
    {
        // We have the ball on our table, user have to react.
        if ((kbhit()))
        {
#ifdef _WIN32
            c = get_character();
            if (c == 0 || c == 224)
            {
#else
            if (get_character() == '\033')
            {
                get_character(); // skip the [
#endif
                c = get_character();
                if (c == LEFT_KEY)
                {
                    if (ball == LEFT)
                    {
                        ball = EMPTY;
                        send_random();
                    }
                    else
                    {
                        // Wrong key
                        game_state = game_over;
                        return;
                    }
                }
                if (c == RIGHT_KEY)
                {
                    if (ball == RIGHT)
                    {
                        ball = EMPTY;
                        send_random();
                    }
                    else
                    {
                        // Wrong key
                        game_state = game_over;
                        return;
                    }
                }
            }
        }
        if (Luos_GetSystick() - empty_date > BALL_TIMEOUT)
        {
            // Timeout!
            game_state = game_over;
            return;
        }
    }
}
