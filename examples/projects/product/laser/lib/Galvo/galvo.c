/******************************************************************************
 * @file galvo
 * @brief driver example a laser galvo
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "galvo.h"
#include "xy2-100.h"
#include "galvo_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
pos_2d_t stream_buf[4096];
streaming_channel_t stream;
time_luos_t period;
control_t control;
buffer_mode_t buffer_mode = SINGLE;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Galvo_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Galvo_Init(void)
{
    Xy_Init();

    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Galvo_MsgHandler, POINT_2D, "galvo", revision);
    stream       = Streaming_CreateChannel(stream_buf, sizeof(stream_buf), 2 * sizeof(uint16_t));
    period       = TimeOD_TimeFrom_s(1.0 / DEFAULT_SAMPLE_FREQUENCY); // Configure the trajectory samplerate at 100Hz
    control.flux = STOP;
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Galvo_Loop(void)
{
}

/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Galvo_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        // The galvo don't send anything back
        return;
    }
    if (msg->header.cmd == CONTROL)
    {
        // Get the control value
        ControlOD_ControlFromMsg(&control, msg);
        if (control.flux == PLAY)
        {
            // Start the trajectory
            Xy_Start(&stream, period);
        }
        else
        {
            // Stop the trajectory
            Xy_Stop();
        }
    }
    if (msg->header.cmd == LINEAR_POSITION_2D)
    {
        if (buffer_mode == STREAM)
        {
            // The galvo is in single mode, we can consider it as a streaming of point that will be consumed by the galvo
            Luos_ReceiveStreaming(service, msg, &stream);
        }
        if (buffer_mode == CONTINUOUS)
        {
            // The galvo is in CONTINUOUS, The buffer need to be loaded with the trajectory and the galvo will play it in loop.
            volatile int size = Luos_ReceiveData(service, msg, (uint8_t *)&stream_buf);
            if (size > 0)
            {
                LUOS_ASSERT(size <= sizeof(stream_buf));
                // overwrite the streaming structure to manage the bufferin loop
                // set the write pointer in the end of the data
                stream.data_ptr = stream_buf + size / stream.data_size;
                if (stream.data_ptr < stream.sample_ptr)
                {
                    // If the read pointer overlap the write pointer point it back to the begining of the buffer
                    // set the read pointer at the begining of the data
                    stream.sample_ptr = stream_buf;
                }
            }
        }
        else
        {
            // The galvo is in SINGLE, The buffer need to be loaded with the trajectory and the galvo will play it from the begining of the buffer to the end.
            volatile int size = Luos_ReceiveData(service, msg, (uint8_t *)&stream_buf);
            if (size > 0)
            {
                LUOS_ASSERT(size <= sizeof(stream_buf));
                // overwrite the streaming structure to manage the bufferin loop
                // set the write pointer in the end of the data
                stream.data_ptr = stream_buf + size / stream.data_size;
                // set the read pointer at the begining of the data
                stream.sample_ptr = stream_buf;
            }
        }
    }
    if (msg->header.cmd == BUFFER_MODE)
    {
        buffer_mode = msg->data[0];
        Xy_BufferMode(buffer_mode);
    }
    if (msg->header.cmd == TIME)
    {
        // Get the time
        TimeOD_TimeFromMsg(&period, msg);
    }
}
