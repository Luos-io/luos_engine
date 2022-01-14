#include "main.h"
#include "unit_test.h"
#include "../inc/msg_alloc.h"

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    ASSERT_ACTIVATION(1);

    // Static functions
    UNIT_TEST_RUN(unittest_DoWeHaveSpace);
    UNIT_TEST_RUN(unittest_CheckMsgSpace);
    UNIT_TEST_RUN(unittest_BufferAvailableSpaceComputation);
    UNIT_TEST_RUN(unittest_OldestMsgCandidate);
    UNIT_TEST_RUN(unittest_ClearMsgTask);
    UNIT_TEST_RUN(unittest_ClearLuosTask);
    UNIT_TEST_RUN(unittest_ClearMsgSpace);
    UNIT_TEST_RUN(unittest_ValidDataIntegrity);
    ////MsgAlloc_FindNewOldestMsg => this function doesn't need unit test

    // Generic functions
    UNIT_TEST_RUN(unittest_MsgAlloc_loop);
    UNIT_TEST_RUN(unittest_MsgAlloc_ValidHeader);
    UNIT_TEST_RUN(unittest_MsgAlloc_InvalidMsg);
    UNIT_TEST_RUN(unittest_MsgAlloc_EndMsg);
    UNIT_TEST_RUN(unittest_MsgAlloc_SetData);
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
    // MsgAlloc_Init         => this function doesn't need unit test

    // Tx functions
    UNIT_TEST_RUN(unittest_SetTxTask_buffer_full);
    UNIT_TEST_RUN(unittest_SetTxTask_Tx_too_long_1);
    UNIT_TEST_RUN(unittest_SetTxTask_Tx_too_long_2);
    UNIT_TEST_RUN(unittest_SetTxTask_Tx_too_long_3);
    UNIT_TEST_RUN(unittest_SetTxTask_Rx_too_long_1);
    UNIT_TEST_RUN(unittest_SetTxTask_Rx_too_long_2);
    UNIT_TEST_RUN(unittest_SetTxTask_Rx_too_long_3);
    UNIT_TEST_RUN(unittest_SetTxTask_Rx_too_long_4);
    UNIT_TEST_RUN(unittest_SetTxTask_Task_already_exists);
    UNIT_TEST_RUN(unittest_SetTxTask_copy_OK);
    UNIT_TEST_RUN(unittest_SetTxTask_ACK);
    UNIT_TEST_RUN(unittest_SetTxTask_internal_localhost);
    UNIT_TEST_RUN(unittest_SetTxTask_multihost);
    UNITY_END();
}
