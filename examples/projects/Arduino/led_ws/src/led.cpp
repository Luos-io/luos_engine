#include <Arduino.h>
#include "luos_engine.h"
#include "ws_network.h"
#include "led.h"

/******************************************************************************
 * @brief Setup ardiuno
 * @param None
 * @return None
 ******************************************************************************/
void setup()
{
    Luos_Init();
    Ws_Init();
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
    Ws_Loop();
    Led_Loop();
}
