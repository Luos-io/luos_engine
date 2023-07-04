#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "luos_engine.h"
#include "robus_network.h"
#include "button.h"
#include "led.h"

// Definitions of LuosTask
const osThreadAttr_t LuosTask_attributes = {
    .name       = "LuosTask",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t ButtonTask_attributes = {
    .name       = "Button",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t LedTask_attributes = {
    .name       = "Led",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

void StartLuosTask(void *argument);
void StartButtonTask(void *argument);
void StartLedTask(void *argument);

void Firmware_Init(void);

void Firmware_Init(void)
{
    // Initialization of LuosTask
    Luos_Init();
    Robus_Init();
    Button_Init();
    Led_Init();

    // Creation of Luos Threads
    osThreadNew(StartLuosTask, NULL, &LuosTask_attributes);
    osThreadNew(StartButtonTask, NULL, &ButtonTask_attributes);
    osThreadNew(StartLedTask, NULL, &LedTask_attributes);
}

void StartLuosTask(void *argument)
{
    while (1)
    {
        Luos_Loop();
        taskYIELD();
    }
}

void StartButtonTask(void *argument)
{
    while (1)
    {
        Button_Loop();
        taskYIELD();
    }
}

void StartLedTask(void *argument)
{
    while (1)
    {
        Led_Loop();
        taskYIELD();
    }
}
