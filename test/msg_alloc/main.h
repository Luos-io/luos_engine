#ifndef MAIN_H
#define MAIN_H

// Node config to default values
#define MAX_SERVICE_NUMBER 5
#define MSG_BUFFER_SIZE    3 * sizeof(msg_t)
#define MAX_MSG_NB         10

// Static functions
void unittest_DoWeHaveSpace(void);
void unittest_CheckMsgSpace(void);
void unittest_BufferAvailableSpaceComputation(void);
void unittest_OldestMsgCandidate(void);
void unittest_ClearMsgTask(void);
void unittest_ClearLuosTask(void);
void unittest_ClearMsgSpace(void);
void unittest_ValidDataIntegrity(void);

// Generic functions
void unittest_MsgAlloc_loop(void);
void unittest_MsgAlloc_ValidHeader(void);
void unittest_MsgAlloc_InvalidMsg(void);
void unittest_MsgAlloc_EndMsg(void);
void unittest_MsgAlloc_SetData(void);
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
void unittest_SetTxTask_Tx_too_long_1(void);
void unittest_SetTxTask_Tx_too_long_2(void);
void unittest_SetTxTask_Tx_too_long_3(void);
void unittest_SetTxTask_Rx_too_long_1(void);
void unittest_SetTxTask_Rx_too_long_2(void);
void unittest_SetTxTask_Rx_too_long_3(void);
void unittest_SetTxTask_Rx_too_long_4(void);
void unittest_SetTxTask_Task_already_exists(void);
void unittest_SetTxTask_copy_OK(void);
void unittest_SetTxTask_ACK(void);
void unittest_SetTxTask_internal_localhost(void);
void unittest_SetTxTask_multihost(void);

#endif //MAIN_H