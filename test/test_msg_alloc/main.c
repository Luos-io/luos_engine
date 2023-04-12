#include "main.h"
#include "unit_test.h"
#include "msg_alloc.h"

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Static functions
    UNIT_TEST_RUN(unittest_DoWeHaveSpace);
    UNIT_TEST_RUN(unittest_CheckMsgSpace);
    UNIT_TEST_RUN(unittest_BufferAvailableSpaceComputation);
    UNIT_TEST_RUN(unittest_OldestMsgCandidate);
    UNIT_TEST_RUN(unittest_ClearMsgTask);
    UNIT_TEST_RUN(unittest_ClearLuosTask);
    UNIT_TEST_RUN(unittest_ClearMsgSpace);

    // Generic functions
    UNIT_TEST_RUN(unittest_MsgAlloc_loop);
    UNIT_TEST_RUN(unittest_MsgAlloc_Alloc);
    UNIT_TEST_RUN(unittest_MsgAlloc_IsEmpty);
    UNIT_TEST_RUN(unittest_MsgAlloc_UsedMsgEnd);
    UNIT_TEST_RUN(unittest_MsgAlloc_GetLuosTaskSourceId);
    UNIT_TEST_RUN(unittest_MsgAlloc_GetLuosTaskCmd);
    UNIT_TEST_RUN(unittest_MsgAlloc_GetLuosTaskSize);
    UNIT_TEST_RUN(unittest_MsgAlloc_TxAllComplete);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullMsgToInterpret);
    UNIT_TEST_RUN(unittest_MsgAlloc_LuosTaskAlloc);
    UNIT_TEST_RUN(unittest_MsgAlloc_LuosTasksNbr);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullMsg);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullMsgFromLuosTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_LookAtLuosTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_ClearMsgFromLuosTasks);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullMsgFromTxTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullServiceFromTxTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_GetTxTask);

    // Tx functions
    UNIT_TEST_RUN(unittest_SetTxTask_buffer_full);
    UNIT_TEST_RUN(unittest_SetTxTask_Tx_too_long);
    UNIT_TEST_RUN(unittest_SetTxTask_Tx_overflow);
    UNIT_TEST_RUN(unittest_SetTxTask_Tx_fit);
    UNIT_TEST_RUN(unittest_SetTxTask_Task_already_exists);
    UNIT_TEST_RUN(unittest_SetTxTask_ACK);
    UNIT_TEST_RUN(unittest_SetTxTask_internal_localhost);
    UNIT_TEST_RUN(unittest_SetTxTask_multihost);
    UNITY_END();
}
