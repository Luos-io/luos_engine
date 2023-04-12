/******************************************************************************
 * @file msgAlloc
 * @brief Message reception allocator manager
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _MSGALLOC_H_
#define _MSGALLOC_H_

#include "struct_luos.h"
#include "struct_io.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CRC_SIZE 2

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

// generic functions
void MsgAlloc_Init(memory_stats_t *memory_stats);
void MsgAlloc_loop(void);

// msg buffering functions
uint8_t *MsgAlloc_Alloc(uint16_t data_size);

error_return_t MsgAlloc_IsEmpty(void);
void MsgAlloc_UsedMsgEnd(void);
void MsgAlloc_Reset(void);
error_return_t MsgAlloc_IsReseted(void);

// msg interpretation task stack
error_return_t MsgAlloc_PullMsgToInterpret(msg_t **returned_msg);

// Luos task stack
void MsgAlloc_LuosTaskAlloc(service_t *service_concerned_by_current_msg, msg_t *concerned_msg);

// Luos task research and pull
error_return_t MsgAlloc_PullMsg(service_t *target_service, msg_t **returned_msg);
error_return_t MsgAlloc_PullMsgFromLuosTask(uint16_t luos_task_id, msg_t **returned_msg);
error_return_t MsgAlloc_LookAtLuosTask(uint16_t luos_task_id, service_t **allocated_service);
error_return_t MsgAlloc_GetLuosTaskSourceId(uint16_t luos_task_id, uint16_t *source_id);
error_return_t MsgAlloc_GetLuosTaskCmd(uint16_t luos_task_id, uint8_t *cmd);
error_return_t MsgAlloc_GetLuosTaskSize(uint16_t luos_task_id, uint16_t *size);
uint16_t MsgAlloc_LuosTasksNbr(void);
void MsgAlloc_ClearMsgFromLuosTasks(msg_t *msg);

// Tx tasks create, get and consume
error_return_t MsgAlloc_SetTxTask(service_t *service_pt, uint8_t *data, uint16_t crc, uint16_t size, luos_localhost_t localhost, uint8_t ack);
void MsgAlloc_PullMsgFromTxTask(void);
void MsgAlloc_PullServiceFromTxTask(uint16_t service_id);
error_return_t MsgAlloc_GetTxTask(service_t **service_pt, uint8_t **data, uint16_t *size, uint8_t *localhost);
error_return_t MsgAlloc_TxAllComplete(void);

#endif /* _MSGALLOC_H_ */
