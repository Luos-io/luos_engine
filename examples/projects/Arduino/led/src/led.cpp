#include <Arduino.h>
#include "luos_engine.h"
#include "serial_network.h"
#include "led.h"

/******************************************************************************
 * @brief Setup ardiuno
 * @param None
 * @return None
 ******************************************************************************/
void setup()
{
    Luos_Init();
    Serial_Init();
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
    Serial_Loop();
    Led_Loop();
}
