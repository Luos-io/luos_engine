#include "unit_test.h"
#include "../src/msg_alloc.c"

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
            alloc_nb             = 0;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 0;
            data_ptr             = (uint8_t *)&msg_buffer[0];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = (uint8_t *)&msg_buffer[0];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 10];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 10];
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
            alloc_nb             = 0;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 0;
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 0;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 0;
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_CheckMsgSpace((uint8_t *)&msg_buffer[10], (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 1]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
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
            alloc_nb             = 0;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 0;
            data_ptr             = &msg_buffer[0];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[10];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_slots[1].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 2;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 2;
            data_ptr             = &msg_buffer[20];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[MSG_BUFFER_SIZE - 5];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[5];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 5];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[MSG_BUFFER_SIZE - 7];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[MSG_BUFFER_SIZE - 5];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));

            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[1];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[MSG_BUFFER_SIZE - 5];
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[1];
            TEST_ASSERT_EQUAL(&msg_buffer[2], MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Check alignement correction limits by looping on it");
    {
        volatile int i;
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data  = 0;
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[0];
            for (i = 0; i < 600; i++)
            {
                volatile uint8_t *data;
                data = MsgAlloc_Alloc(7, 0x01);
                // Check if the data pointer moved accordingly => 7 + 1 for alignement
                if (((i + 1) * 8) >= MSG_BUFFER_SIZE)
                {
                    TEST_ASSERT_EQUAL((i * 8) - MSG_BUFFER_SIZE + (MSG_BUFFER_SIZE - (8 * (int)(MSG_BUFFER_SIZE / 8))), ((uintptr_t)data) - (uintptr_t)&msg_buffer[0]);
                }
                else
                {
                    TEST_ASSERT_EQUAL((i * 8), ((uintptr_t)data) - (uintptr_t)&msg_buffer[0]);
                }
                // MsgAlloc_Free(0, (const uint8_t *)data);
                // // Sort the allocator
                // MsgAlloc_Loop();
            }
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[0];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[2];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[2];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - 2 - (MSG_BUFFER_SIZE % 2)];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[MSG_BUFFER_SIZE - 2 - (MSG_BUFFER_SIZE % 2)];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[10];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[0];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(11, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
            data_ptr             = &msg_buffer[MSG_BUFFER_SIZE - 8];
            TEST_ASSERT_EQUAL(0, MsgAlloc_Alloc(10, 0x01));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_MsgAlloc_Reference(void)
{

    NEW_TEST_CASE("Check if we assert when we try to reference a message with a bad data and/or no phy");
    {

        memset(alloc_slots, 0, sizeof(alloc_slots));
        TRY
        {
            MsgAlloc_Reference((uint8_t *)(msg_buffer - 1), 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_Reference((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE], 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            MsgAlloc_Reference((uint8_t *)&msg_buffer[0], 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            alloc_nb             = MAX_MSG_NB;
            oldest_alloc_slot    = 0;
            available_alloc_slot = MAX_MSG_NB;
            MsgAlloc_Reference((uint8_t *)&msg_buffer[0], 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Check normal referencing condition");
    {
        TRY
        {
            alloc_nb             = 0;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 0;
            MsgAlloc_Reference((uint8_t *)&msg_buffer[0], 1);
            TEST_ASSERT_EQUAL(&msg_buffer[0], alloc_slots[0].data);
            TEST_ASSERT_EQUAL(1, alloc_nb);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_MsgAlloc_IsEmpty(void)
{
    NEW_TEST_CASE("Check if we return succed when we have no message");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_nb             = 0;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 0;
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
            alloc_slots[0].data  = (uint8_t *)&msg_buffer[0];
            alloc_nb             = 1;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 1;
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
            alloc_nb                  = 1;
            oldest_alloc_slot         = 0;
            available_alloc_slot      = 1;
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
            alloc_nb             = 0;
            oldest_alloc_slot    = 0;
            available_alloc_slot = 0;
            MsgAlloc_Free(1, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(0, alloc_nb);
            TEST_ASSERT_EQUAL(0, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(0, available_alloc_slot);
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
            alloc_nb                  = 1;
            oldest_alloc_slot         = 0;
            available_alloc_slot      = 1;
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(0, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(0, alloc_slots[0].phy_filter);
            TEST_ASSERT_EQUAL(1, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(1, available_alloc_slot);
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
            alloc_nb                  = 1;
            oldest_alloc_slot         = 0;
            available_alloc_slot      = 1;
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[0], alloc_slots[0].data);
            TEST_ASSERT_EQUAL(0x02, alloc_slots[0].phy_filter);
            TEST_ASSERT_EQUAL(1, alloc_nb);
            TEST_ASSERT_EQUAL(0, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(1, available_alloc_slot);
            MsgAlloc_Free(1, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(0, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(0x00, alloc_slots[0].phy_filter);
            TEST_ASSERT_EQUAL(0, alloc_nb);
            TEST_ASSERT_EQUAL(1, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(1, available_alloc_slot);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
    NEW_TEST_CASE("Try to free a message that is not the oldest one.");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[0].data       = (uint8_t *)&msg_buffer[0];
            alloc_slots[0].phy_filter = 0x01;
            alloc_slots[1].data       = (uint8_t *)&msg_buffer[1];
            alloc_slots[1].phy_filter = 0x01;
            alloc_nb                  = 2;
            oldest_alloc_slot         = 0;
            available_alloc_slot      = 2;
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[1]);
            TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[0], alloc_slots[0].data);
            TEST_ASSERT_EQUAL(1, alloc_nb);
            TEST_ASSERT_EQUAL(0, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(2, available_alloc_slot);
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(0, alloc_slots[0].data);
            TEST_ASSERT_EQUAL(0, alloc_nb);
            TEST_ASSERT_EQUAL(2, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(2, available_alloc_slot);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Try to free a message that is not the oldest one with a buffer loop.");
    {
        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[MAX_MSG_NB - 1].data       = (uint8_t *)&msg_buffer[0];
            alloc_slots[MAX_MSG_NB - 1].phy_filter = 0x01;
            alloc_slots[0].data                    = (uint8_t *)&msg_buffer[1];
            alloc_slots[0].phy_filter              = 0x01;
            alloc_nb                               = 2;
            oldest_alloc_slot                      = MAX_MSG_NB - 1;
            available_alloc_slot                   = 1;
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[1]);
            TEST_ASSERT_EQUAL(1, alloc_nb);
            TEST_ASSERT_EQUAL(MAX_MSG_NB - 1, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(1, available_alloc_slot);
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(0, alloc_nb);
            TEST_ASSERT_EQUAL(1, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(1, available_alloc_slot);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;

        TRY
        {
            memset(alloc_slots, 0, sizeof(alloc_slots));
            alloc_slots[MAX_MSG_NB - 1].data       = (uint8_t *)&msg_buffer[0];
            alloc_slots[MAX_MSG_NB - 1].phy_filter = 0x01;
            alloc_slots[0].data                    = (uint8_t *)&msg_buffer[1];
            alloc_slots[0].phy_filter              = 0x01;
            alloc_nb                               = 2;
            oldest_alloc_slot                      = MAX_MSG_NB - 1;
            available_alloc_slot                   = 1;
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[0]);
            TEST_ASSERT_EQUAL(1, alloc_nb);
            TEST_ASSERT_EQUAL(0, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(1, available_alloc_slot);
            MsgAlloc_Free(0, (uint8_t *)&msg_buffer[1]);
            TEST_ASSERT_EQUAL(0, alloc_nb);
            TEST_ASSERT_EQUAL(1, oldest_alloc_slot);
            TEST_ASSERT_EQUAL(1, available_alloc_slot);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Static functions

    UNIT_TEST_RUN(unittest_BufferAvailableSpaceComputation);
    UNIT_TEST_RUN(unittest_DoWeHaveSpaceUntilBufferEnd);
    UNIT_TEST_RUN(unittest_CheckMsgSpace);

    // Generic functions
    UNIT_TEST_RUN(unittest_MsgAlloc_Loop);
    UNIT_TEST_RUN(unittest_MsgAlloc_Alloc);
    UNIT_TEST_RUN(unittest_MsgAlloc_Reference);
    UNIT_TEST_RUN(unittest_MsgAlloc_IsEmpty);
    UNIT_TEST_RUN(unittest_MsgAlloc_Free);

    UNITY_END();
}
