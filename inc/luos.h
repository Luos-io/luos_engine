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

/******************************************************************************
 * @struct luos_stats_t
 * @brief store informations about luos stats
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            memory_stats_t memory;
            uint8_t max_loop_time_ms;
        };
        uint8_t unmap[5]; /*!< streamable form. */
    };
} luos_stats_t;

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
error_return_t Luos_ReadMsg(module_t *module, msg_t **returned_msg);
error_return_t Luos_ReadFromModule(module_t *module, int16_t id, msg_t **returned_msg);
uint8_t Luos_SendData(module_t *module, msg_t *msg, void *bin_data, uint16_t size);
uint8_t Luos_ReceiveData(module_t *module, msg_t *msg, void *bin_data);
uint8_t Luos_SendStreaming(module_t *module, msg_t *msg, streaming_channel_t *stream);
uint8_t Luos_ReceiveStreaming(module_t *module, msg_t *msg, streaming_channel_t *stream);
void Luos_SetBaudrate(uint32_t baudrate);
void Luos_SendBaudrate(module_t *module, uint32_t baudrate);
uint8_t Luos_SetExternId(module_t *module, target_mode_t target_mode, uint16_t target, uint16_t newid);
uint16_t Luos_NbrAvailableMsg(void);

#endif /* LUOS_H */
