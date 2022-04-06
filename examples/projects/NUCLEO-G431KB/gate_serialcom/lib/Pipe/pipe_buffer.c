/******************************************************************************
 * @file pipe_buffer
 * @brief communication buffer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "pipe_com.h"
#include "pipe_buffer.h"
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t P2L_Buffer[PIPE_TO_LUOS_BUFFER_SIZE] = {0};
uint8_t L2P_Buffer[LUOS_TO_PIPE_BUFFER_SIZE] = {0};

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeBuffer_SetL2PMsg(uint8_t *data, uint16_t size)
{
    streaming_channel_t *StreamChannel = get_L2P_StreamChannel();
    SerialProtocol_t SerialProtocol    = {SERIAL_HEADER, 0, SERIAL_FOOTER};

    SerialProtocol.Size = size;
    Stream_PutSample(StreamChannel, &SerialProtocol, 3);
    Stream_PutSample(StreamChannel, data, size);
    Stream_PutSample(StreamChannel, &SerialProtocol.Footer, 1);
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
uint8_t PipeBuffer_GetP2LMsg(uint16_t *size)
{
    streaming_channel_t *StreamChannel = get_P2L_StreamChannel();
    uint16_t TotalSize                 = Stream_GetAvailableSampleNB(StreamChannel);
    uint16_t SizeUntilEnd              = 0;

    if (TotalSize > 0)
    {
        for (uint16_t i = 0; i < TotalSize; i++)
        {
            if (*((uint8_t *)(StreamChannel->sample_ptr)) == SERIAL_HEADER)
            {
                TotalSize    = TotalSize - i;
                SizeUntilEnd = Stream_GetAvailableSampleNBUntilEndBuffer(StreamChannel);
                if (SizeUntilEnd > 1)
                {
                    *size = (uint16_t)(*((uint8_t *)(StreamChannel->sample_ptr + 1)));
                }
                else
                {
                    *size = (uint16_t)(*((uint8_t *)(StreamChannel->ring_buffer)));
                }

                if (SizeUntilEnd > 2)
                {
                    *size |= (uint16_t)(*((uint8_t *)(StreamChannel->sample_ptr + 2)) << 8);
                }
                else
                {
                    *size |= (uint16_t)(*((uint8_t *)(StreamChannel->ring_buffer)) << 8);
                }
                if (TotalSize > *size)
                {
                    if (SizeUntilEnd > (*size + 3))
                    {
                        if (*((uint8_t *)(StreamChannel->sample_ptr + (*size + 3))) == SERIAL_FOOTER)
                        {
                            Stream_RmvAvailableSampleNB(StreamChannel, 3);
                            return true;
                        }
                    }
                    else
                    {
                        if (*((uint8_t *)(StreamChannel->ring_buffer + ((*size + 3) - SizeUntilEnd))) == SERIAL_FOOTER)
                        {
                            Stream_RmvAvailableSampleNB(StreamChannel, 3);
                            return true;
                        }
                    }
                }
                else
                {
                    if (*size < PIPE_TO_LUOS_BUFFER_SIZE)
                    {
                        return false;
                    }
                }
            }
            Stream_RmvAvailableSampleNB(StreamChannel, 1);
        }
    }
    return false;
}
/******************************************************************************
 * @brief Get L2P Buffer adresse
 * @param None
 * @return None
 ******************************************************************************/
uint8_t *PipeBuffer_GetL2PBuffer(void)
{
    return &L2P_Buffer[0];
}
/******************************************************************************
 * @brief Get P2L Buffer adresse
 * @param None
 * @return None
 ******************************************************************************/
uint8_t *PipeBuffer_GetP2LBuffer(void)
{
    return &P2L_Buffer[0];
}
