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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 30
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4

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
struct mg_mgr mgr;            // Event manager
bool connection_done = false; // Event handler flips it to true
struct mg_connection *c;      // Client connection
static const char *s_url = "ws://192.33.0.75:8000/ws";

/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_CRCInit(void);
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_RegisterPTP(void);

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
    if (c && connection_done)
    {
        mg_mgr_poll(&mgr, 1000);
    }
}

// Print websocket response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_OPEN)
    {
        c->is_hexdumping = 1;
        printf("This is really working\n");
    }
    else if (ev == MG_EV_ERROR)
    {
        // On error, log error message
        MG_ERROR(("%p %s", c->fd, (char *)ev_data));
    }
    else if (ev == MG_EV_WS_OPEN)
    {
        // When websocket handshake is successful, send message
        printf("Handshake is OK\n");
        // mg_ws_send(c, "hello", 5, WEBSOCKET_OP_TEXT);
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // When we get echo response, print it
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        printf("GOT ECHO REPLY: [%.*s]\n", (int)wm->data.len, wm->data.ptr);
    }

    if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_MSG)
    {
        *(bool *)fn_data = true; // Signal that we're done
    }
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    mg_mgr_init(&mgr);                                          // Initialise event manager
    mg_log_set(MG_LL_DEBUG);                                    // Set log level
    c = mg_ws_connect(&mgr, s_url, fn, &connection_done, NULL); // Create client
    if (c)
    {
        printf("Connected to %s\n", s_url);
    }
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
    // We consider this information sent
    Recep_Timeout();
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
}

/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
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
