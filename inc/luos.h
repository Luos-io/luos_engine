/******************************************************************************
 * @file luos
 * @brief User functionalities of the Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef LUOS_H
#define LUOS_H

#include "module_list.h"
#include "module_structs.h"
#include "routingTable.h"
#include "luos_od.h"
#include "streaming.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void Luos_Init(void);
void Luos_Loop(void);
void Luos_ModulesClear(void);
module_t *Luos_CreateModule(MOD_CB mod_cb, uint8_t type, const char *alias, char *firm_revision);
void Luos_ModuleEnableRT(module_t *module);
uint8_t Luos_SendMsg(module_t *module, msg_t *msg);
msg_t *Luos_ReadMsg(module_t *module);
msg_t *Luos_ReadFromModule(module_t *module, short id);
uint8_t Luos_NbrAvailableMsg(void);
uint8_t Luos_SendData(module_t *module, msg_t *msg, void *bin_data, uint16_t size);
uint8_t Luos_ReceiveData(module_t *module, msg_t *msg, void *bin_data);
uint8_t Luos_SendStreaming(module_t *module, msg_t *msg, streaming_channel_t *stream);
uint8_t Luos_ReceiveStreaming(module_t *module, msg_t *msg, streaming_channel_t *stream);
void Luos_SetBaudrate(module_t *module, uint32_t baudrate);

#endif /* LUOS_H */
