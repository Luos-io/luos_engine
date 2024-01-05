#include "luos_engine.h"
#include "ws_network.h"
#include "button.h"

int main(void)
{
    Luos_Init();
    Ws_Init();
    Button_Init();
    while (1)
    {
        Luos_Loop();
        Button_Loop();
        Ws_Loop();
    }
}
