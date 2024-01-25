/******************************************************************************
 * @file wsHAL
 * @brief WebSocket Hardware Abstration Layer. Describe Low layer fonction
 * @Family x86/Linux/Mac
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
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

#include <cheerp/client.h>
#include <cheerp/clientlib.h>

using namespace client;

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static const char *s_url = WS_NETWORK_BROKER_ADDR;
[[cheerp::genericjs]] WebSocket *clientWebsocket;

/*******************************************************************************
 * Function
 ******************************************************************************/

[[cheerp::genericjs]] void openEventCallback()
{
    console.log("Websocket opened on", s_url);
}

[[cheerp::genericjs]] void closeEventCallback()
{
    console.log("Websocket closed.");
}

[[cheerp::genericjs]] void errorEventCallback()
{
    console.log("Websocket error.");
}

void WsHAL_ReceptionWeb(const std::vector<uint8_t> &data)
{
    Ws_Reception((uint8_t *)data.data(), data.size());
}

[[cheerp::genericjs]] void WsHAL_SendWeb(const std::vector<uint8_t> &data)
{
    // Convert the data to ArrayBufferView*
    ArrayBufferView *arrayBufferView = cheerp::MakeTypedArray(data.data(), data.size());
    clientWebsocket->send(arrayBufferView);
}

[[cheerp::genericjs]] void WsHAL_InitWeb()
{
    __asm__("const WebSocket = require('ws')");
    clientWebsocket = new WebSocket(s_url);
    clientWebsocket->addEventListener(
        "open",
        cheerp::Callback(openEventCallback));
    clientWebsocket->addEventListener(
        "close",
        cheerp::Callback(closeEventCallback));
    clientWebsocket->addEventListener(
        "error",
        cheerp::Callback(errorEventCallback));

    clientWebsocket->addEventListener(
        "message",
        cheerp::Callback(
            [](MessageEvent *e)
            {
                auto *dataArray = (Uint8Array *)e->get_data();
                std::vector<uint8_t> wasmData(dataArray->get_length());                               // allocate some linear memory
                Uint8Array *wasmDataArray = cheerp::MakeTypedArray(wasmData.data(), wasmData.size()); // create a Uint8Array wrapper ove the linear memory held by the vector
                wasmDataArray->set(dataArray);                                                        // copy the data with the builtin set() function. A for() loop with element-wise copy would do but this is faster
                WsHAL_ReceptionWeb(wasmData);                                                         // NOTE: this now takes a const std::vector<uint8_t>& (or a std::vector<uint8_t> if you want to pass ownership)
            }));
}

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Init(void)
{
    // Create a client WebSocket instance to s_url
    WsHAL_InitWeb();
}

/******************************************************************************
 * @brief Luos HAL general loop
 * @param None
 * @return None
 ******************************************************************************/
void WsHAL_Loop(void)
{
    // Nothing to do here
}

/******************************************************************************
 * @brief Transmit data
 * @param data to send
 * @param size of data to send
 * @return None
 ******************************************************************************/
void WsHAL_Send(const uint8_t *data, uint16_t size)
{
    // Convert the data and size to a std::vector
    std::vector<uint8_t> wasmData(data, data + size);
    WsHAL_SendWeb(wasmData);
}
