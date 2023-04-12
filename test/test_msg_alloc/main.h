#ifndef MAIN_H
#define MAIN_H

// Static functions
void unittest_DoWeHaveSpace(void);
void unittest_CheckMsgSpace(void);
void unittest_BufferAvailableSpaceComputation(void);
void unittest_OldestMsgCandidate(void);
void unittest_ClearMsgTask(void);
void unittest_ClearLuosTask(void);
void unittest_ClearMsgSpace(void);

// Generic functions
void unittest_MsgAlloc_loop(void);
void unittest_MsgAlloc_Alloc(void);
void unittest_MsgAlloc_IsEmpty(void);
void unittest_MsgAlloc_UsedMsgEnd(void);
void unittest_MsgAlloc_GetLuosTaskSourceId(void);
void unittest_MsgAlloc_GetLuosTaskCmd(void);
void unittest_MsgAlloc_GetLuosTaskSize(void);
void unittest_MsgAlloc_TxAllComplete(void);
void unittest_MsgAlloc_PullMsgToInterpret(void);
void unittest_MsgAlloc_LuosTaskAlloc(void);
void unittest_MsgAlloc_LuosTasksNbr(void);
void unittest_MsgAlloc_PullMsg(void);
void unittest_MsgAlloc_PullMsgFromLuosTask(void);
void unittest_MsgAlloc_LookAtLuosTask(void);
void unittest_MsgAlloc_ClearMsgFromLuosTasks(void);
void unittest_MsgAlloc_PullMsgFromTxTask(void);
void unittest_MsgAlloc_PullServiceFromTxTask(void);
void unittest_MsgAlloc_GetTxTask(void);

// Tx functions
void unittest_SetTxTask_buffer_full(void);
void unittest_SetTxTask_Tx_too_long(void);
void unittest_SetTxTask_Tx_overflow(void);
void unittest_SetTxTask_Tx_fit(void);
void unittest_SetTxTask_Task_already_exists(void);
void unittest_SetTxTask_ACK(void);
void unittest_SetTxTask_internal_localhost(void);
void unittest_SetTxTask_multihost(void);

#endif // MAIN_H