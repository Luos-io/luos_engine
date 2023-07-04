#include "luos_engine.h"
#include "robus_network.h"
#include "pipe.h"
#include "gate.h"
#include <pthread.h>

void *Gate_Pipe_LoopThread(void *vargp)
{
    while (1)
    {
        Pipe_Loop();
        Gate_Loop();
    }
    return NULL;
}

int main(void)
{
    Luos_Init();
    Robus_Init();
    Pipe_Init();
    Gate_Init();
    // Create a thread to convert messages into Json and steam them using Websocket
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, Gate_Pipe_LoopThread, NULL);
    while (1)
    {
        Luos_Loop();
    }
}
