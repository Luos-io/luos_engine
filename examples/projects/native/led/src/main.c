#include "luos_engine.h"
#include "ws_network.h"
#include "led.h"

int main(void)
{
    Luos_Init();
    Ws_Init();
    Led_Init();
    while (1)
    {
        Luos_Loop();
        Ws_Loop();
        Led_Loop();
    }
}
