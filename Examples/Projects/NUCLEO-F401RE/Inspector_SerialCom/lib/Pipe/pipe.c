/******************************************************************************
 * @file pipe
 * @brief driver example a simple pipe
 * @author Luos
 * @version 0.1.0
 ******************************************************************************/
#include "pipe.h"
#include "pipe_com.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
streaming_channel_t P2L_StreamChannel;
streaming_channel_t L2P_StreamChannel;

uint8_t L2P_CompleteMsg = true;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Pipe_MsgHandler(service_t *service, msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Pipe_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    PipeCom_Init();
    Luos_CreateService(Pipe_MsgHandler, PIPE_TYPE, "Pipe", revision);
    P2L_StreamChannel = Stream_CreateStreamingChannel(PipeBuffer_GetP2LBuffer(), PIPE_TO_LUOS_BUFFER_SIZE, 1);
    L2P_StreamChannel = Stream_CreateStreamingChannel(PipeBuffer_GetL2PBuffer(), LUOS_TO_PIPE_BUFFER_SIZE, 1);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Pipe_Loop(void)
{
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Pipe_MsgHandler(service_t *service, msg_t *msg)
{
    SerialProtocol_t SerialProtocol = {SERIAL_HEADER, 0, SERIAL_FOOTER};
    uint16_t size                   = 0;
    if (msg->header.cmd == GET_CMD)
    {
        if (true == PipeBuffer_GetP2LMsg(&size))
        {
            // fill the message infos
            msg_t pub_msg;
            pub_msg.header.cmd         = SET_CMD;
            pub_msg.header.target_mode = ID;
            pub_msg.header.target      = msg->header.source;
            Luos_SendStreamingSize(service, &pub_msg, &P2L_StreamChannel, size);
        }
    }
    else if (msg->header.cmd == SET_CMD)
    {
        if (msg->header.size > 0)
        {
            if (L2P_CompleteMsg == true)
            {
                L2P_CompleteMsg     = false;
                SerialProtocol.Size = msg->header.size;
                Stream_PutSample(&L2P_StreamChannel, &SerialProtocol, 3);
            }
            if (Luos_ReceiveStreaming(service, msg, &L2P_StreamChannel) == SUCCEED)
            {
                Stream_PutSample(&L2P_StreamChannel, &SerialProtocol.Footer, 1);
                L2P_CompleteMsg = false;
            }
        }
        if (PipeCom_SendL2PPending() == false)
        {
            size = Stream_GetAvailableSampleNBUntilEndBuffer(&L2P_StreamChannel);
            if (size > 0)
            {
                PipeCom_SendL2P(L2P_StreamChannel.sample_ptr, size);
            }
        }
    }
    else if (msg->header.cmd == PARAMETERS)
    {
        // return output streaming channel adress
        msg_t pub_msg;
        pub_msg.header.cmd         = PARAMETERS;
        pub_msg.header.target_mode = IDACK;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(void *);
        int value                  = (int)&PipeBuffer_SetL2PMsg;
        memcpy(pub_msg.data, &value, sizeof(void *));
        Luos_SendMsg(service, &pub_msg);
    }
    else if (msg->header.cmd == REINIT)
    {
        L2P_StreamChannel.sample_ptr = L2P_StreamChannel.data_ptr;
    }
}
/******************************************************************************
 * @brief get_L2P_StreamChannel get adresse streaming
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
streaming_channel_t *get_L2P_StreamChannel(void)
{
    return &L2P_StreamChannel;
}
/******************************************************************************
 * @brief get_L2P_StreamChannel get adresse streaming
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
streaming_channel_t *get_P2L_StreamChannel(void)
{
    return &P2L_StreamChannel;
}
