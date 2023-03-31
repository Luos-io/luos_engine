/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdbool.h>
#include "pipe_com.h"
#include "luos_engine.h"
#include "luos_utils.h"
#include <mongoose.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef PIPE_WS_SERVER_ADDR
    #define PIPE_WS_SERVER_ADDR "ws://localhost:9342"
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *s_listen_on = PIPE_WS_SERVER_ADDR;
static struct mg_mgr mgr;                          // Event manager
static struct mg_connection *ws_connection = NULL; // Websocket connection
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief This RESTful server implements the following endpoints:
 *    /ws - upgrade to Websocket, and implement websocket server
 * @param mg_connection* c connection pointer
 * @param int ev event type
 * @param void* ev_data event data
 * @param void* fn_data function data
 * @return None
 ******************************************************************************/
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_WS_OPEN)
    {
        printf("Websocket connection opened\n");
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_http_match_uri(hm, "/ws"))
        {
            // Upgrade to websocket. From now on, a connection is a full-duplex
            // Websocket connection, which will receive MG_EV_WS_MSG events.
            mg_ws_upgrade(c, hm, NULL);
            ws_connection = c;
        }
        else
        {
            // Serve static files
            printf("Unmanaged request receive\n");
        }
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // Got websocket frame. Received data is wm->data. save it into the Pipe streaming channel
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        Streaming_PutSample(Pipe_GetRxStreamChannel(), wm->data.ptr, wm->data.len);
    }
    else if (ev == MG_EV_CLOSE)
    {
        ws_connection = NULL;
        printf("Websocket is disconnected \n");
    }
    (void)fn_data;
}

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Init(void)
{
    static bool first_init = true;
    if (first_init)
    {
        first_init = false;
        mg_mgr_init(&mgr); // Initialise event manager
        printf("Starting WS listener on %s/ws\n", s_listen_on);
        mg_http_listen(&mgr, s_listen_on, fn, NULL); // Create HTTP listener
    }
}

/******************************************************************************
 * @brief compute the size we can send
 * @param None
 * @return uint16_t size
 ******************************************************************************/
uint16_t PipeCom_GetSizeToSend(void)
{
    if ((Streaming_GetAvailableSampleNB(Pipe_GetTxStreamChannel())) > Streaming_GetAvailableSampleNBUntilEndBuffer(Pipe_GetTxStreamChannel()))
    {
        return Streaming_GetAvailableSampleNBUntilEndBuffer(Pipe_GetTxStreamChannel());
    }
    return Streaming_GetAvailableSampleNB(Pipe_GetTxStreamChannel());
}

/******************************************************************************
 * @brief We need to send something
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Send(void)
{
    uint32_t start_tick;
    if (ws_connection != NULL)
    {
        uint16_t size = PipeCom_GetSizeToSend();
        while (size != 0)
        {
            mg_ws_send(ws_connection, (const char *)Pipe_GetTxStreamChannel()->sample_ptr, size, WEBSOCKET_OP_BINARY);
            Streaming_RmvAvailableSampleNB(Pipe_GetTxStreamChannel(), size);
            size       = PipeCom_GetSizeToSend();
            start_tick = Luos_GetSystick();
            while (Luos_GetSystick() - start_tick < 2)
                ;
        }
    }
}

/******************************************************************************
 * @brief Check if a message is available
 * @param None
 * @return None
 ******************************************************************************/
uint8_t PipeCom_Receive(uint16_t *size)
{
    *size = Streaming_GetAvailableSampleNB(Pipe_GetRxStreamChannel());
    return (*size > 0);
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_Loop(void)
{
    mg_mgr_poll(&mgr, 10); // Infinite event loop
}
