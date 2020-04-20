#ifndef STREAMING_H
#define STREAMING_H

/** 
 * Streaming channel
 *  This structure manage a ring buffer as a streaming channel.
 * 
 *   |--------------------- ring_buffer_size --------------------|
 *   |*****************|...............|                         |
 *   ^                 ^               ^                         ^
 * ring_buffer    sample_ptr       data_ptr               end_ring_buffer
 * 
 */
typedef struct
{
    void* ring_buffer;     // Begin ring buffer pointer
    void* end_ring_buffer; // End of the ring buffer
    void* sample_ptr;      // Current sample pointer (pointer always point a fresh data)
    void* data_ptr;        // Current pointer of data
    char data_size;        // Size granulariry of the data contained on the ring buffer
} streaming_channel_t;

/**
 * \fn streaming_channel_t create_streaming_channel(const void* ring_buffer, int ring_buffer_size, char data_size)
 * \brief Initialisation of a streaming channel.
 *
 * \param ring_buffer Pointer to a data table
 * \param ring_buffer_size size of the buffer in number of values.
 * \param data_size values size.
 *
 * \return the streaming channel.
 *
 */
streaming_channel_t create_streaming_channel(const void *ring_buffer, int ring_buffer_size, char data_size);

/**
 * \fn void reset_streaming_channel(streaming_channel_t *stream)
 * \brief re initialize a streaming channel.
 *
 * \param stream streaming channel pointer
 *
 */
void reset_streaming_channel(streaming_channel_t *stream);

/**
 * \fn int get_sample(streaming_channel_t* stream, void* data)
 * \brief copy a sample from ring buffer to a data.
 *
 * \param stream streaming channel pointer
 * \param data a pointer of data
 *
 * \return the number of available samples.
 *
 */
int get_sample(streaming_channel_t *stream, void *data);

/**
 * \fn int get_sample(streaming_channel_t* stream, void* data)
 * \brief copy multiples samples from ring buffer to a data table.
 *
 * \param stream streaming channel pointer
 * \param data a pointer of data table
 * \param size The number of data to copy
 *
 * \return the number of available samples.
 *
 */
int get_samples(streaming_channel_t *stream, void *data, int size);

/**
 * \fn int get_nb_available_samples(streaming_channel_t *stream);
 * \brief return the number of available samples.
 *
 * \param stream streaming channel pointer
 *
 * \return the number of available samples.
 *
 */
int get_nb_available_samples(streaming_channel_t *stream);

/**
 * \fn int set_sample(streaming_channel_t *stream, void *data)
 * \brief set data into ring buffer.
 *
 * \param stream streaming channel pointer
 * \param data a pointer to the data
 * 
 * \return the number of available samples.
 *
 */
int set_sample(streaming_channel_t *stream, const void *data);

/**
 * \fn int set_samples(streaming_channel_t *stream, void *data, int size)
 * \brief set data into ring buffer.
 *
 * \param stream streaming channel pointer
 * \param data a pointer to the data table
 * \param size The number of data to copy
 * 
 * \return the number of available samples.
 *
 */
int set_samples(streaming_channel_t *stream, const void *data, int size);

#endif /* LUOS_H */