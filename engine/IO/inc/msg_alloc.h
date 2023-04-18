/******************************************************************************
 * @file msgAlloc
 * @brief Message reception allocator manager
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _MSGALLOC_H_
#define _MSGALLOC_H_

#include "struct_luos.h"

/*******************************************************************************
 * Functions
 ******************************************************************************/

// Generic functions
void MsgAlloc_Init(memory_stats_t *memory_stats);
void MsgAlloc_Loop(void);

// Msg buffering functions
uint8_t *MsgAlloc_Alloc(uint16_t data_size, uint8_t phy_filter);
void MsgAlloc_Free(uint8_t phy_id, const uint8_t *data);
error_return_t MsgAlloc_IsEmpty(void);

#endif /* _MSGALLOC_H_ */
