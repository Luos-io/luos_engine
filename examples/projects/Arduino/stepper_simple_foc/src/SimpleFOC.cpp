#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "motor.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "luos_engine.h"

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
    Motor_Init();
}
/******************************************************************************
 * @brief Loop Arduino
 * @param None
 * @return None
 ******************************************************************************/
void loop()
{
    Luos_Loop();
    Motor_Loop();
}
