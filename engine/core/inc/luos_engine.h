/******************************************************************************
 * @file luos_engine
 * @brief User functionalities of the Luos engine library
 * @author Luos
 ******************************************************************************/
#ifndef LUOS_ENGINE_H
#define LUOS_ENGINE_H

#include "luos_utils.h"
#include "luos_list.h"
#include "service_structs.h"
#include "routing_table.h"
#include "luos_od.h"
#include "streaming.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// Initialise package with a macro
#define LUOS_ADD_PACKAGE(_name) \
    Luos_AddPackage(_name##_Init, _name##_Loop);

#define LUOS_RUN() Luos_Run();

/******************************************************************************
 * @struct general_stats_t
 * @brief format all datas to be sent trough msg
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            luos_stats_t node_stat;
            service_stats_t service_stat;
        };
        uint8_t unmap[sizeof(luos_stats_t) + sizeof(service_stats_t)]; /*!< streamable form. */
    };
} general_stats_t;

/*******************************************************************************
 * Function
 ******************************************************************************/
void Luos_Init(void);
void Luos_Loop(void);

// ***************** Node management *****************
void Luos_ResetStatistic(void);
bool Luos_IsNodeDetected(void);
void Luos_SetVerboseMode(uint8_t mode);
void Luos_SetFilterState(uint8_t state, service_t *service);

// ***************** Package management *****************
void Luos_AddPackage(void (*Init)(void), void (*Loop)(void));
void Luos_Run(void);

// ***************** Service management *****************
service_t *Luos_CreateService(SERVICE_CB service_cb, uint8_t type, const char *alias, revision_t revision);
error_return_t Luos_UpdateAlias(service_t *service, const char *alias, uint16_t size);
void Luos_Detect(service_t *service);
void Luos_ServicesClear(void);

// ***************** Messaging management *****************
void Luos_Flush(void);

// *** Timestamping
time_luos_t Luos_Timestamp(void);
bool Luos_IsMsgTimstamped(msg_t *msg);
time_luos_t Luos_GetMsgTimestamp(msg_t *msg);

// *** Send
error_return_t Luos_SendMsg(service_t *service, msg_t *msg);
error_return_t Luos_SendTimestampMsg(service_t *service, msg_t *msg, time_luos_t timestamp);
void Luos_SendData(service_t *service, msg_t *msg, void *bin_data, uint16_t size);
void Luos_SendStreaming(service_t *service, msg_t *msg, streaming_channel_t *stream);
void Luos_SendStreamingSize(service_t *service, msg_t *msg, streaming_channel_t *stream, uint32_t max_size);
error_return_t Luos_TxComplete(void);

void Luos_SetExternId(service_t *service, target_mode_t target_mode, uint16_t target, uint16_t newid);

// *** Receive
error_return_t Luos_ReadMsg(service_t *service, msg_t **returned_msg);
error_return_t Luos_ReadFromService(service_t *service, int16_t id, msg_t **returned_msg);
int Luos_ReceiveData(service_t *service, msg_t *msg, void *bin_data);
error_return_t Luos_ReceiveStreaming(service_t *service, msg_t *msg, streaming_channel_t *stream);
uint16_t Luos_NbrAvailableMsg(void);
uint32_t Luos_GetSystick(void);
error_return_t Luos_TxComplete(void);
void Luos_ResetStatistic(void);
bool Luos_IsNodeDetected(void);
void Luos_AddPackage(void (*Init)(void), void (*Loop)(void));
void Luos_SetVerboseMode(uint8_t mode);
void Luos_SetFilterState(uint8_t state, service_t *service);
error_return_t Luos_TopicSubscribe(service_t *service, uint16_t topic);
error_return_t Luos_TopicUnsubscribe(service_t *service, uint16_t topic);
void Luos_Run(void);
void Luos_Detect(service_t *service);

#endif /* LUOS_ENGINE_H */
