/******************************************************************************
 * @file pipe
 * @brief driver example a simple pipe
 * @author Luos
 * @version 0.1.0
 ******************************************************************************/
#include <stdio.h>
#include "luos_engine.h"
#include "pipe.h"
#include "_pipe.h"
#include "pipe_com.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
streaming_channel_t rx_StreamChannel;
streaming_channel_t tx_StreamChannel;
uint8_t rx_Buffer[PIPE_RX_BUFFER_SIZE] = {0};
uint8_t tx_Buffer[PIPE_TX_BUFFER_SIZE] = {0};

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Pipe_MsgHandler(service_t *service, const msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Pipe_Init(void)
{
    rx_StreamChannel = Streaming_CreateChannel(rx_Buffer, PIPE_RX_BUFFER_SIZE, 1);
    tx_StreamChannel = Streaming_CreateChannel(tx_Buffer, PIPE_TX_BUFFER_SIZE, 1);
    PipeCom_Init();
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    Luos_CreateService(Pipe_MsgHandler, PIPE_TYPE, "Pipe", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Pipe_Loop(void)
{
    PipeCom_Loop();
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Pipe_MsgHandler(service_t *service, const msg_t *msg)
{
    uint16_t size = 0;

    if (msg->header.cmd == GET_CMD)
    {
        if (true == PipeCom_Receive(&size))
        {
            // fill the message infos
            msg_t pub_msg;
            pub_msg.header.cmd         = SET_CMD;
            pub_msg.header.target_mode = SERVICEID;
            pub_msg.header.target      = msg->header.source;
            Luos_SendStreamingSize(service, &pub_msg, &rx_StreamChannel, size);
        }
    }
    else if (msg->header.cmd == SET_CMD)
    {
        if (msg->header.size > 0)
        {
            if (Luos_ReceiveStreaming(service, msg, &tx_StreamChannel) == SUCCEED)
            {
                // Data reception is complete, we can send it.
                PipeCom_Send();
            }
        }
        else
        {
            // The source of the data we have to send is localhost and the data is already in the buffer, we just have to send it.
            PipeCom_Send();
        }
    }
    else if (msg->header.cmd == PARAMETERS)
    {
        // return output streaming channel adress
        msg_t pub_msg;
        pub_msg.header.cmd         = PARAMETERS;
        pub_msg.header.target_mode = SERVICEIDACK;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(uintptr_t);
        uintptr_t value            = (uintptr_t)&tx_StreamChannel;
        memcpy(pub_msg.data, &value, sizeof(uintptr_t));
        Luos_SendMsg(service, &pub_msg);
    }
    else if (msg->header.cmd == REINIT)
    {
        tx_StreamChannel.data_ptr   = tx_StreamChannel.ring_buffer;
        tx_StreamChannel.sample_ptr = tx_StreamChannel.data_ptr;

        rx_StreamChannel.data_ptr   = rx_StreamChannel.ring_buffer;
        rx_StreamChannel.sample_ptr = rx_StreamChannel.data_ptr;
        PipeCom_Init();
    }
}

// This function is directly called by Luos_utils in case of curent node assert. DO NOT RENAME IT
void node_assert(char *file, uint32_t line)
{
    // manage self crashing scenario
    char json[512];
    sprintf(json, "{\"assert\":{\"node_id\":1,\"file\":\"%s\",\"line\":%d}}\n", file, (unsigned int)line);
    Streaming_PutSample(&tx_StreamChannel, json, strlen(json));

    // Send the message
    PipeCom_Send();
}
/******************************************************************************
 * @brief get TX (Luos to outside) StreamChannel
 * @param None
 * @return streaming_channel_t*
 ******************************************************************************/
streaming_channel_t *Pipe_GetTxStreamChannel(void)
{
    return &tx_StreamChannel;
}
/******************************************************************************
 * @brief get RX (outside to Luos) StreamChannel
 * @param None
 * @return streaming_channel_t*
 ******************************************************************************/
streaming_channel_t *Pipe_GetRxStreamChannel(void)
{
    return &rx_StreamChannel;
}
