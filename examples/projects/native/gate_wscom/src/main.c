#include "luos_engine.h"
#include "pipe.h"
#include "gate.h"

int main(void)
{
    Luos_Init();
    Pipe_Init();
    Gate_Init();
    while (1)
    {
        Luos_Loop();
        Pipe_Loop();
        Gate_Loop();
    }
}