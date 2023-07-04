#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "luos_engine.h"
#include "robus_network.h"
#include "led.h"

#ifdef __cplusplus
}
#endif

/******************************************************************************
 * @brief Setup ardiuno
 * @param None
 * @return None
 ******************************************************************************/
void setup()
{
    Luos_Init();
    Robus_Init();
    Led_Init();
}
/******************************************************************************
 * @brief Loop Arduino
 * @param None
 * @return None
 ******************************************************************************/
void loop()
{
    Luos_Loop();
    Led_Loop();
}
