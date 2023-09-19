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

#include "pipe_com.h"
#include "../serial_protocol.h"

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
    SerialProtocol_Init();
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Loop(void)
{
    uint8_t data = 0;
    while (Serial.available() > 0)
    {
        data = Serial.read();
        Streaming_PutSample(Pipe_GetRxStreamChannel(), &data, 1);
    }
}
/******************************************************************************
 * @brief Check if a message is available
 * @param None
 * @return None
 ******************************************************************************/
uint8_t PipeCom_Receive(uint16_t *size)
{
    return SerialProtocol_IsMsgComplete(size);
}
/******************************************************************************
 * @brief PipeCom_Send
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Send(void)
{
    SerialProtocol_CreateTxMsg();
    uint16_t size = SerialProtocol_GetSizeToSend();
    while (size > 0)
    {
        Serial.write(SerialProtocol_GetDataToSend(), size);
        Streaming_RmvAvailableSampleNB(Pipe_GetTxStreamChannel(), size);
        size = SerialProtocol_GetSizeToSend();
    }
}
