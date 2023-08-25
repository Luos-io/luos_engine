#include "luos_engine.h"
#include "serial_network.h"
#include "pipe.h"
#include "gate.h"
#include <pthread.h>

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *Gate_Pipe_LoopThread(void *vargp)
{
    while (1)
    {
        Pipe_Loop();
        Gate_Loop();
    }
    return NULL;
}

void handler(int sig)
{
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main(void)
{
    signal(SIGSEGV, handler); // install our handler
    Luos_Init();
    Serial_Init();
    Pipe_Init();
    Gate_Init();
    // Create a thread to convert messages into Json and steam them using Websocket
    // pthread_t thread_id;
    // pthread_create(&thread_id, NULL, Gate_Pipe_LoopThread, NULL);
    while (1)
    {
        Luos_Loop();
        Serial_Loop();
        Pipe_Loop();
        Gate_Loop();
    }
}
