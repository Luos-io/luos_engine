/******************************************************************************
 * @file robusHAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @Family x86/Linux/Mac
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include <WiFi101.h>
#include <ArduinoHttpClient.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "ws_hal.h"
#include "_ws_network.h"
#include "luos_utils.h"
#ifdef __cplusplus
}
#endif

WiFiClient wifi;
static const char *s_url  = WS_NETWORK_BROKER_ADDR; // ws://192.168.10.39:8000/
const char *serverAddress = &s_url[5];
// char serverAddress[] = "echo.websocket.org"; // server address
// int port             = 80;

WebSocketClient *client;
int status = WL_IDLE_STATUS;

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Init(void)
{
    // Extract IP  and port from URL
    int ip_table[4];
    int port;
    sscanf(s_url, "ws://%d.%d.%d.%d:%d", &ip_table[0], &ip_table[1], &ip_table[2], &ip_table[3], &port);
    IPAddress ip_serv = IPAddress(ip_table[0], ip_table[1], ip_table[2], ip_table[3]);

    // Create a client instance
    static WebSocketClient aclient = WebSocketClient(wifi, ip_serv, port);
    client                         = &aclient;

    Serial.begin(9600);
    if (!Serial)
        delay(3000);
    // Connect to WIFi:
    while (WiFi.status() != WL_CONNECTED)
    {
        char ssid[] = SECRET_SSID;
        char pass[] = SECRET_PASS;
        Serial.print("Attempting to connect to: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network:
        status = WiFi.begin(ssid, pass);
    }

    // Print the SSID of the network you're attached to:
    Serial.print("Connected to SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}

/******************************************************************************
 * @brief Luos HAL general loop
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Loop(void)
{
    // if not connected to the socket server, try to connect:
    if (!client->connected())
    {
        Serial.print("Connecting to the broker on: ");
        Serial.println(s_url);
        client->begin();
        delay(1000);
        if (!client->connected())
        {
            Serial.println("Failed to connect to the broker");
            return;
        }
        else
        {
            Serial.println("Connected to the broker");
        }
        // skip the rest of the loop:
        return;
    }
    // Check for received messages
    int messageSize = client->parseMessage();
    while (messageSize > 0)
    {
        // Transform the incoming data into a message
        uint8_t msg[200];
        if (client->read(msg, messageSize) != messageSize)
        {
            Serial.println("Error while reading message");
            return;
        }
        Serial.print("Received message: ");
        Ws_Reception(msg, (uint32_t)messageSize);
        messageSize = client->parseMessage();
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
    client->beginMessage(TYPE_BINARY);
    client->write(data, size);
    if (client->endMessage() == 0)
    {
        Serial.println("Message sent");
    }
    else
    {
        Serial.println("Error while sending message");
    }
}
