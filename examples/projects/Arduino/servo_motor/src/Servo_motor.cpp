#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "luos_engine.h"
#include "robus.h"
#include "servo_motor.h"

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
    Servo_Init();
}
/******************************************************************************
 * @brief Loop Arduino
 * @param None
 * @return None
 ******************************************************************************/
void loop()
{
    Luos_Loop();
    Servo_Loop();
}
