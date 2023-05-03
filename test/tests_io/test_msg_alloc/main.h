#ifndef MAIN_H
#define MAIN_H

// Static functions
void unittest_SlotPack(void);
void unittest_GetOldestMsg(void);
void unittest_BufferAvailableSpaceComputation(void);
void unittest_DoWeHaveSpaceUntilBufferEnd(void);
void unittest_CheckMsgSpace(void);

// public functions
void unittest_MsgAlloc_Loop(void);
void unittest_MsgAlloc_Alloc(void);
void unittest_MsgAlloc_IsEmpty(void);
void unittest_MsgAlloc_Free(void);

#endif // MAIN_H
