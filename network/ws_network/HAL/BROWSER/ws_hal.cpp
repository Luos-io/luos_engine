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

namespace [[cheerp::genericjs]] client
{
    Promise *import(const String &path);
}

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

[[cheerp::genericjs]] void WsHAL_AddWsEventHandlers()
{
    clientWebsocket = new WebSocket(s_url);
    clientWebsocket->set_binaryType("arraybuffer");
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
        cheerp::Callback([](MessageEvent *e)
                         {
            auto *dataBuffer = (ArrayBuffer *)e->get_data();
            auto *dataArray = new Uint8Array(dataBuffer);
            std::vector<uint8_t> wasmData(dataArray->get_length());
            Uint8Array *wasmDataArray = cheerp::MakeTypedArray(wasmData.data(), wasmData.size());
            wasmDataArray->set(dataArray);
            WsHAL_ReceptionWeb(wasmData); }));
}

[[cheerp::genericjs]] void WsHAL_InitWeb()
{
    bool isBrowser;
    __asm__("typeof %1 !== 'undefined'" : "=r"(isBrowser) : "r"(&client::window));
    if (isBrowser)
    {
        WsHAL_AddWsEventHandlers();
    }
    else
    {
        client::import("ws")
            ->then(cheerp::Callback([](client::Object *lib)
                                    { __asm__("globalThis.WebSocket=%0.WebSocket" ::"r"(lib)); }))
            ->then(cheerp::Callback([]()
                                    { WsHAL_AddWsEventHandlers(); }));
    }
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
