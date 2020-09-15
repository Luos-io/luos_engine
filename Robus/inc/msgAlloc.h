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
void MsgAlloc_InvalidHeader(void);
void MsgAlloc_ValidHeader(void);
void MsgAlloc_EndMsg(void);
uint8_t MsgAlloc_SetData(uint8_t data);
msg_t *MsgAlloc_GetCurrentMsg(void);
void MsgAlloc_GetAllocVars(msg_allocator_t *_alloc_table, uint16_t *_alloc_id);
void MsgAlloc_ClearSlot(uint16_t _alloc_id);

#endif /* _MSGALLOC_H_ */