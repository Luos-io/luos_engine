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
    Serial.begin(PIPE_SERIAL_BAUDRATE);
} /******************************************************************************
   * @brief loop must be call in project loop
   * @param None
   * @return None
   ******************************************************************************/
void PipeCom_Loop(void)
{
    uint16_t size = 0;
    uint8_t data  = 0;
    while (Serial.available() > 0)
    {
        data = Serial.read();
        Stream_PutSample(get_P2L_StreamChannel(), &data, 1);
    }
    // Check if we need to transmit
    if (PipeCom_SendL2PPending() == false)
    {
        streaming_channel_t *stream_channel = get_L2P_StreamChannel();
        size                                = Stream_GetAvailableSampleNB(stream_channel);
        if (size != 0)
        {
            PipeCom_SendL2P((uint8_t *)stream_channel->sample_ptr, size);
            Stream_RmvAvailableSampleNB(stream_channel, size);
        }
    }
}
/******************************************************************************
 * @brief check if pipe is sending
 * @param None
 * @return true/false
 ******************************************************************************/
volatile uint8_t PipeCom_SendL2PPending(void)
{
    return false;
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_ReceiveP2L(void)
{
}
/******************************************************************************
 * @brief PipeCom_SendL2P
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_SendL2P(uint8_t *data, uint16_t size)
{
    Serial.write(data, size);
}
