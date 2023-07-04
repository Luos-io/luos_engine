
#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <pipe.h>
#include <inspector.h>
#include "luos_engine.h"
#include "robus_network.h"

#ifdef __cplusplus
}
#endif

void setup()
{

    Luos_Init();
    Robus_Init();
    Inspector_Init();
    Pipe_Init();
}

void loop()
{
    Luos_Loop();
    Inspector_Loop();
    Pipe_Loop();
}
