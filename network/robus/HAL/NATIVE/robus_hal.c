/******************************************************************************
 * @file robusHAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @Family x86/Linux/Mac
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "robus_hal.h"

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include "reception.h"
#include "context.h"

#include <mongoose.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "luos_engine.h"
#include "luos_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 30
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4
#ifndef WS_BROKER_ADDR
    #define WS_BROKER_ADDR "ws://127.0.0.1:8000"
#endif

// #define WS_PRINT

/*******************************************************************************
 * Variables
 ******************************************************************************/
typedef struct
{
    uint16_t Pin;
    uint8_t *Port; // STUB
    uint8_t IRQ;
} Port_t;

Port_t PTP[NBR_PORT];

volatile uint8_t *tx_data = 0;

// Mongoose connection information
struct mg_mgr robus_mgr; // Event manager
struct mg_connection *c; // Client connection
static const char *s_url = WS_BROKER_ADDR;
volatile bool ptpa       = false;
volatile bool ptpb       = false;
volatile bool ptp_update = true;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_CRCInit(void);
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_RegisterPTP(void);

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
void RobusHAL_Init(void)
{
    // IO Initialization
    RobusHAL_GPIOInit();

    // CRC Initialization
    RobusHAL_CRCInit();

    // Com Initialization
    RobusHAL_ComInit(DEFAULTBAUDRATE);
}

/******************************************************************************
 * @brief Luos HAL general loop
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Loop(void)
{
    // Just sleep to avoid 100% CPU usage and websockets overflows
    msleep(5);
}

void *WSrobusThread(void *vargp)
{
    while (1)
    {
        msleep(1);
        if (c)
        {
            mg_mgr_poll(&robus_mgr, 10);
        }
    }
    return NULL;
}

// Print websocket response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
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
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // Transform the incoming data into a message
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        // Check if the message is a PTP message
        if (wm->data.ptr[0] == 'P' & wm->data.ptr[1] == 'T' & wm->data.ptr[2] == 'P')
        {
#ifdef WS_PRINT
            printf("PTP%c state: %d\n", wm->data.ptr[3], wm->data.ptr[4] == '1');
#endif
            if (wm->data.ptr[3] == 'A')
            {
                ptpa = wm->data.ptr[4] == '1';
                if (ptp_update == true)
                {
                    PortMng_PtpHandler(0);
#ifdef WS_PRINT
                    printf("PTP event handler called for PTPA\n");
#endif
                }
            }
            else if (wm->data.ptr[3] == 'B')
            {
                ptpb = wm->data.ptr[4] == '1';
                if (ptp_update == true)
                {
                    PortMng_PtpHandler(1);
#ifdef WS_PRINT
                    printf("PTP event handler called for PTPB\n");
#endif
                }
            }
            ptp_update = true;
        }
        else
        {
// Message is not a PTP message, so it is a Luos message
#ifdef WS_PRINT
            printf("\n : Robus receive %d bytes:\n", (int)wm->data.len);
            msg_t *current_msg = (msg_t *)wm->data.ptr;
            printf("*******header data*******\n");
            printf("protocol : 0x%04x\n", current_msg->header.config);         /*!< Protocol version. */
            printf("target : 0x%04x\n", current_msg->header.target);           /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
            printf("target_mode : 0x%04x\n", current_msg->header.target_mode); /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
            printf("source : 0x%04x\n", current_msg->header.source);           /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
            printf("cmd : 0x%04x\n", current_msg->header.cmd);                 /*!< msg definition. */
            printf("size : 0x%04x\n", current_msg->header.size);               /*!< Size of the data field. */
#endif
            for (uint16_t i = 0; i < (int)wm->data.len; i++)
            {
                ctx.rx.callback((volatile uint8_t *)&wm->data.ptr[i]);
            }
#ifdef WS_PRINT
            printf("\n");
#endif
            // We consider this information received and acked
            Recep_Timeout();
        }
    }

    if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_OPEN)
    {
        *(bool *)fn_data = true; // Signal that we're connected
    }
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    static bool connection_done = false;                                      // Event handler flips it to true
    mg_mgr_init(&robus_mgr);                                                  // Initialise event manager
    mg_log_set(MG_LL_NONE);                                                   // Set log level => MG_LL_VERBOSE
    c = mg_ws_connect(&robus_mgr, s_url, fn, (void *)&connection_done, NULL); // Create client
    if (c)
    {
        while (connection_done == false)
        {
            // Wait for connection to be established
            mg_mgr_poll(&robus_mgr, 1000);
        }
    }
    // Create a thread to poll the websocket.
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, WSrobusThread, NULL);
}

/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
}

/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void RobusHAL_SetRxState(uint8_t Enable)
{
}

/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    // don't send ACK
    if (size > 1)
    {
#ifdef WS_PRINT
        printf("Transmiting %d bytes : %.*s\n\n", size, size, data);
#endif
        mg_ws_send(c, data, size, WEBSOCKET_OP_BINARY);
        // Avoid the need of ack
        ctx.tx.status = TX_OK;
        // Check if ack was needed
        msg_t *current_msg = (msg_t *)data;
        // We consider this information sent
        Recep_Timeout();
        // Check if this was a reset detection or a routing table message
        if ((current_msg->header.cmd == START_DETECTION) || (current_msg->header.cmd == LOCAL_RTB))
        {
            // wait to be sure everyone received it
            msleep(200);
        }
    }
}

/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
}

/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t RobusHAL_GetTxLockState(void)
{
    return 0;
}

/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
}

/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
}

/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
}

/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void RobusHAL_RegisterPTP(void)
{
#if (NBR_PORT >= 1)
    PTP[0].Pin  = PTPA_PIN;
    PTP[0].Port = PTPA_PORT;
    PTP[0].IRQ  = PTPA_IRQ;
#endif

#if (NBR_PORT >= 2)
    PTP[1].Pin  = PTPB_PIN;
    PTP[1].Port = PTPB_PORT;
    PTP[1].IRQ  = PTPB_IRQ;
#endif

#if (NBR_PORT >= 3)
    PTP[2].Pin  = PTPC_PIN;
    PTP[2].Port = PTPC_PORT;
    PTP[2].IRQ  = PTPC_IRQ;
#endif

#if (NBR_PORT >= 4)
    PTP[3].Pin  = PTPD_PIN;
    PTP[3].Port = PTPD_PORT;
    PTP[3].IRQ  = PTPD_IRQ;
#endif
}

/******************************************************************************
 * @brief Set PTP for Detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    uint32_t start_tick;

    if (c)
    {
#ifdef WS_PRINT
        printf("transmitting PTP%c release state: %d\n", PTPNbr + 'A', 0);
#endif
        // Send a PTP False message to the server
        if (PTPNbr == 0)
        {
            mg_ws_send(c, "PTPA0", sizeof("PTPA0"), WEBSOCKET_OP_TEXT);
            // Wait to avoid message bursts
            start_tick = LuosHAL_GetSystick();
            while (LuosHAL_GetSystick() - start_tick < 1)
                ;
        }
        else if (PTPNbr == 1)
        {
            mg_ws_send(c, "PTPB0", sizeof("PTPB0"), WEBSOCKET_OP_TEXT);
            // Wait to avoid message bursts
            start_tick = LuosHAL_GetSystick();
            while (LuosHAL_GetSystick() - start_tick < 1)
                ;
        }
    }
}

/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
}

/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    ptp_update = false;
}

/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    ptp_update = false;
#ifdef WS_PRINT
    printf("transmitting PTP%c POKE state: %d\n", PTPNbr + 'A', 1);
#endif
    // When this function is called, this means that we are poking the PTP line
    if (PTPNbr == 0)
    {
        // Ask PTPA state to the server
        mg_ws_send(c, "PTPA1", sizeof("PTPA1"), WEBSOCKET_OP_TEXT);
        // Wait for the server to acknowledge the PTP update
        while (ptp_update == false)
        {
            msleep(10);
        }
        return ptpa;
    }
    else if (PTPNbr == 1)
    {
        // Ask PTPB state to the server
        mg_ws_send(c, "PTPB1", sizeof("PTPB1"), WEBSOCKET_OP_TEXT);
        // Wait for the server to acknowledge the PTP update
        while (ptp_update == false)
        {
            msleep(10);
        }
        return ptpb;
    }
    return 0;
}

/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
}

/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
    for (uint8_t i = 0; i < 1; ++i)
    {
        uint16_t dbyte = data[i];
        *(uint16_t *)crc ^= dbyte << 8;
        for (uint8_t j = 0; j < 8; ++j)
        {
            uint16_t mix     = *(uint16_t *)crc & 0x8000;
            *(uint16_t *)crc = (*(uint16_t *)crc << 1);
            if (mix)
                *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
        }
    }
}
