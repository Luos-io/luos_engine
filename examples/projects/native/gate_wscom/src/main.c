#include "luos_engine.h"
#include "button.h"
#include "pipe.h"
#include "gate.h"

int main(void)
{
    Luos_Init();
    Button_Init();
    Pipe_Init();
    Gate_Init();
    while (1)
    {
        Luos_Loop();
        Button_Loop();
        Pipe_Loop();
        Gate_Loop();
    }
}