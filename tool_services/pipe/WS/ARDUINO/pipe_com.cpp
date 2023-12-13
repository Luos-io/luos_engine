/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <WiFi.h>
#include <WebSocketsServer.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "pipe_com.h"
#include "luos_utils.h"

#ifdef __cplusplus
}
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STATION_SSID "yourNetworkName"
#define STATION_PASS "yourNetworkPassword"
#define STATION_PORT 80

/*******************************************************************************
 * Variables
 ******************************************************************************/
const char *ssid     = STATION_SSID;
const char *password = STATION_PASS;

WebSocketsServer webSocket = WebSocketsServer(STATION_PORT);

static volatile uint8_t ws_connection = false;
static volatile uint8_t ws_cli        = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    // Figure out the type of WebSocket event
    switch (type)
    {
        // Client has disconnected
        case WStype_DISCONNECTED:
            ws_connection = false;
            Serial.printf("[%u] Disconnected!\n", num);
            break;

        // New client has connected
        case WStype_CONNECTED:
        {
            ws_cli        = num;
            ws_connection = true;
            IPAddress ip  = webSocket.remoteIP(num);
            Serial.printf("[%u] Connection from ", num);
            Serial.println(ip.toString());
        }
        break;

        // Echo text message back to client
        case WStype_TEXT:
        case WStype_BIN:
            Serial.printf("[%u] Text: %s\n", num, payload);
            Streaming_PutSample(Pipe_GetRxStreamChannel(), payload, length);
            break;

        // For everything else: do nothing
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        default:
            break;
    }
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PipeCom_WifiConnection(void)
{
    Serial.begin(115200);
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("\nLocal ESP32 IP: ");
    Serial.println(WiFi.localIP());

    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
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
        PipeCom_WifiConnection();
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
    if (ws_connection)
    {
        uint16_t size = PipeCom_GetSizeToSend();
        while (size != 0)
        {
            webSocket.sendBIN(ws_cli, (uint8_t *)Pipe_GetTxStreamChannel()->sample_ptr, size);
            Streaming_RmvAvailableSampleNB(Pipe_GetTxStreamChannel(), size);
            size = PipeCom_GetSizeToSend();
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
    webSocket.loop();
}
