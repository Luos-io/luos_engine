/******************************************************************************
 * @file data manager
 * @brief data manager for inspector
 * @author Luos
 ******************************************************************************/
#include "pipe_link.h"
#include "data_manager.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_ASSERT_NUMBER  3
#define MAX_TOTAL_MSG_SIZE 135
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t inspector_state = STARTED;
// assert messages counter
uint8_t assert_num = 0;
// assert buffer for storing the messages
uint8_t assert_buf[MAX_ASSERT_NUMBER][MAX_TOTAL_MSG_SIZE];
// table with all the complete sizes of the assert messages
uint8_t assert_buf_size[MAX_ASSERT_NUMBER] = {0};
/*******************************************************************************
 * Function
 ******************************************************************************/
/******************************************************************************
 * @brief function to send the full routing table
 * @param service pointer
 * @return None
 ******************************************************************************/
void DataManager_SendRoutingTB(service_t *service)
{
    search_result_t result;
    uint8_t data[MAX_RTB_ENTRY * sizeof(routing_table_t)] = {0};

    RTFilter_Service(RTFilter_Reset(&result), service);
    // store the address of the RoutingTB
    routing_table_t *routing_table = RoutingTB_Get();
    msg_t msg;
    msg.header.config      = BASE_PROTOCOL;
    msg.header.target      = DEFAULTID;
    msg.header.target_mode = SERVICEID;
    msg.header.source      = result.result_table[0]->id;
    msg.header.cmd         = RTB;
    msg.header.size        = RoutingTB_GetLastEntry() * sizeof(routing_table_t);

    memcpy(data, msg.stream, sizeof(header_t));
    memcpy(&data[sizeof(header_t)], routing_table, msg.header.size);

    PipeLink_Send(service, data, msg.header.size + sizeof(header_t));
}
/******************************************************************************
 * @brief extract the command value for the pipe messages
 * @param service pointer
 * @return None
 ******************************************************************************/
uint8_t DataManager_ExtractCommand(msg_t *msg)
{
    return msg->data[4];
}
/******************************************************************************
 * @brief function to pull the external messages from the pipe
 * @param service pointer
 * @return None
 ******************************************************************************/
void DataManager_GetPipeMsg(service_t *service, msg_t *data_msg)
{
    int *pointer;
    msg_t msg;
    uint8_t cmd;
    // pipe sent the streaming channel
    if (data_msg->header.cmd == PARAMETERS)
    {
        // link the address of the streaming channel L2P
        memcpy(&pointer, data_msg->data, sizeof(void *));
        PipeLink_SetDirectPipeSend((void *)pointer);
        return;
    }

    cmd = DataManager_ExtractCommand(data_msg);
    //  This message is a command from pipe
    switch (cmd)
    {
        case ASK_DETECTION:
            Luos_Detect(service);
            break;
        case RTB:
            // first message for the inspector
            // send the routing table using pipe
            DataManager_SendRoutingTB(service);
            break;
        case CONTROL:
            // if we receive a CONTROL we should desactivate or activate the filtering
            if (data_msg->data[7] == true)
            {
                // check if data is true so that the inspector needs to start
                Luos_SetFilterState(false, service);
                inspector_state = STARTED;
            }
            else
            {
                // if we receive a false we should reactivate the filtering - inspector stopped
                Luos_SetFilterState(true, service);
                inspector_state = STOPPED;
            }
            break;
        case LUOS_STATISTICS:
            // extract service that we want the stats
            msg.header.target      = (data_msg->data[8] << 8) + data_msg->data[7];
            msg.header.target_mode = SERVICEID;
            msg.header.cmd         = LUOS_STATISTICS;
            msg.header.size        = 0;
            Luos_SendMsg(service, &msg);
            break;
        case LUOS_REVISION:
            // extract service that we want the luos revision
            msg.header.target      = (data_msg->data[8] << 8) + data_msg->data[7];
            msg.header.target_mode = SERVICEID;
            msg.header.cmd         = LUOS_REVISION;
            msg.header.size        = 0;
            Luos_SendMsg(service, &msg);
            break;
        case REVISION:
            // extract service that we want the firmware revision
            msg.header.target      = (data_msg->data[8] << 8) + data_msg->data[7];
            msg.header.target_mode = SERVICEID;
            msg.header.cmd         = REVISION;
            msg.header.size        = 0;
            Luos_SendMsg(service, &msg);
            break;
        case ASSERT:
            if (((data_msg->data[6] << 8) + data_msg->data[5]) == 0)
            {
                for (uint8_t i = 0; i < assert_num; i++)
                {
                    //  send all the existing assert messages
                    PipeLink_Send(service, &assert_buf[i][0], assert_buf_size[i]);
                }
                assert_num = 0;
            }
            // empty the assert messages buffer by turning the counter to 0
            break;
        default:
            break;
    }
}
/******************************************************************************
 * @brief function to pull the messages from services
 * @param service pointer
 * @return None
 ******************************************************************************/
void DataManager_GetServiceMsg(service_t *service)
{
    // loop into services.
    msg_t *data_msg;
    search_result_t result;
    uint8_t i = 0;

    RTFilter_Reset(&result);
    while (i < result.result_nbr)
    {
        // pull available messages
        if (Luos_ReadFromService(service, result.result_table[i]->id, &data_msg) == SUCCEED)
        {
            // drop the messages that are destined to pipe
            if (data_msg->header.target == PipeLink_GetId())
            {
                i++;
                continue;
            }
            // Check if this is a message from pipe
            if (data_msg->header.source == PipeLink_GetId())
            {
                // treat message from pipe
                DataManager_GetPipeMsg(service, data_msg);
                i++;
                continue;
            }
            // check if this is an assert
            if ((data_msg->header.cmd == ASSERT) && (data_msg->header.size > 0))
            {
                if (assert_num >= MAX_ASSERT_NUMBER)
                {
                    // if we reached the maximum number of asserts delete the older and keep the newer
                    for (uint8_t j = 1; j < MAX_ASSERT_NUMBER; j++)
                    {
                        memcpy(&assert_buf[j - 1][0], &assert_buf[j][0], assert_buf_size[j]);
                        assert_buf_size[j - 1] = assert_buf_size[j];
                    }
                    assert_num--;
                }
                // save the assert message to the assert messages buffer
                memcpy(&assert_buf[assert_num][0], data_msg->stream, sizeof(header_t) + data_msg->header.size);
                // store the size of this message
                assert_buf_size[assert_num] = sizeof(header_t) + data_msg->header.size;
                assert_num++;
                i++;
                continue;
            }
            if (data_msg->header.cmd == END_DETECTION)
            {
                PipeLink_Find(service);
                i++;
                continue;
            }
            if (data_msg->header.cmd == ASK_DETECTION)
            {
                i++;
                continue;
            }
            // send any other message to pipe
            PipeLink_Send(service, data_msg->stream, (sizeof(uint8_t) * data_msg->header.size) + sizeof(header_t));
        }
        i++;
    }
}
/******************************************************************************
 * @brief get if the inspector is started or stopped
 * @param service pointer
 * @return None
 ******************************************************************************/
uint8_t DataManager_GetInspectorState(void)
{
    return inspector_state;
}
