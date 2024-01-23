/******************************************************************************
 * @file ws_network.c
 * @brief Websocket communication driver for Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

/******************************************************************************
 * # WebSocket Topology management algorythm:
 *
 * +----+-----+          +------------+--------------+          +--------------+------------+          +--------------+------------+
 * |          |          |     node 1 (master)       |          |          node 2           |          |           node 3          |
 * |  Broker  |          +------------+--------------+          +--------------+------------+          +--------------+------------+
 * |          |          |  luos_phy  |  ws_network  |          |   luos_phy   | ws_network |          |   luos_phy   | ws_network |
 * +----------+          +------------+--------------+          +--------------+------------+          +--------------+------------+
 * |          |          | topo_run --+-->           |          |              |            |          |              |            |
 * |        <=+==========+============+=== PING      |          |              |            |          |              |            |
 * |   OK   ==+==========+============+==>           |          |              |            |          |              |            |
 * |   PING ==+==========+============+==============+==========+==============+==>         |          |              |            |
 * |          |          |            |              |          |      <-------+-topo_source|          |              |            |
 * |        <=+==========+============+==============+==========+==============+=== end     |          |              |            |
 * |          |          |            |              |          |      <-------+--topo_done |          |              |            |
 * |   END  ==+==========+============+===>          |          |              |            |          |              |            |
 * |          |          |      <-----+--- topo_next |          |              |            |          |              |            |
 * |          |          | topo_run --+-->           |          |              |            |          |              |            |
 * |        <=+==========+============+=== PING      |          |              |            |          |              |            |
 * |   OK   ==+==========+============+==>           |          |              |            |          |              |            |
 * |   PING ==+==========+============+==============+==========+==============+============+==========+==============+==>         |
 * |          |          |            |              |          |              |            |          |      <-------+-topo_source|
 * |        <=+==========+============+==============+==========+==============+============+==========+==============+=== end     |
 * |          |          |            |              |          |              |            |          |      <-------+--topo_done |
 * |   END  ==+==========+============+===>          |          |              |            |          |              |            |
 * |          |          |      <-----+--- topo_next |          |              |            |          |              |            |
 * |          |          | topo_run --+-->           |          |              |            |          |              |            |
 * |        <=+==========+============+=== PING      |          |              |            |          |              |            |
 * |   KO   ==+==========+============+==>           |          |              |            |          |              |            |
 * |          |          |      <-----+--- topo_end  |          |              |            |          |              |            |
 * +----------+          +------------+--------------+          +--------------+------------+          +--------------+------------+
 *
 * ==> Websocket messages
 * --> Function calling
 *
 * This algorythm is used to detect all the nodes on the network.
 * All nodes will be detected as star connection.
 * The broker doesn't apear on the Luos_engine architecture, it's just the server allowing to broadcast messages.
 *
 ******************************************************************************/

#include "luos_phy.h"
#include "ws_network.h"
#include "_ws_network.h"
#include "ws_hal.h"

#include <stdio.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    INACTIVE,
    WAITING,
    PING_ACKED,
    PING_NACKED,
    PING_RECEIVED
} wait_ack_t;

typedef enum
{
    PING = 0,
    END  = 1,
    OK   = 2,
    NOK  = 3
} ping_t;

// Phy callback definitions
static void Ws_JobHandler(luos_phy_t *phy_ptr, phy_job_t *job);
static error_return_t Ws_RunTopology(luos_phy_t *phy_ptr, uint8_t *portId);
static void Ws_Reset(luos_phy_t *phy_ptr);

/*******************************************************************************
 * Variables
 ******************************************************************************/
luos_phy_t *phy_ws              = NULL;
volatile wait_ack_t ping_status = INACTIVE; // This flag indicate the status of the ping request

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Initialisation of the WebSocket communication
 * @param None
 * @return None
 ******************************************************************************/
PUBLIC void Ws_Init(void)
{
    // Instantiate the phy struct
    phy_ws = Phy_Create(Ws_JobHandler, Ws_RunTopology, Ws_Reset);
    LUOS_ASSERT(phy_ws);

    Ws_Reset(phy_ws);
    WsHAL_Init();
}

/******************************************************************************
 * @brief Reset the WebSocket communication variables
 * @return None
 ******************************************************************************/
void Ws_Reset(luos_phy_t *phy_ptr)
{
    // Init the WebSocket communication variables
    ping_status = INACTIVE;
}

/******************************************************************************
 * @brief Loop of the WebSocket communication
 * @param None
 * @return None
 ******************************************************************************/
PUBLIC void Ws_Loop(void)
{
    WsHAL_Loop();
}

/******************************************************************************
 * @brief WebSocket job handler
 * @param phy_ptr
 * @param job
 * @return None
 ******************************************************************************/
void Ws_JobHandler(luos_phy_t *phy_ptr, phy_job_t *job)
{
    // Send the message
    WsHAL_Send(job->data_pt, job->size);
    // Free the message
    Phy_RmJob(phy_ws, job);
}

/******************************************************************************
 * @brief Store received data in the rx buffer
 * @param data data to store
 * @param size size of the data
 * @return None
 *****************************************************************************/
_CRITICAL void Ws_Reception(uint8_t *data, uint32_t size)
{
    LUOS_ASSERT((size <= sizeof(msg_t)) && (data != NULL));
    if (size >= sizeof(header_t))
    {
        // Reception is finished, we can give it to luos_engine
        phy_ws->rx_timestamp = Phy_GetTimestamp();
        // Only give the header to begin
        phy_ws->rx_data        = data;
        phy_ws->rx_buffer_base = data;
        phy_ws->received_data  = sizeof(header_t);
        Phy_ComputeHeader(phy_ws);
        if (phy_ws->rx_keep == true)
        {
            // Header computing need to get this message
            phy_ws->received_data = size;
            // Validate it
            Phy_ValidMsg(phy_ws);
            if (phy_ws->rx_data == NULL)
            {
                // The message wasn't kept, there is no more space on the luos_engine buffer, or the message is corrupted.
                // This is a critical failure
                LUOS_ASSERT(0);
            }
        }
    }
    else
    {
        LUOS_ASSERT(size == 1); // if size is not 1 or sizeof(header_t), this is a critical failure
        switch (data[0])
        {
            case PING:
                // This is a ping message
                // We receive this ping from a master node
                // This port become the topology source of this node
                // Notify luos_phy about it
                Phy_TopologySource(phy_ws, 0);
                ping_status = PING_RECEIVED;
                // Luos will try to detect other networks, then try to find someone else on this one by calling the `Ws_RunTopology` function.
                // This function will send an end message indicating that this branch detection is over.
                break;
            case END:
                // This is an end message, we receive it because we are the detection source of this phy.
                // This port is done, we just have to wait for the next port request from Luos. (In Websocket network, we consider each node reachable by this phy as a port, to get a star topology)
                Phy_TopologyNext();
                break;
            case OK:
                // This is a OK message comming from the broker indicating that we find someone.
                LUOS_ASSERT(ping_status == WAITING); // We should be waiting for a ping ack
                ping_status = PING_ACKED;
                break;
            case NOK:
                // This is a NOK message comming from the broker indicating that we didn't find someone.
                LUOS_ASSERT(ping_status == WAITING); // We should be waiting for a ping ack
                ping_status = PING_NACKED;
                break;
            default:
                LUOS_ASSERT(0); // This is a critical failure
                break;
        }
    }
}

/******************************************************************************
 * @brief Find the next neighbour on this phy
 * @param None
 * @return error_return_t
 ******************************************************************************/
error_return_t Ws_RunTopology(luos_phy_t *phy_ptr, uint8_t *portId)
{
    static uint8_t port_id = 0;
    if (ping_status == PING_RECEIVED)
    {
        // We already received a ping on this phy, we need to send an end message indicating that this branche is done
        uint8_t end = END;
        WsHAL_Send(&end, 1);
        // Consider this branch as done
        Phy_TopologyDone(phy_ws);
        // reset the port counter
        port_id     = 0;
        ping_status = INACTIVE;
        return FAILED;
    }
    else if (ping_status == INACTIVE)
    {
        // No one pinged us, this node is the one that initiate the ping
        // We need to send a ping to the broker
        uint8_t ping = PING;
        WsHAL_Send(&ping, 1);
        // We need to wait for the broker answer
        ping_status      = WAITING;
        uint32_t timeout = LuosHAL_GetSystick() + 200;
        while ((ping_status == WAITING) && (LuosHAL_GetSystick() < timeout))
        {
            // Treat potential received data
            Ws_Loop();
        }
        // Check if we received the answer
        switch (ping_status)
        {
            case WAITING:
                // We didn't receive the answer, meaning that we loose the broker consider this branch as done
                Phy_TopologyDone(phy_ws);
                printf("ERROR : can't reach the broker\n");
                port_id     = 0;
                ping_status = INACTIVE;
                return FAILED;
                break;
            case PING_ACKED:
                // We found someone
                // Reference this port ID
                *portId = port_id;
                port_id++;
                ping_status = INACTIVE;
                return SUCCEED;
                break;
            case PING_NACKED:
                // We didn't find someone
                // Consider this branch as done
                Phy_TopologyDone(phy_ws);
                port_id     = 0;
                ping_status = INACTIVE;
                return FAILED;
                break;
            default:
                LUOS_ASSERT(0); // This is a critical failure
                break;
        }
    }
    LUOS_ASSERT(0); // This is a critical failure
    return FAILED;
}
