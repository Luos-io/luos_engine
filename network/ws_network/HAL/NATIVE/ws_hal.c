/******************************************************************************
 * @file robusHAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @Family x86/Linux/Mac
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "ws_hal.h"
#include "_ws_network.h"
#include "mongoose.h"
#include "luos_utils.h"

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// #define WS_PRINT
static void ws_cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
static void *WSThread(void *vargp);

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint8_t *tx_data = 0;

// Mongoose connection information
struct mg_mgr ws_mgr;    // Event manager
struct mg_connection *c; // Client connection
static const char *s_url   = WS_NETWORK_BROKER_ADDR;
volatile bool ws_connected = false;
/*******************************************************************************
 * Function
 ******************************************************************************/

/* msleep(): Sleep for the requested number of milliseconds. */
static int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec  = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Init(void)
{
    static bool connection_done = false;                                      // Event handler flips it to true
    mg_mgr_init(&ws_mgr);                                                     // Initialise event manager
    mg_log_set(MG_LL_NONE);                                                   // Set log level => MG_LL_VERBOSE
    c = mg_ws_connect(&ws_mgr, s_url, ws_cb, (void *)&connection_done, NULL); // Create client
    if (c)
    {
        while (connection_done == false)
        {
            // Wait for connection to be established
            mg_mgr_poll(&ws_mgr, 1000);
        }
    }
    // Create a thread to poll the websocket.
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, WSThread, NULL);
}

/******************************************************************************
 * @brief Luos HAL general loop
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Loop(void)
{
    // Just sleep to avoid 100% CPU usage and websockets overflows
    msleep(1);
}

void *WSThread(void *vargp)
{
    while (1)
    {
        msleep(1);
        if (c)
        {
            mg_mgr_poll(&ws_mgr, 10);
        }
    }
    return NULL;
}

// Print websocket response and signal that we're done
void ws_cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
#ifdef WS_PRINT
    if (ev == MG_EV_OPEN)
    {
        printf("opened\n");
    }
    else if (ev == MG_EV_CONNECT)
    {
        printf("connected\n");
    }
    else
#endif
        if (ev == MG_EV_CLOSE)
    {
        printf("Connection closed\n");
    }
    else if (ev == MG_EV_ERROR)
    {
        // On error, log error message
        printf("Error:");
        MG_ERROR(("%p %s", c->fd, (char *)ev_data));
    }
    else if (ev == MG_EV_WS_OPEN)
    {
        // When websocket handshake is successful, send message
        printf("Connected to %s\n", s_url);
        ws_connected = true;
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // Transform the incoming data into a message
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        Ws_Reception((uint8_t *)wm->data.ptr, (uint32_t)wm->data.len);
    }

    if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_OPEN)
    {
        *(bool *)fn_data = true; // Signal that we're connected
    }
}

/******************************************************************************
 * @brief Transmit data
 * @param data to send
 * @param size of data to send
 * @return None
 ******************************************************************************/
void WsHAL_Send(const uint8_t *data, uint16_t size)
{
    mg_ws_send(c, data, size, WEBSOCKET_OP_BINARY);
}
