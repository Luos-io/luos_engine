#include <Arduino.h>
#include "motor.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "luos_engine.h"
#include "robus_network.h"

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
