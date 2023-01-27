#include "luos_engine.h"
#include "ping_pong.h"

int main(void)
{
    Luos_Init();
    PingPong_Init();
    while (1)
    {
        Luos_Loop();
        PingPong_Loop();
    }
}