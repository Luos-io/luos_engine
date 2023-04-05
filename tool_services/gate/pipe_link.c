/******************************************************************************
 * @file pipe_link
 * @brief Manage the communication with a pipe.
 * @author Luos
 ******************************************************************************/
#include "pipe_link.h"
#include "streaming.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint16_t pipe_id                         = 0;
streaming_channel_t *PipeDirectPutSample = NULL;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief send  message to the connected pipe
 * @param service pointer, data to send, size of data to send
 * @return None
 ******************************************************************************/
void PipeLink_Send(service_t *service, void *data, uint32_t size)
{
    LUOS_ASSERT(pipe_id > 0);
    msg_t msg;
    msg.header.target      = pipe_id;
    msg.header.cmd         = SET_CMD;
    msg.header.target_mode = SERVICEIDACK;
    if (PipeDirectPutSample == 0)
    {
        // We are not using localhost send the entire data trough the Luos network
        Luos_SendData(service, &msg, data, size);
    }
    else
    {
        // We have a localhost pipe
        // Copy the data directly into the local streaming channel without passing by Luos.
        Streaming_PutSample(PipeDirectPutSample, data, size);
        // Send a void set_cmd to strat data transmission on pipe.
        msg.header.size = 0;
        Luos_SendMsg(service, &msg);
    }
}
/******************************************************************************
 * @brief find a pipe and get its id
 * @param service pointer
 * @return pipe_id
 ******************************************************************************/
uint16_t PipeLink_Find(service_t *service)
{
    search_result_t result;
    uint8_t localhost = false;
    // search a pipe type in localhost
    RTFilter_Node(RTFilter_Type(RTFilter_Reset(&result), PIPE_TYPE), RoutingTB_NodeIDFromID(service->id));

    if (result.result_nbr > 0)
    {
        // we found a pipe in the same node
        localhost = true;
    }
    else
    {
        // pipe is not in the same node
        RTFilter_Type(RTFilter_Reset(&result), PIPE_TYPE);
        if (result.result_nbr == 0)
        {
            // No pipe found
            return 0;
        }
    }
    // keep pipe_id
    pipe_id = result.result_table[0]->id;

    if (pipe_id > 0)
    {
        // We find one, ask it to auto-update at 1000Hz
        msg_t msg;
        msg.header.target      = pipe_id;
        msg.header.target_mode = SERVICEIDACK;
        time_luos_t time       = TimeOD_TimeFrom_s(0.001f);
        TimeOD_TimeToMsg(&time, &msg);
        msg.header.cmd = UPDATE_PUB;
        while (Luos_SendMsg(service, &msg) != SUCCEED)
            ;

        if (localhost)
        {
            // This is a localhost pipe
            // Ask for a Streaming channel
            msg_t msg;
            msg.header.target      = pipe_id;
            msg.header.target_mode = SERVICEIDACK;
            msg.header.cmd         = PARAMETERS;
            msg.header.size        = 0;
            LUOS_ASSERT(service->id != 0);
            while (Luos_SendMsg(service, &msg) != SUCCEED)
                ;
        }
    }
    return pipe_id;
}
/******************************************************************************
 * @brief reset the connection of pipe
 * @param service pointer, msg received
 * @return None
 ******************************************************************************/
void PipeLink_Reset(service_t *service)
{
    LUOS_ASSERT(pipe_id > 0);
    msg_t msg;
    msg.header.target      = pipe_id;
    msg.header.cmd         = REINIT;
    msg.header.target_mode = SERVICEIDACK;
    msg.header.size        = 0;
    Luos_SendMsg(service, &msg);
}
/******************************************************************************
 * @brief set fonction to direct send in buffer if pipe is in localhost
 * @param streaming channel of pipe
 * @return None
 ******************************************************************************/
void PipeLink_SetDirectPipeSend(void *PipeSend)
{
    PipeDirectPutSample = PipeSend;
}
/******************************************************************************
 * @brief get id from pipe
 * @param None
 * @return pipe_id
 ******************************************************************************/
uint16_t PipeLink_GetId(void)
{
    return pipe_id;
}
