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
        // We received something
        for (uint16_t i = 0; i < TotalSize; i++)
        {
            if (*((uint8_t *)(StreamChannel->sample_ptr)) == SERIAL_HEADER)
            {
                // The first byte is a header
                // Make TotalSize fit the size of remaining datas.
                TotalSize    = TotalSize - i;
                i            = 0;
                SizeUntilEnd = Stream_GetAvailableSampleNBUntilEndBuffer(StreamChannel);
                // Make size pointer point to the size data (1 byte after the SERIAL_HEADER 16bits length)
                if (SizeUntilEnd > 1)
                {
                    *size = (uint16_t)(*((uint8_t *)(StreamChannel->sample_ptr + 1)));
                    if (SizeUntilEnd > 2)
                    {
                        *size |= (uint16_t)(*((uint8_t *)(StreamChannel->sample_ptr + 2)) << 8);
                    }
                    else
                    {
                        *size |= (uint16_t)(*((uint8_t *)(StreamChannel->ring_buffer)) << 8);
                    }
                }
                else
                {
                    *size = (uint16_t)(*((uint8_t *)(StreamChannel->ring_buffer)));
                    *size |= (uint16_t)(*((uint8_t *)(StreamChannel->ring_buffer + 1)) << 8);
                }

                if (TotalSize > *size)
                {
                    // We should have a complete message
                    if (SizeUntilEnd > (*size + 3))
                    {
                        // The message is continuons on the ring buffer
                        if (*((uint8_t *)(StreamChannel->sample_ptr + (*size + 3))) == SERIAL_FOOTER)
                        {
                            // This is a good message remove the header and size from the available sample keeping only the data
                            Stream_RmvAvailableSampleNB(StreamChannel, 3);
                            return true;
                        }
                        // Else this is not really a massage begin, so we have to remove the fake SERIAL_HEADER byte an dcontinue looking for it.
                    }
                    else
                    {
                        // The message is not continuons on the ring buffer
                        if (*((uint8_t *)(StreamChannel->ring_buffer + ((*size + 3) - SizeUntilEnd))) == SERIAL_FOOTER)
                        {
                            // This is a good message remove the header and size from the available sample keeping only the data
                            Stream_RmvAvailableSampleNB(StreamChannel, 3);
                            return true;
                        }
                        // Else this is not really a massage begin, so we have to remove the fake SERIAL_HEADER byte an dcontinue looking for it.
                    }
                }
                else
                {
                    // The message is not complete
                    // Check if the size mak sense, if not this is not really a massage begin, so we have to remove the fake SERIAL_HEADER byte an dcontinue looking for it.
                    if (*size < PIPE_TO_LUOS_BUFFER_SIZE)
                    {
                        // The message sounds good but is just not finished.
                        return false;
                    }
                }
            }
            // The first byte is not a good header remove the byte
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
