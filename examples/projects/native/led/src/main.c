#include "luos_engine.h"
#include "serial_network.h"
#include "led.h"

int main(void)
{
    Luos_Init();
    Serial_Init();
    Led_Init();
    while (1)
    {
        Luos_Loop();
        Serial_Loop();
        Led_Loop();
    }
}
