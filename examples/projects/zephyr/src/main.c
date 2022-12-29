#include <zephyr/kernel.h>
#include <luos_engine.h>
#include <robus_hal.h>
#include <led.h>


static void luos_thread_loop(void *arg1, void *arg2, void *arg3);
K_THREAD_DEFINE(luos_thread, 8192, luos_thread_loop, NULL, NULL, NULL, 8, 0, 0);

static void luos_thread_loop(void *arg1, void *arg2, void *arg3)
{
    Luos_Init();
    Led_Init();

    while (1)
    {   
        //Way to make Luos run in event driven mode 
        RobusHAL_WaitNetworkActivity();
        printk("Network activity detected, spinning luos! \n");
        Luos_Loop();
        Led_Loop();
    }

}

void main(void)
{
    printk("Luos App on Zephyr is running \n");
}
