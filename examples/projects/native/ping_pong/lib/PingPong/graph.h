/******************************************************************************
 * @file graph
 * @brief The graph of the game
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef GRAPH_H
#define GRAPH_H

typedef enum
{
    EMPTY,
    LEFT,
    RIGHT
} ball_t;

typedef void (*SCREEN)(void);
void set_screen_to(SCREEN);
void create_ball(ball_t *ballpt);
void *Graph_LoopThread(void *vargp);
int msleep(long msec);

void start_view(void);
void service_view(void);
void gameOver_view(void);
void game_view(void);
void wait_view(void);
void alone_view(void);
void score_view(void);

#endif /* PINGPONG_H */