/******************************************************************************
 * @file luos_engine
 * @brief User functionalities of the Luos engine library
 * @author Luos
 ******************************************************************************/
#ifndef LUOS_ENGINE_H
#define LUOS_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include "luos_utils.h"
#include "luos_list.h"
#include "struct_luos.h"
#include "routing_table.h"
#include "luos_od.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// Initialise package with a macro
#define LUOS_ADD_PACKAGE(_name) \
    Luos_AddPackage(_name##_Init, _name##_Loop);

#define LUOS_RUN() Luos_Run();

/*******************************************************************************
 * Function
 ******************************************************************************/
void Luos_Init(void);
void Luos_Loop(void);
void Luos_ResetStatistic(void);
const revision_t *Luos_GetVersion(void);

// ***************** Node management *****************
uint32_t Luos_GetSystick(void);
bool Luos_IsDetected(void);
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

// *** Streaming management ***
#include "streaming.h"
void Luos_SendStreaming(service_t *service, msg_t *msg, streaming_channel_t *stream);
void Luos_SendStreamingSize(service_t *service, msg_t *msg, streaming_channel_t *stream, uint32_t max_size);
error_return_t Luos_ReceiveStreaming(service_t *service, const msg_t *msg, streaming_channel_t *stream);

// *** Timestamping management (in file `timestamp.c`)***
time_luos_t Luos_Timestamp(void);
bool Luos_IsMsgTimstamped(const msg_t *msg);
time_luos_t Luos_GetMsgTimestamp(msg_t *msg);
error_return_t Luos_SendTimestampMsg(service_t *service, msg_t *msg, time_luos_t timestamp);

// *** Pub/Sub management (in file `pub_sub.c`)***
error_return_t Luos_Subscribe(service_t *service, uint16_t topic);
error_return_t Luos_Unsubscribe(service_t *service, uint16_t topic);

// *** Big data management ***
void Luos_SendData(service_t *service, msg_t *msg, void *bin_data, uint16_t size);
int Luos_ReceiveData(service_t *service, const msg_t *msg, void *bin_data);

// *** Basic transmission management ***
error_return_t Luos_SendMsg(service_t *service, msg_t *msg);
error_return_t Luos_TxComplete(void);

// *** Polling reception management ***
error_return_t Luos_ReadMsg(service_t *service, msg_t *msg_to_write);
error_return_t Luos_ReadFromService(service_t *service, uint16_t id, msg_t *msg_to_write);
uint16_t Luos_NbrAvailableMsg(void);

#endif /* LUOS_ENGINE_H */
