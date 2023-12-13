/******************************************************************************
 * @file pipe_com
 * @brief communication driver
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdbool.h>
#include "pipe_com.h"
#include "luos_utils.h"
#include <mongoose.h>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STATION_WIFI_SSID     "yourNetworkName"
#define STATION_WIFI_PASS     "yourNetworkPassword"
#define STATION_WIFI_PORT     80
#define STATION_MAXIMUM_RETRY 10

#ifndef PIPE_WS_SERVER_ADDR
    #define PIPE_WS_SERVER_ADDR "ws://192.168.1.0:9342"
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static char s_listen_on[64] = "ws://192.168.1.0:9342";
static struct mg_mgr mgr;                          // Event manager
static struct mg_connection *ws_connection = NULL; // Websocket connection

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;
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
        ESP_LOGI(TAG, "Websocket connection opened\n");
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
            ESP_LOGI(TAG, "Unmanaged request receive\n");
        }
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // Got websocket frame. Received data is wm->data. save it into the Pipe streaming channel
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        Streaming_PutSample(Pipe_GetRxStreamChannel(), wm->data.ptr, wm->data.len);
    }
    else if (ev == MG_EV_ERROR)
    {
        // On error, log error message
        MG_ERROR(("%p %s", c->fd, (char *)ev_data));
    }
    (void)fn_data;
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < STATION_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        sprintf(s_listen_on, "ws://%d.%d.%d.%d:%d", IP2STR(&event->ip_info.ip), STATION_WIFI_PORT);
    }
}
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void wifi_init_sta(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = STATION_WIFI_SSID,
            .password = STATION_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable  = true,
                .required = false},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 STATION_WIFI_SSID, STATION_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 STATION_WIFI_SSID, STATION_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
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

        wifi_init_sta();

        mg_mgr_init(&mgr);       // Initialise event manager
        mg_log_set(MG_LL_DEBUG); // Set log level
        ESP_LOGI(TAG, "Starting WS listener on %s/ws\n", s_listen_on);
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
    if (ws_connection != NULL)
    {
        uint16_t size = PipeCom_GetSizeToSend();
        while (size != 0)
        {
            mg_ws_send(ws_connection, (const char *)Pipe_GetTxStreamChannel()->sample_ptr, size, WEBSOCKET_OP_BINARY);
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
    mg_mgr_poll(&mgr, 0); // Infinite event loop
}