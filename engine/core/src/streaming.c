/******************************************************************************
 * @file Streaming
 * @brief Streaming data through network
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <string.h>
#include "luos_engine.h"
#include "streaming.h"
#include "luos_utils.h"
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
 * @brief Initialisation of a streaming channel.
 * @param ring_buffer : Pointer to a data table
 * @param ring_buffer_size : Size of the buffer in number of values.
 * @param data_size : Values size.
 * @return Streaming channel
 ******************************************************************************/
streaming_channel_t Streaming_CreateChannel(const void *ring_buffer, uint16_t ring_buffer_size, uint8_t data_size)
{
    streaming_channel_t stream;
    LUOS_ASSERT((ring_buffer != NULL) || (ring_buffer_size > 0) || (data_size > 0));
    // Save ring buffer informations
    stream.ring_buffer     = (void *)ring_buffer;
    stream.data_size       = data_size;
    stream.end_ring_buffer = (void *)ring_buffer + (stream.data_size * ring_buffer_size);

    // Set data pointers to 0
    stream.data_ptr   = stream.ring_buffer;
    stream.sample_ptr = stream.ring_buffer;
    return stream;
}
/******************************************************************************
 * @brief Re initialize a streaming channel.
 * @param stream : Streaming channel pointer
 * @return None
 ******************************************************************************/
void Streaming_ResetChannel(streaming_channel_t *stream)
{
    stream->data_ptr   = stream->ring_buffer;
    stream->sample_ptr = stream->ring_buffer;
}
/******************************************************************************
 * @brief Set data into ring buffer.
 * @param stream : Streaming channel pointer
 * @param data : A pointer to the data table
 * @param size : The number of data to copy
 * @return Number of samples to put in buffer
 ******************************************************************************/
uint16_t Streaming_PutSample(streaming_channel_t *stream, const void *data, uint16_t size)
{
    // check if we exceed ring buffer capacity
    LUOS_ASSERT((Streaming_GetAvailableSampleNB(stream) + size) <= (stream->end_ring_buffer - stream->ring_buffer));
    if (((size * stream->data_size) + stream->data_ptr) >= stream->end_ring_buffer)
    {
        // our data exceeds ring buffer end, cut it and copy.
        uint16_t chunk1 = stream->end_ring_buffer - stream->data_ptr;
        uint16_t chunk2 = (size * stream->data_size) - chunk1;
        // Everything good copy datas.
        memcpy(stream->data_ptr, data, chunk1);
        memcpy(stream->ring_buffer, (char *)data + chunk1, chunk2);
        // Set the new data pointer
        stream->data_ptr = stream->ring_buffer + chunk2;
    }
    else
    {
        // our data fit before ring buffer end
        memcpy(stream->data_ptr, data, (size * stream->data_size));
        // Set the new data pointer
        stream->data_ptr = stream->data_ptr + (size * stream->data_size);
    }
    return Streaming_GetAvailableSampleNB(stream);
}
/******************************************************************************
 * @brief Copy a sample from ring buffer to a data.
 * @param stream : Streaming channel pointer
 * @param data : A pointer of data
 * @param size : data size
 * @return None
 ******************************************************************************/
uint16_t Streaming_GetSample(streaming_channel_t *stream, void *data, uint16_t size)
{
    uint16_t nb_available_samples = Streaming_GetAvailableSampleNB(stream);
    if (nb_available_samples >= size)
    {
        // check if we need to loop in ring buffer
        if ((stream->sample_ptr + (size * stream->data_size)) > stream->end_ring_buffer)
        {
            // requested data exceeds ring buffer end, cut it and copy.
            int chunk1 = stream->end_ring_buffer - stream->sample_ptr;
            int chunk2 = (size * stream->data_size) - chunk1;
            memcpy(data, stream->sample_ptr, chunk1);
            memcpy((char *)data + chunk1, stream->ring_buffer, chunk2);
            // Set the new sample pointer
            stream->sample_ptr = stream->ring_buffer + chunk2;
        }
        else
        {
            data = memcpy(data, stream->sample_ptr, (size * stream->data_size));
            // Set the new sample pointer
            stream->sample_ptr = stream->sample_ptr + (size * stream->data_size);
        }

        nb_available_samples -= size;
    }
    else
    {
        // no more data
        return 0;
    }
    return nb_available_samples;
}
/******************************************************************************
 * @brief Return the number of available samples
 * @param stream : Streaming channel pointer
 * @return Number of availabled samples
 ******************************************************************************/
uint16_t Streaming_GetAvailableSampleNB(streaming_channel_t *stream)
{
    int32_t nb_available_sample = (stream->data_ptr - stream->sample_ptr) / stream->data_size;
    if (nb_available_sample < 0)
    {
        // The buffer have looped
        nb_available_sample = ((stream->end_ring_buffer - stream->sample_ptr) + (stream->data_ptr - stream->ring_buffer)) / stream->data_size;
    }
    LUOS_ASSERT(nb_available_sample >= 0);
    return (uint16_t)nb_available_sample;
}
/******************************************************************************
 * @brief Get sample number availabled in buffer
 * @param stream : Streaming channel pointer
 * @return Number of available samples
 ******************************************************************************/
uint16_t Streaming_GetAvailableSampleNBUntilEndBuffer(streaming_channel_t *stream)
{
    int32_t nb_available_sample = (stream->data_ptr - stream->sample_ptr) / stream->data_size;
    if (nb_available_sample < 0)
    {
        // The buffer have looped
        nb_available_sample = (stream->end_ring_buffer - stream->sample_ptr) / stream->data_size;
    }
    LUOS_ASSERT(nb_available_sample >= 0);
    return (uint16_t)nb_available_sample;
}
/******************************************************************************
 * @brief Set a number of sample available in buffer
 * @param stream : Streaming channel pointer
 * @param size : The number of data to copy
 * @return Number of samples to add to channel
 ******************************************************************************/
uint16_t Streaming_AddAvailableSampleNB(streaming_channel_t *stream, uint16_t size)
{
    LUOS_ASSERT((uint32_t)(Streaming_GetAvailableSampleNB(stream) + size) < (uint32_t)(stream->end_ring_buffer - stream->ring_buffer));
    if (((size * stream->data_size) + stream->data_ptr) >= stream->end_ring_buffer)
    {
        uint16_t chunk1  = stream->end_ring_buffer - stream->data_ptr;
        uint16_t chunk2  = (size * stream->data_size) - chunk1;
        stream->data_ptr = stream->ring_buffer + chunk2;
    }
    else
    {
        stream->data_ptr = stream->data_ptr + (size * stream->data_size);
    }
    return Streaming_GetAvailableSampleNB(stream);
}
/******************************************************************************
 * @brief Remove a specific number of samples in buffer
 * @param stream : Streaming channel pointer
 * @param size : The number of data to remove
 * @return Number of availabled samples
 ******************************************************************************/
uint16_t Streaming_RmvAvailableSampleNB(streaming_channel_t *stream, uint16_t size)
{
    LUOS_ASSERT(Streaming_GetAvailableSampleNB(stream) >= size);
    // Check if we exceed ring buffer capacity
    if (((size * stream->data_size) + stream->sample_ptr) > stream->end_ring_buffer)
    {
        // We exceed ring buffer end.
        uint16_t chunk1    = stream->end_ring_buffer - stream->sample_ptr;
        uint16_t chunk2    = (size * stream->data_size) - chunk1;
        stream->sample_ptr = stream->ring_buffer + chunk2;
    }
    else
    {
        stream->sample_ptr = stream->sample_ptr + (size * stream->data_size);
        if (stream->sample_ptr == stream->end_ring_buffer)
        {
            // If we are exactly at the end of the ring buffer, we need to loop
            stream->sample_ptr = stream->ring_buffer;
        }
    }
    return Streaming_GetAvailableSampleNB(stream);
}

/******************************************************************************
 * @brief Send datas of a streaming channel
 * @param Service : Who send
 * @param msg : Message to send
 * @param stram: Streaming channel pointer
 * @return None
 ******************************************************************************/
void Luos_SendStreaming(service_t *service, msg_t *msg, streaming_channel_t *stream)
{
    // Compute number of message needed to send available datas on ring buffer
    Luos_SendStreamingSize(service, msg, stream, Streaming_GetAvailableSampleNB(stream));
}
/******************************************************************************
 * @brief Send a number of datas of a streaming channel
 * @param service : Who send
 * @param msg : Message to send
 * @param stream : Streaming channel pointer
 * @param max_size : Maximum sample to send
 * @return None
 ******************************************************************************/
void Luos_SendStreamingSize(service_t *service, msg_t *msg, streaming_channel_t *stream, uint32_t max_size)
{
    // Compute number of message needed to send available datas on ring buffer
    int msg_number = 1;
    int data_size  = Streaming_GetAvailableSampleNB(stream);
    if (data_size > max_size)
    {
        data_size = max_size;
    }
    const int max_data_msg_size = (MAX_DATA_MSG_SIZE / stream->data_size);
    if (data_size > max_data_msg_size)
    {
        msg_number = (data_size / max_data_msg_size);
        msg_number += ((msg_number * max_data_msg_size) < data_size);
    }

    // Send messages one by one
    for (volatile uint16_t chunk = 0; chunk < msg_number; chunk++)
    {
        // compute chunk size
        uint16_t chunk_size = 0;
        if (data_size > max_data_msg_size)
        {
            chunk_size = max_data_msg_size;
        }
        else
        {
            chunk_size = data_size;
        }

        // Copy data into message
        Streaming_GetSample(stream, msg->data, chunk_size);
        msg->header.size = data_size;

        // Send message
        uint32_t tickstart = Luos_GetSystick();
        while (Luos_SendMsg(service, msg) == FAILED)
        {
            // No more memory space available
            // 500ms of timeout after start trying to load our data in memory. Perhaps the buffer is full of RX messages try to increate the buffer size.
            LUOS_ASSERT(((volatile uint32_t)Luos_GetSystick() - tickstart) < 500);
        }

        // check end of data
        if (data_size > max_data_msg_size)
        {
            data_size -= max_data_msg_size;
        }
        else
        {
            data_size = 0;
        }
    }
}
/******************************************************************************
 * @brief Receive a streaming channel datas
 * @param service : Who send
 * @param msg : Message to send
 * @param stream : Streaming channel pointer
 * @return error
 ******************************************************************************/
error_return_t Luos_ReceiveStreaming(service_t *service, msg_t *msg, streaming_channel_t *stream)
{
    // Get chunk size
    unsigned short chunk_size = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
        chunk_size = MAX_DATA_MSG_SIZE;
    else
        chunk_size = msg->header.size;

    // Copy data into buffer
    Streaming_PutSample(stream, msg->data, (chunk_size / stream->data_size));

    // Check end of data
    if ((msg->header.size <= MAX_DATA_MSG_SIZE))
    {
        // Chunk collection finished
        return SUCCEED;
    }
    return FAILED;
}