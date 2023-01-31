/******************************************************************************
 * @file ping_pong
 * @brief driver example a simple button
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

void start_view(void);
void service_view(void);
void gameOver_view(void);
void game_view(void);
void wait_view(void);
void alone_view(void);

#endif /* PINGPONG_H */