#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <luos.h>
#include "fingerprint.h"

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
    Fingerprint_Init();
}
/******************************************************************************
 * @brief Loop Arduino
 * @param None
 * @return None
 ******************************************************************************/
void loop()
{
    Luos_Loop();
    Fingerprint_Loop();
}
