#include "main.h"
#include "unit_test.h"
#include "msg_alloc.h"

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Static functions

    UNIT_TEST_RUN(unittest_SlotPack);
    UNIT_TEST_RUN(unittest_GetOldestMsg);
    UNIT_TEST_RUN(unittest_BufferAvailableSpaceComputation);
    UNIT_TEST_RUN(unittest_DoWeHaveSpaceUntilBufferEnd);
    UNIT_TEST_RUN(unittest_CheckMsgSpace);

    // Generic functions
    UNIT_TEST_RUN(unittest_MsgAlloc_Loop);
    UNIT_TEST_RUN(unittest_MsgAlloc_Alloc);
    UNIT_TEST_RUN(unittest_MsgAlloc_IsEmpty);
    UNIT_TEST_RUN(unittest_MsgAlloc_Free);

    UNITY_END();
}
