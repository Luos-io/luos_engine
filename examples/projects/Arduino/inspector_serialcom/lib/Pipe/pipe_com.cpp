/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "pipe_com.h"
#include "luos_utils.h"

#ifdef __cplusplus
}
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t is_sending = false;
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Init(void)
{
    Serial.begin(1000000);
} /******************************************************************************
   * @brief loop must be call in project loop
   * @param None
   * @return None
   ******************************************************************************/
void PipeCom_Loop(void)
{
}
/******************************************************************************
 * @brief check if pipe is sending
 * @param None
 * @return true/false
 ******************************************************************************/
volatile uint8_t PipeCom_SendL2PPending(void)
{
    return is_sending;
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_ReceiveP2L(void)
{
    uint8_t data = 0;
    while (Serial.available() > 0)
    {
        data = Serial.read();
        Stream_PutSample(get_P2L_StreamChannel(), &data, 1);
    }
}
/******************************************************************************
 * @brief PipeCom_SendL2P
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_SendL2P(uint8_t *data, uint16_t size)
{
    is_sending = true;
    Serial.write(data, size);
    is_sending = false;
}
