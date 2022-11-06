#include <zephyr/kernel.h>
#include <luos_engine.h>
#include <led.h>

int main(void)
{
    Luos_Init();
    Led_Init();
    while (1)
    {
        Luos_Loop();
        Led_Loop();
        k_sleep(K_MSEC(10));
        printk("Spining Luos Engine each 10 ms \n");
    }
}
