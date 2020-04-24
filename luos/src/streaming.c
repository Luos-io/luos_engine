#include "streaming.h"
#include <string.h>

streaming_channel_t create_streaming_channel(const void *ring_buffer, int ring_buffer_size, char data_size)
{
    streaming_channel_t stream;
    if ((ring_buffer == NULL) || (ring_buffer_size < 1) || (data_size < 1))
    {
            while (1);
        }
    // Save ring buffer informations
    stream.ring_buffer = ring_buffer;
    stream.data_size = data_size;
    stream.end_ring_buffer = ring_buffer + (stream.data_size * ring_buffer_size);

    // Set data pointers to 0
    stream.data_ptr = stream.ring_buffer;
    stream.sample_ptr = stream.ring_buffer;
    return stream;
}

void reset_streaming_channel(streaming_channel_t *stream)
{
    stream->data_ptr = stream->ring_buffer;
    stream->sample_ptr = stream->ring_buffer;
}

int get_sample(streaming_channel_t *stream, void *data)
{
    return get_samples(stream, data, 1);
}

int get_samples(streaming_channel_t *stream, void *data, int size)
{
    int nb_available_samples = get_nb_available_samples(stream);
    if (nb_available_samples >= size)
    {
        // check if we need to loop in ring buffer
        if ((stream->sample_ptr + (size * stream->data_size)) > stream->end_ring_buffer)
        {
            // requested data exceeds ring buffer end, cut it and copy.
            int chunk1 = stream->end_ring_buffer - stream->sample_ptr;
            int chunk2 = (size * stream->data_size) - chunk1;
            memcpy(data, stream->sample_ptr, chunk1);
            memcpy(&data[chunk1], stream->ring_buffer, chunk2);
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

int get_nb_available_samples(streaming_channel_t *stream)
{
    int nb_available_sample = (stream->data_ptr - stream->sample_ptr) / stream->data_size;
    if (nb_available_sample < 0)
    {
        // The buffer have looped
        nb_available_sample = ((stream->end_ring_buffer - stream->sample_ptr) + (stream->data_ptr - stream->ring_buffer)) / stream->data_size;
    }
    return nb_available_sample;
}

int set_sample(streaming_channel_t *stream, const void *data)
{
    return set_samples(stream, data, 1);
}

int set_samples(streaming_channel_t *stream, const void *data, int size)
{
    if (((size * stream->data_size) + stream->data_ptr) > stream->end_ring_buffer) {
        // our data exceeds ring buffer end, cut it and copy.
        int chunk1 = stream->end_ring_buffer - stream->data_ptr;
        int chunk2 = (size * stream->data_size) - chunk1;
        // check if we exceed ring buffer capacity
        if (stream->sample_ptr < (stream->ring_buffer + chunk2))
        {
            // data overwrite unread samples
            while(1);
        }
        // Everything good copy datas.
        memcpy(stream->data_ptr, data, chunk1);
        memcpy(stream->ring_buffer, &data[chunk1], chunk2);
        // Set the new data pointer
        stream->data_ptr = stream->ring_buffer + chunk2;
    }
    else
    {
        // our data fit before ring buffer end
        // check if we exceed ring buffer capacity
        if ((stream->data_ptr < stream->sample_ptr) && ((stream->data_ptr + (size * stream->data_size)) > stream->sample_ptr))
        {
            // data overwrite unread samples
            while(1);
        }
        memcpy(stream->data_ptr, data, (size * stream->data_size));
        // Set the new data pointer
        stream->data_ptr = stream->data_ptr + (size * stream->data_size);
    }
    return get_nb_available_samples(stream);
}