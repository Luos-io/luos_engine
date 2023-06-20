#include "luos_engine.h"
#include "robus.h"
#include "button.h"

int main(void)
{
    Luos_Init();
    Robus_Init();
    Button_Init();
    while (1)
    {
        Luos_Loop();
        Button_Loop();
    }
}
