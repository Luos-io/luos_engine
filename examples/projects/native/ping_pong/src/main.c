#include "luos_engine.h"
#include "robus_network.h"
#include "ping_pong.h"
#include <pthread.h>

void *PingPong_LoopThread(void *vargp)
{
    while (1)
    {
        PingPong_Loop();
    }
    return NULL;
}

int main(void)
{
    Luos_Init();
    Robus_Init();
    PingPong_Init();
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, PingPong_LoopThread, NULL);
    while (1)
    {
        Luos_Loop();
        Robus_Loop();
    }
}
