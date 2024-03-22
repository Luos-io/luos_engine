/******************************************************************************
 * @file serial_protocol.h
 * @brief Protocol dedicated to serial communication allowing to catch the completude of the message
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include <stdbool.h>
#include <string.h>
#include "_pipe.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SERIAL_HEADER 0x7E
#define SERIAL_FOOTER 0x81

/*******************************************************************************
 * Variables
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    uint8_t header;
    uint16_t size;
} SerialHeader_t;

typedef struct __attribute__((__packed__))
{
    uint8_t footer;
    SerialHeader_t header;
} SerialProtocol_t;

streaming_channel_t *serialTx_StreamChannel = NULL;
uint8_t *size_to_update                     = NULL; // Size is uint16_t but we use uint8_t* here to avoid memory alignement issues

static inline void SerialProtocol_Init(void)
{
    // Get tx stream channel
    serialTx_StreamChannel = Pipe_GetTxStreamChannel();
    // write header + size into the buffer
    SerialHeader_t SerialHeader = {
        SERIAL_HEADER,
        0,
    };
    Streaming_PutSample(serialTx_StreamChannel, &SerialHeader, sizeof(SerialHeader_t));
    // Keep size to update, size are the last 2 bytes of the StreamChannel
    size_to_update = (uint8_t *)((int)serialTx_StreamChannel->data_ptr - 2);
    // Now we are ready to get data
}

static inline void SerialProtocol_CreateTxMsg(void)
{
    static char *last_data_end = NULL;
    // Evaluate size
    // We want to evaluate the size of the last message only. To do that we create a fake StreamChannel with the same properties as the serialTx_StreamChannel but with the sample_ptr pointing to the begining of our message.
    streaming_channel_t fake_StreamChannel = *serialTx_StreamChannel;
    memcpy(&fake_StreamChannel, serialTx_StreamChannel, sizeof(streaming_channel_t));
    fake_StreamChannel.sample_ptr = size_to_update;                                          // We make it point to the size
    uint16_t size                 = Streaming_GetAvailableSampleNB(&fake_StreamChannel) - 2; // We remove the size bytes from the calculated size.
    // Update size
    if ((size_to_update + 2) > (uint8_t *)((int)serialTx_StreamChannel->end_ring_buffer))
    {
        // Size is 2 bytes and those bytes are splitted, one at the end of the buffer and one at the beginning of the buffer
        size_to_update[0]                                   = size & 0xFF;
        ((uint8_t *)serialTx_StreamChannel->ring_buffer)[0] = size >> 8;
    }
    else
    {
        // Size is not splitted, we can write it directly in the buffer
        // We still writing it byte by bytes avoiding memory alignement issues
        size_to_update[0] = size & 0xFF;
        size_to_update[1] = size >> 8;
    }
    last_data_end = (char *)serialTx_StreamChannel->data_ptr;
    // Write footer and  header + size of the next message into the buffer
    SerialProtocol_t SerialProtocol = {
        SERIAL_FOOTER,
        {SERIAL_HEADER,
         0} // Size will be updated on the next call of SerialProtocol_CreateTxMsg()
    };
    Streaming_PutSample(serialTx_StreamChannel, &SerialProtocol, sizeof(SerialProtocol_t));

    // Keep size to update, size are the last 2 bytes of the StreamChannel
    if (serialTx_StreamChannel->data_ptr == serialTx_StreamChannel->ring_buffer)
    {
        size_to_update = (uint8_t *)((int)serialTx_StreamChannel->end_ring_buffer - 2);
    }
    else if ((uintptr_t)serialTx_StreamChannel->data_ptr == (uintptr_t)serialTx_StreamChannel->ring_buffer + (uintptr_t)serialTx_StreamChannel->data_size)
    {
        size_to_update = (uint8_t *)((int)serialTx_StreamChannel->end_ring_buffer - 1);
    }
    else
    {
        size_to_update = (uint8_t *)((int)serialTx_StreamChannel->data_ptr - 2);
    }
}

static inline uint16_t SerialProtocol_GetSizeToSend(void)
{
    // We want to evaluate the size of the last message only. To do that we create a fake StreamChannel with the same properties as the serialTx_StreamChannel but with the sample_ptr pointing to the begining of our message.
    streaming_channel_t fake_StreamChannel = *serialTx_StreamChannel;
    memcpy(&fake_StreamChannel, serialTx_StreamChannel, sizeof(streaming_channel_t));
    fake_StreamChannel.data_ptr = size_to_update; // We the end of the data point to the size
    if ((Streaming_GetAvailableSampleNB(&fake_StreamChannel) - 1) > Streaming_GetAvailableSampleNBUntilEndBuffer(&fake_StreamChannel))
    {
        return Streaming_GetAvailableSampleNBUntilEndBuffer(&fake_StreamChannel);
    }
    else
    {
        return Streaming_GetAvailableSampleNB(&fake_StreamChannel) - 1;
    }
}

static inline char *SerialProtocol_GetDataToSend(void)
{
    return (char *)serialTx_StreamChannel->sample_ptr;
}

static inline uint8_t SerialProtocol_IsMsgComplete(uint16_t *size)
{
    streaming_channel_t *serialRx_StreamChannel = Pipe_GetRxStreamChannel();
    uint16_t TotalSize                          = Streaming_GetAvailableSampleNB(serialRx_StreamChannel);
    uint16_t SizeUntilEnd                       = 0;

    if (TotalSize > 4)
    {
        // We received something
        for (uint16_t i = 0; i < TotalSize; i++)
        {
            if (*((uint8_t *)(serialRx_StreamChannel->sample_ptr)) == SERIAL_HEADER)
            {
                // The first byte is a header
                // Make TotalSize fit the size of remaining datas.
                TotalSize    = TotalSize - i;
                i            = 0;
                SizeUntilEnd = Streaming_GetAvailableSampleNBUntilEndBuffer(serialRx_StreamChannel);
                // Make size pointer point to the size data (1 byte after the SERIAL_HEADER 16bits length)
                if (SizeUntilEnd > 1)
                {
                    *size = (uint16_t)(*((uint8_t *)((int)serialRx_StreamChannel->sample_ptr + 1)));
                    if (SizeUntilEnd > 2)
                    {
                        *size |= (uint16_t)(*((uint8_t *)((int)serialRx_StreamChannel->sample_ptr + 2)) << 8);
                    }
                    else
                    {
                        *size |= (uint16_t)(*((uint8_t *)(serialRx_StreamChannel->ring_buffer)) << 8);
                    }
                }
                else
                {
                    *size = (uint16_t)(*((uint8_t *)(serialRx_StreamChannel->ring_buffer)));
                    *size |= (uint16_t)(*((uint8_t *)((int)serialRx_StreamChannel->ring_buffer + 1)) << 8);
                }

                if (TotalSize > (*size + 3))
                {
                    // We should have a complete message
                    if (SizeUntilEnd > (*size + 3))
                    {
                        // The message is continuons on the ring buffer
                        if (*((uint8_t *)((int)serialRx_StreamChannel->sample_ptr + (*size + 3))) == SERIAL_FOOTER)
                        {
                            // This is a good message remove the header and size from the available sample keeping only the data
                            Streaming_RmvAvailableSampleNB(serialRx_StreamChannel, 3);
                            return true;
                        }
                        // Else this is not really a massage begin, so we have to remove the fake SERIAL_HEADER byte an dcontinue looking for it.
                    }
                    else
                    {
                        // The message is not continuons on the ring buffer
                        if (*((uint8_t *)((int)serialRx_StreamChannel->ring_buffer + ((*size + 3) - SizeUntilEnd))) == SERIAL_FOOTER)
                        {
                            // This is a good message remove the header and size from the available sample keeping only the data
                            Streaming_RmvAvailableSampleNB(serialRx_StreamChannel, 3);
                            return true;
                        }
                        // Else this is not really a massage begin, so we have to remove the fake SERIAL_HEADER byte an dcontinue looking for it.
                    }
                }
                else
                {
                    // The message is not complete
                    // Check if the size make sense, if not this is not really a massage begin, so we have to remove the fake SERIAL_HEADER byte and continue looking for it.
                    if (*size < PIPE_RX_BUFFER_SIZE)
                    {
                        // The message sounds good but is just not finished.
                        return false;
                    }
                }
            }
            // The first byte is not a good header remove the byte
            Streaming_RmvAvailableSampleNB(serialRx_StreamChannel, 1);
        }
    }
    return false;
}
