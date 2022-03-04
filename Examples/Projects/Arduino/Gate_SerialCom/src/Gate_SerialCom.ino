
#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <pipe.h>
#include <gate.h>
#include <luos.h>

#ifdef __cplusplus
}
#endif

void setup()
{

    Luos_Init();
    Gate_Init();
    Pipe_Init();
}

void loop()
{
    Luos_Loop();
    Gate_Loop();
    Pipe_Loop();
}
