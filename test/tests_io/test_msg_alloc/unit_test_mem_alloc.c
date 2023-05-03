#include "main.h"
#include "unit_test.h"
#include "../src/msg_alloc.c"

/*******************************************************************************
 * Static Function
 ******************************************************************************/

void unittest_SlotPack(void)
{
    uint8_t data = 0;
    // This funciton should reorganize the buffer to pack the messages
    // Create a already organized buffer with a random data

    NEW_TEST_CASE("Check if we assert when allocating more than the buffer size");
    {
        alloc_slot_index = MAX_MSG_NB + 1;
        TRY
        {
            MsgAlloc_SlotPack();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check if an already organized buffer is not modified");
    {
        TRY
        {
            alloc_slots[0].data = &data;
            alloc_slots[1].data = &data;
            alloc_slots[2].data = &data;
            alloc_slots[3].data = &data;
            alloc_slots[4].data = 0;
            alloc_slot_index    = 4;
            MsgAlloc_SlotPack();
            TEST_ASSERT_EQUAL(&data, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[1].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[2].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[3].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[4].data);
            TEST_ASSERT_EQUAL(4, alloc_slot_index);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check if an unorganized buffer is organized");
    {
        TRY
        {
            alloc_slots[0].data = 0;
            alloc_slots[1].data = &data;
            alloc_slots[2].data = &data;
            alloc_slots[3].data = 0;
            alloc_slots[4].data = 0;
            alloc_slots[5].data = 0;
            alloc_slots[6].data = &data;
            alloc_slots[7].data = 0;
            alloc_slot_index    = 8;
            MsgAlloc_SlotPack();
            TEST_ASSERT_EQUAL(&data, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[1].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[2].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[3].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[4].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[5].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[6].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[7].data);
            TEST_ASSERT_EQUAL(3, alloc_slot_index);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check how it's managing the buffer to it's limits");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data              = 0;
            alloc_slots[1].data              = &data;
            alloc_slots[2].data              = &data;
            alloc_slots[3].data              = 0;
            alloc_slots[4].data              = 0;
            alloc_slots[5].data              = 0;
            alloc_slots[6].data              = &data;
            alloc_slots[MAX_MSG_NB - 1].data = &data;
            alloc_slot_index                 = MAX_MSG_NB;
            MsgAlloc_SlotPack();
            TEST_ASSERT_EQUAL(&data, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[1].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[2].data);
            TEST_ASSERT_EQUAL(&data, alloc_slots[3].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[4].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[5].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[6].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[7].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[MAX_MSG_NB - 1].data);
            TEST_ASSERT_EQUAL(4, alloc_slot_index);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_GetOldestMsg(void)
{
    NEW_TEST_CASE("Check if we assert when allocating more than the buffer size");
    {
        alloc_slot_index = MAX_MSG_NB + 1;
        TRY
        {
            MsgAlloc_GetOldestMsg();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check the behavior if we dont have any message");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slot_index = 0;
            TEST_ASSERT_EQUAL(0, MsgAlloc_GetOldestMsg());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the behavior if we have a message in the first slot");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)0x1;
            alloc_slot_index    = 1;
            TEST_ASSERT_EQUAL((uint8_t *)0x1, MsgAlloc_GetOldestMsg());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the behavior if we have a message in another place");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[3].data = (uint8_t *)0x1;
            alloc_slot_index    = 4;
            TEST_ASSERT_EQUAL((uint8_t *)0x1, MsgAlloc_GetOldestMsg());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the behavior if we have a message in the last slot");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[MAX_MSG_NB - 1].data = (uint8_t *)0x1;
            alloc_slot_index                 = MAX_MSG_NB;
            TEST_ASSERT_EQUAL((uint8_t *)0x1, MsgAlloc_GetOldestMsg());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the behavior if we have a message in the last slot and the first one");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data              = (uint8_t *)0x1;
            alloc_slots[MAX_MSG_NB - 1].data = (uint8_t *)0x2;
            alloc_slot_index                 = MAX_MSG_NB;
            TEST_ASSERT_EQUAL((uint8_t *)0x1, MsgAlloc_GetOldestMsg());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the behavior if we have a message in the last slot and the first one");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data              = (uint8_t *)0x1;
            alloc_slots[MAX_MSG_NB - 1].data = (uint8_t *)0x2;
            alloc_slot_index                 = MAX_MSG_NB;
            TEST_ASSERT_EQUAL((uint8_t *)0x1, MsgAlloc_GetOldestMsg());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_BufferAvailableSpaceComputation(void)
{
    // This computation is based on the computed oldest message and the data ptr
    NEW_TEST_CASE("Check if we assert when data_ptr is out of the buffer");
    {
        alloc_slots[0].data = 0;
        TRY
        {
            MsgAlloc_BufferAvailableSpaceComputation();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        alloc_slots[0].data = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE];
        TRY
        {
            MsgAlloc_BufferAvailableSpaceComputation();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check if we assert when olest_msg is out of the buffer");
    {

        TRY
        {
            MsgAlloc_BufferAvailableSpaceComputation();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        data_ptr = &msg_buffer[MSG_BUFFER_SIZE];
        TRY
        {
            MsgAlloc_BufferAvailableSpaceComputation();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }

    NEW_TEST_CASE("Check the result if we don't have any messages");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slot_index = 0;
            data_ptr         = (uint8_t *)&msg_buffer[0];
            TEST_ASSERT_EQUAL(MSG_BUFFER_SIZE, MsgAlloc_BufferAvailableSpaceComputation());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result in a situation we don't have to loop around the buffer end");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            data_ptr            = (uint8_t *)&msg_buffer[0];
            TEST_ASSERT_EQUAL(10, MsgAlloc_BufferAvailableSpaceComputation());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result in a situation we have to loop around the buffer end");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            data_ptr            = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 10];
            TEST_ASSERT_EQUAL(20, MsgAlloc_BufferAvailableSpaceComputation());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result in a situation we have to loop around the buffer end and data_ptr is set to the begin of the buffer");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            data_ptr            = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 10];
            TEST_ASSERT_EQUAL(10, MsgAlloc_BufferAvailableSpaceComputation());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_DoWeHaveSpaceUntilBufferEnd(void)
{
    // In this specific function we may have a pointer after the end of the buffer but never before

    NEW_TEST_CASE("Check if we assert when to pointer is before the buffer");
    {
        TRY
        {
            MsgAlloc_DoWeHaveSpaceUntilBufferEnd(0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check if it return SUCCEED when the pointer is in the buffer");
    {
        TRY
        {
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_DoWeHaveSpaceUntilBufferEnd((uint8_t *)&msg_buffer[0]));
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_DoWeHaveSpaceUntilBufferEnd((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check if it return FAILED when the pointer is after the buffer");
    {
        TRY
        {
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_DoWeHaveSpaceUntilBufferEnd((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE]));
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_DoWeHaveSpaceUntilBufferEnd((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE + 1]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_CheckMsgSpace(void)
{
    NEW_TEST_CASE("Check if we assert when from pointer is after the to pointer or at the same place");
    {
        TRY
        {
            MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[0], (uint8_t *)&msg_buffer[0]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[1], (uint8_t *)&msg_buffer[0]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }

    NEW_TEST_CASE("Check if we assert when one of the pointer is out of the buffer");
    {
        TRY
        {
            MsgAlloc_CheckMsgSpace(0, (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE + 1], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1], 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check the result if we don't have any message yet");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slot_index = 0;
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[0], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result if we have one message and the new one is before the old one");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[0], (uint8_t *)&msg_buffer[9]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result if we have one message and the new one is after the old one");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[11], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result if we have one message and the new one is in the middle of the old one");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[0], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result if we have one message and the new one is at the end of the old one");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[0], (uint8_t *)&msg_buffer[10]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check the result if we have one message and the new one is at the beginning of the old one");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[10], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

/*******************************************************************************
 * Public Function
 ******************************************************************************/

void unittest_MsgAlloc_Loop()
{
    NEW_TEST_CASE("Verify buffer occupation rate stat computing");
    MsgAlloc_Init(NULL);
    {
        // data_ptr is incremented : buffer occupation computing is verified
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------------^---------------------^-------------+
        //        |                         |                     |
        //    data_ptr                   data_ptr                data_ptr
        //      (0% occupation)          (X% occupation)         (etc...)
        //

        // Init variables
        //---------------
        uint32_t expected_buffer_occupation_ratio;
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .engine_msg_stack_ratio  = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);
        data_ptr = &msg_buffer[0];

        // Call function and Verify
        //---------------------------
        // Empty buffer
        TRY
        {
            expected_buffer_occupation_ratio = 0;
            MsgAlloc_Loop();

            NEW_STEP("Check that the buffer is empty");
            TEST_ASSERT_EQUAL(0, memory_stats.buffer_occupation_ratio);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            // Buffer occupation from 0 -> 100%
            NEW_STEP("Check buffer is full in all cases");
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            for (uint16_t i = 1; i < MSG_BUFFER_SIZE; i++)
            {
                data_ptr                         = &msg_buffer[i];
                expected_buffer_occupation_ratio = (i * 100) / (MSG_BUFFER_SIZE);

                MsgAlloc_Loop();

                TEST_ASSERT_EQUAL(expected_buffer_occupation_ratio, memory_stats.buffer_occupation_ratio);
            }
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_MsgAlloc_Alloc()
{

    NEW_TEST_CASE("Check if we assert when we try to allocate a message with no size and/or no phy");
    {
        TRY
        {
            MsgAlloc_Alloc(0, 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_Alloc(1, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_Alloc(0, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check if we assert when we try to allocate a message with a size > MSG_BUFFER_SIZE");
    {
        TRY
        {
            MsgAlloc_Alloc(MSG_BUFFER_SIZE + 1, 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check an allocation with no message already registered");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slot_index = 0;
            data_ptr         = &msg_buffer[0];
            TEST_ASSERT_EQUAL(&msg_buffer[0], MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check an allocation with one message already registered");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[10];
            TEST_ASSERT_EQUAL(&msg_buffer[10], MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check an allocation with two messages already registered");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slots[1].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 2;
            data_ptr            = &msg_buffer[20];
            TEST_ASSERT_EQUAL(&msg_buffer[20], MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check an allocation overflowing the buffer end");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 10];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[MSG_BUFFER_SIZE - 5];
            TEST_ASSERT_EQUAL(&msg_buffer[0], MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check an allocation with a message into the space we need");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[5];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check an allocation with a message into the space we need before the end of the buffer");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 5];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[MSG_BUFFER_SIZE - 7];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check an allocation with a message into the space we need at the begining of the buffer");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[MSG_BUFFER_SIZE - 5];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));

            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[1];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[MSG_BUFFER_SIZE - 5];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check alignement correction");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[1];
            TEST_ASSERT_EQUAL(&msg_buffer[2], MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check an allocation with a message at the limit of the space we have");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[0];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[2];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[2];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 2 - (MSG_BUFFER_SIZE % 2)];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[MSG_BUFFER_SIZE - 2 - (MSG_BUFFER_SIZE % 2)];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[10];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[0];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(11, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            data_ptr            = &msg_buffer[MSG_BUFFER_SIZE - 8];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_MsgAlloc_IsEmpty()
{
    NEW_TEST_CASE("Check if we assert when we have more slot than we can handle");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slot_index = MSG_BUFFER_SIZE + 1;
            MsgAlloc_IsEmpty();
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check if we return succed when we have no message");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slot_index = 0;
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_IsEmpty());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check if we return failed when we have a message");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data = (uint8_t *)&msg_buffer[0];
            alloc_slot_index    = 1;
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_IsEmpty());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_MsgAlloc_Free(void)
{
    NEW_TEST_CASE("Check if we assert when we pass a null pointer");
    {
        TRY
        {
            MsgAlloc_Free(0, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check if we assert when we pass a philter that is not in the list");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data       = (uint8_t *)&msg_buffer[0];
            alloc_slots[0].phy_filter = 0x01;
            alloc_slot_index          = 1;
            MsgAlloc_Free(1, (uint8_t *)&msg_buffer[0]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Check if we don't assert when there is no message in the list . (This can happen in case of reset)");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slot_index = 0;
            MsgAlloc_Free(1, (uint8_t *)&msg_buffer[0]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check a normal free situation with the last concerned phy id");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data       = (uint8_t *)&msg_buffer[0];
            alloc_slots[0].phy_filter = 0x01;
            alloc_slot_index          = 1;
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(0, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[0].phy_filter);
            TEST_ASSERT_EQUAL(1, alloc_slot_index);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Check a normal free situation with the multiple phy id");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data       = (uint8_t *)&msg_buffer[0];
            alloc_slots[0].phy_filter = 0x03;
            alloc_slot_index          = 1;
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[0], alloc_slots[0].data);
            TEST_ASSERT_EQUAL(0x02, alloc_slots[0].phy_filter);
            TEST_ASSERT_EQUAL(1, alloc_slot_index);
            MsgAlloc_Free(1, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(0, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(0x00, alloc_slots[0].phy_filter);
            TEST_ASSERT_EQUAL(1, alloc_slot_index);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}
