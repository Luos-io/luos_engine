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

/******************************************************************************
 * @struct msg_allocator_t
 * @brief Message allocator loger structure.
 * 
 * This structure is used to link modules and messages into the allocator.
 * 
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    msg_t *msg_pt; /*!< Start pointer of the msg on msg_buffer. */
    vm_t *vm_pt;   /*!< Pointer to the concerned vm. */
} msg_allocator_t;

/*******************************************************************************
 * Functions
 ******************************************************************************/

void MsgAlloc_Init(void);
void MsgAlloc_ValidHeader(vm_t *concerned_module);
void MsgAlloc_InvalidMsg(void);
void MsgAlloc_EndMsg(void);
void MsgAlloc_SetData(uint8_t data);
msg_t *MsgAlloc_GetCurrentMsg(void);

error_return_t MsgAlloc_PullMsg(vm_t *target_module, msg_t **returned_msg);
error_return_t MsgAlloc_LookAtAllocatorSlot(uint16_t alocator_id, vm_t **allocated_module, msg_t **unconsumed_message);
uint16_t MsgAlloc_AllocNbr(void);

#endif /* _MSGALLOC_H_ */