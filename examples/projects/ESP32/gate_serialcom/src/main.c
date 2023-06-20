
#include "luos_engine.h"
#include "robus.h"
#include "pipe.h"
#include "gate.h"

void app_main(void)
{
    Luos_Init();
    Robus_Init();
    Gate_Init();
    Pipe_Init();

    while (1)
    {
        Luos_Loop();
        Gate_Loop();
        Pipe_Loop();
    }
}
