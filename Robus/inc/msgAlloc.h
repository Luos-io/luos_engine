/******************************************************************************
 * @file msgAlloc
 * @brief Message reception allocator manager
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _MSGALLOC_H_
#define _MSGALLOC_H_

#include "robus_struct.h"
#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

// generic functions
void MsgAlloc_Init(memory_stats_t *memory_stats);
void MsgAlloc_loop(void);

// msg buffering functions
void MsgAlloc_ValidHeader(uint8_t status, uint16_t data_size);
void MsgAlloc_InvalidMsg(void);
void MsgAlloc_EndMsg(void);
void MsgAlloc_SetData(uint8_t data);
void MsgAlloc_SetMessage(msg_t *msg);
msg_t *MsgAlloc_GetCurrentMsg(void);
error_return_t MsgAlloc_IsEmpty(void);
void MsgAlloc_UsedMsgEnd(void);

// msg interpretation task stack
error_return_t MsgAlloc_PullMsgToInterpret(msg_t **returned_msg);

// Luos task stack
void MsgAlloc_LuosTaskAlloc(vm_t *container_concerned_by_current_msg, msg_t *concerned_msg);

// Luos task research and pull
error_return_t MsgAlloc_PullMsg(vm_t *target_container, msg_t **returned_msg);
error_return_t MsgAlloc_PullMsgFromLuosTask(uint16_t luos_task_id, msg_t **returned_msg);
error_return_t MsgAlloc_LookAtLuosTask(uint16_t luos_task_id, vm_t **allocated_container);
error_return_t MsgAlloc_GetLuosTaskSourceId(uint16_t luos_task_id, uint16_t *source_id);
error_return_t MsgAlloc_GetLuosTaskCmd(uint16_t luos_task_id, uint8_t *cmd);
error_return_t MsgAlloc_GetLuosTaskSize(uint16_t luos_task_id, uint16_t *size);
uint16_t MsgAlloc_LuosTasksNbr(void);
void MsgAlloc_ClearMsgFromLuosTasks(msg_t *msg);

#endif /* _MSGALLOC_H_ */
