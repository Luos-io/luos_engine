#include "luos_engine.h"
#include "robus_network.h"
#include "led.h"

void app_main(void)
{
    Luos_Init();
    Robus_Init();
    Led_Init();

    while (1)
    {
        Luos_Loop();
        Led_Loop();
    }
}
