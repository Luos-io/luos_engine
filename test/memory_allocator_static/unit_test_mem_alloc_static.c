#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../test/unit_test.h"
#include "../Robus/inc/robus_struct.h"
#include "../src/msg_alloc.c"

void unittest_DoWeHaveSpace(void)
{
    NEW_TEST_CASE("\t* There is enough space");
    MsgAlloc_Init(NULL);
    {
        NEW_STEP();
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_DoWeHaveSpace((void *)&msg_buffer[i]));
        }
    }

    NEW_TEST_CASE("\t* there is not enough space");
    MsgAlloc_Init(NULL);
    {
        NEW_STEP();
        for (uint16_t i = MSG_BUFFER_SIZE; i < MSG_BUFFER_SIZE + 10; i++)
        {
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_DoWeHaveSpace((void *)&msg_buffer[i]));
        }
    }
}

void unittest_CheckMsgSpace(void)
{
    NEW_TEST_CASE("\t* Check Message Space");
    MsgAlloc_Init(NULL);
    {
        // Declaration of dummy message start and end pointer
        uint32_t *mem_start;
        uint32_t *mem_end;

        // Initialize pointer to buffer beginning
        used_msg = (msg_t *)&msg_buffer[sizeof(msg_t)];

        // Test function considering "used_msg"
        // ("oldest_message" unit testing is useless as it's the same behaviour)

        // Init variables
        //---------------
        mem_start = (uint32_t *)used_msg - 2;
        mem_end   = mem_start + 1;
        // Call function and Verify
        //---------------------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end));

        // Init variables
        //---------------
        mem_start = (uint32_t *)used_msg - 1;
        mem_end   = mem_start + 1;
        // Call function and Verify
        //---------------------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end));

        // Init variables
        //---------------
        mem_start = (uint32_t *)used_msg;
        mem_end   = mem_start + 1;
        // Call function and Verify
        //---------------------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end));

        // Init variables
        //---------------
        mem_start = (uint32_t *)used_msg + 1;
        mem_end   = mem_start + 1;
        // Call function and Verify
        //---------------------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end));
    }
}

void unittest_BufferAvailableSpaceComputation(void)
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
#define assert_nb 6
        typedef struct
        {
            bool expected_asserts;      // if "true" : expect a Luos assert
            msg_t *oldest_msg_position; // oldest_msg position in msg_buffer
        } assert_scenario;

        assert_scenario assert_sc[assert_nb];
        uint32_t free_space = MSG_BUFFER_SIZE;

        // Expected Values
        assert_sc[0].expected_asserts = true;
        assert_sc[1].expected_asserts = false;
        assert_sc[2].expected_asserts = false;
        assert_sc[3].expected_asserts = false;
        assert_sc[4].expected_asserts = true;
        assert_sc[5].expected_asserts = true;

        assert_sc[0].oldest_msg_position = (msg_t *)&msg_buffer[0] - 1;
        assert_sc[1].oldest_msg_position = (msg_t *)&msg_buffer[0];
        assert_sc[2].oldest_msg_position = (msg_t *)&msg_buffer[1];
        assert_sc[3].oldest_msg_position = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE] - 1;
        assert_sc[4].oldest_msg_position = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE];
        assert_sc[5].oldest_msg_position = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE] + 1;

        // Launch test
        for (uint8_t i = 0; i < assert_nb; i++)
        {
            reset_assert();
            oldest_msg = assert_sc[i].oldest_msg_position;
            MsgAlloc_BufferAvailableSpaceComputation();
            NEW_STEP();
            TEST_ASSERT_EQUAL(assert_sc[i].expected_asserts, is_assert());
        }
    }

    NEW_TEST_CASE("\t* No task is availabled");
    MsgAlloc_Init(NULL);
    {
        uint32_t remaining_datas;
        uint32_t expected_size = 0;
        uint32_t free_space    = 0;
        oldest_msg             = (msg_t *)0xFFFFFFFF; //No oldest message

        //Test remaining space computing for all message size possibilities
        NEW_STEP();
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE - 2; i++)
        {
            // Init variables
            //---------------
            current_msg         = (msg_t *)&msg_buffer[i];
            data_end_estimation = (uint8_t *)&msg_buffer[i + 1];
            for (uint8_t j = 0; j < MSG_BUFFER_SIZE - 1; j++)
            {
                // Test is launched only if "data_end_estimation" doesn't overflows "msg_buffer" size
                if (data_end_estimation < (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE])
                {
                    reset_assert();
                    remaining_datas = (uint32_t)data_end_estimation - (uint32_t)current_msg;
                    expected_size   = MSG_BUFFER_SIZE - remaining_datas;
                    // Call function
                    //---------------
                    free_space = MsgAlloc_BufferAvailableSpaceComputation();

                    // Verify
                    //---------------
                    TEST_ASSERT_FALSE(is_assert());
                    TEST_ASSERT_EQUAL(expected_size, free_space);

                    data_end_estimation++;
                }
                else
                {
                    break;
                }
            }
        }
    }

    NEW_TEST_CASE("\t* Oldest task is between `data_end_estimation` and the end of message buffer");
    MsgAlloc_Init(NULL);
    {
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        +------^---------------------^--------------------------------+
        //               |                     |
        //               |<-----Free space---->|
        //               |                     |
        //               data_end_estimation    oldest_task

        uint32_t free_space    = 0;
        uint32_t expected_size = 0;
        oldest_msg             = (msg_t *)&msg_buffer[1];
        data_end_estimation    = (uint8_t *)oldest_msg - 1;

        NEW_STEP();
        //Test remaining space computing for all possibilities
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE - 1; i++)
        {
            oldest_msg = (msg_t *)&msg_buffer[1];
            //for (uint8_t j = 0; j < 2; j++)
            for (uint16_t j = i; j < MSG_BUFFER_SIZE - 1; j++)
            {
                if ((uint32_t)oldest_msg > (uint32_t)data_end_estimation)
                {
                    reset_assert();
                    expected_size = (uint32_t)oldest_msg - (uint32_t)data_end_estimation;
                    free_space    = MsgAlloc_BufferAvailableSpaceComputation();
                    TEST_ASSERT_FALSE(is_assert());
                    TEST_ASSERT_EQUAL(expected_size, free_space);
                }
                //oldest_msg++;
                oldest_msg = (msg_t *)&msg_buffer[j + 1];
            }
            data_end_estimation++;
        }
    }

    NEW_TEST_CASE("\t* Oldest task is between the begin of the buffer and current_msg`");
    MsgAlloc_Init(NULL);
    {
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        +-------------^--------------^------------------^-------------+
        //                      |              |                  |
        //        <-Free space->|              |                  |<-Free space->
        //                      |              |                  |
        //                      |              |                  |
        //                      oldest_task     current_message   data_end_estimation

        uint32_t free_space    = 0;
        uint32_t expected_size = 0;
        data_end_estimation    = (uint8_t *)&msg_buffer[1];
        oldest_msg             = (msg_t *)data_end_estimation - 1;

        NEW_STEP();
        //Test remaining space computing for all possibilities
        //while (data_end_estimation < &msg_buffer[MSG_BUFFER_SIZE])
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE - 1; i++)
        {
            oldest_msg = (msg_t *)&msg_buffer[1];
            for (uint16_t j = 0; j < MSG_BUFFER_SIZE - 1; j++)
            {
                if ((uint32_t)oldest_msg < (uint32_t)data_end_estimation)
                {
                    reset_assert();
                    expected_size = (uint32_t)&msg_buffer[MSG_BUFFER_SIZE] - (uint32_t)data_end_estimation;
                    expected_size += (uint32_t)oldest_msg - (uint32_t)&msg_buffer[0];
                    free_space = MsgAlloc_BufferAvailableSpaceComputation();

                    TEST_ASSERT_FALSE(is_assert());
                    TEST_ASSERT_EQUAL(expected_size, free_space);
                }
                oldest_msg = (msg_t *)&msg_buffer[j + 1];
            }
            data_end_estimation++;
        }
    }
}

void unittest_OldestMsgCandidate(void)
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
#define assert_nb 6
        typedef struct
        {
            bool expected_asserts;
            msg_t *oldest_stack_msg_pt;
        } assert_scenario;

        assert_scenario assert_sc[assert_nb];
        uint32_t free_space = MSG_BUFFER_SIZE;

        // Expected Values
        assert_sc[0].expected_asserts    = true;
        assert_sc[0].oldest_stack_msg_pt = (msg_t *)&msg_buffer[0] - 1;
        assert_sc[1].expected_asserts    = false;
        assert_sc[1].oldest_stack_msg_pt = (msg_t *)&msg_buffer[0];
        assert_sc[2].expected_asserts    = false;
        assert_sc[2].oldest_stack_msg_pt = (msg_t *)&msg_buffer[1];
        assert_sc[3].expected_asserts    = false;
        assert_sc[3].oldest_stack_msg_pt = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - 1];
        assert_sc[4].expected_asserts    = true;
        assert_sc[4].oldest_stack_msg_pt = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE];
        assert_sc[5].expected_asserts    = true;
        assert_sc[5].oldest_stack_msg_pt = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE] + 1;

        // Launch test
        for (uint8_t i = 0; i < assert_nb; i++)
        {
            reset_assert();
            MsgAlloc_OldestMsgCandidate(assert_sc[i].oldest_stack_msg_pt);

            NEW_STEP();
            TEST_ASSERT_EQUAL(assert_sc[i].expected_asserts, is_assert());
        }
    }

    NEW_TEST_CASE("\t* Verify case \"oldest_stack_msg_pt\" is NULL");
    MsgAlloc_Init(NULL);
    {
        // Pass NULL pointer to MsgAlloc_OldestMsgCandidate => oldest_msg doesn't change
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------------------------------------------------+
        //        |
        //        oldest_msg

        oldest_msg                          = (msg_t *)&msg_buffer[0];
        volatile msg_t *expected_oldest_msg = oldest_msg;
        msg_t *oldest_stack_msg_pt          = NULL;

        reset_assert();
        MsgAlloc_OldestMsgCandidate(oldest_stack_msg_pt);

        NEW_STEP();
        TEST_ASSERT_FALSE(is_assert());
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_oldest_msg, oldest_msg);
    }

    NEW_TEST_CASE("\t* Verify other cases");
    MsgAlloc_Init(NULL);
    {
#define CASE 18
        msg_t *oldest_stack_msg_pt;
        volatile msg_t *expected_oldest_msg;

        // Array with all possible cases
        int cases[CASE][4] = {//current_msg  oldest_msg  oldest_stack_msg_pt  expected_oldest_msg
                              {/* */ 0, /*     */ 1, /*        */ 2, /*            */ 1},
                              {/* */ 0, /*     */ 2, /*        */ 1, /*            */ 1},
                              {/* */ 1, /*     */ 0, /*        */ 2, /*            */ 2},
                              {/* */ 1, /*     */ 2, /*        */ 0, /*            */ 2},
                              {/* */ 2, /*     */ 0, /*        */ 1, /*            */ 0},
                              {/* */ 2, /*     */ 1, /*        */ 0, /*            */ 0},
                              {/* */ 0, /*     */ 1, /*        */ 0, /*            */ 1},
                              {/* */ 0, /*     */ 2, /*        */ 0, /*            */ 2},
                              {/* */ 1, /*     */ 0, /*        */ 1, /*            */ 0},
                              {/* */ 1, /*     */ 2, /*        */ 1, /*            */ 2},
                              {/* */ 2, /*     */ 0, /*        */ 2, /*            */ 0},
                              {/* */ 2, /*     */ 1, /*        */ 2, /*            */ 1},
                              {/* */ 0, /*     */ 1, /*        */ 1, /*            */ 1},
                              {/* */ 0, /*     */ 2, /*        */ 2, /*            */ 2},
                              {/* */ 1, /*     */ 0, /*        */ 0, /*            */ 0},
                              {/* */ 1, /*     */ 2, /*        */ 2, /*            */ 2},
                              {/* */ 2, /*     */ 0, /*        */ 0, /*            */ 0},
                              {/* */ 2, /*     */ 1, /*        */ 1, /*            */ 1}};

        for (uint8_t i = 0; i < CASE; i++)
        {
            current_msg         = (msg_t *)&msg_buffer[cases[i][0]];
            oldest_msg          = (msg_t *)&msg_buffer[cases[i][1]];
            oldest_stack_msg_pt = (msg_t *)&msg_buffer[cases[i][2]];
            expected_oldest_msg = (msg_t *)&msg_buffer[cases[i][3]];
            MsgAlloc_OldestMsgCandidate(oldest_stack_msg_pt);
            NEW_STEP();
            TEST_ASSERT_EQUAL(expected_oldest_msg, oldest_msg);
        }
    }
}

void unittest_ValidDataIntegrity(void)
{
    NEW_TEST_CASE("\t* No copy needed");
    MsgAlloc_Init(NULL);
    {
        uint8_t expected_msg_buffer[MSG_BUFFER_SIZE];
        mem_clear_needed = 0;
        copy_task_pointer == NULL;

        memset((void *)&msg_buffer[0], 0xAA, MSG_BUFFER_SIZE);
        memset((void *)&expected_msg_buffer[0], 0xAA, MSG_BUFFER_SIZE);
        reset_assert();
        MsgAlloc_ValidDataIntegrity();

        NEW_STEP();
        TEST_ASSERT_FALSE(is_assert());
        NEW_STEP();
        TEST_ASSERT_EQUAL_MEMORY(expected_msg_buffer, msg_buffer, MSG_BUFFER_SIZE);
    }

    NEW_TEST_CASE("\t* Copy header to begin of message buffer");
    MsgAlloc_Init(NULL);
    {
        // Tx message size is greater than Rx size currently received.
        // Tx message doesn't fit in msg buffer.
        // There is already a Task , function should return FAILED
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-------|Header|----------------------------------------------|
        //        +-------------------------------------------------------------+
        //
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |Header|------------------------------------------------------|
        //        +-------------------------------------------------------------+

        uint8_t expected_msg_buffer[MSG_BUFFER_SIZE];
        mem_clear_needed  = 0;
        copy_task_pointer = (header_t *)&msg_buffer[sizeof(msg_t)];

        memset((void *)&msg_buffer[0], 0, MSG_BUFFER_SIZE);
        memset((void *)&expected_msg_buffer[0], 0, MSG_BUFFER_SIZE);
        memset((void *)&msg_buffer[sizeof(msg_t)], 1, sizeof(header_t));
        memset((void *)&expected_msg_buffer[0], 1, sizeof(header_t));

        reset_assert();
        MsgAlloc_ValidDataIntegrity();

        NEW_STEP();
        TEST_ASSERT_FALSE(is_assert());
        NEW_STEP();
        TEST_ASSERT_EQUAL(copy_task_pointer, NULL);
        NEW_STEP();
        TEST_ASSERT_EQUAL_MEMORY(expected_msg_buffer, msg_buffer, sizeof(header_t));
    }

    NEW_TEST_CASE("\t* Verify memory cleaning");
    MsgAlloc_Init(NULL);
    {
        mem_clear_needed    = 1;
        current_msg         = (msg_t *)&msg_buffer[0];
        data_end_estimation = (uint8_t *)(current_msg + 1);
        reset_assert();
        MsgAlloc_ValidDataIntegrity();

        NEW_STEP();
        TEST_ASSERT_FALSE(is_assert());
        NEW_STEP();
        TEST_ASSERT_EQUAL(0, mem_clear_needed);
        // No more TEST_ASSERT needed as MsgAlloc_ClearMsgSpace has already been tested
    }
}

void unittest_ClearMsgSpace(void)
{
    NEW_TEST_CASE("\t* There is not enough space in memory");
    MsgAlloc_Init(NULL);
    {
        void *memory_start = (void *)&msg_buffer[0];
        void *memory_end   = (void *)&msg_buffer[MSG_BUFFER_SIZE];

        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_ClearMsgSpace(memory_start, memory_end));
    }

    NEW_TEST_CASE("\t* Drop used messages");
    MsgAlloc_Init(NULL);
    {
        //        used_msg will be dropped
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------^----------^------------------------------------------+
        //        |       |          |
        //        start   used_msg   end

        void *memory_start;
        void *memory_end;
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        used_msg     = (msg_t *)&msg_buffer[1];
        memory_start = (void *)&msg_buffer[0];
        memory_end   = (void *)&msg_buffer[2];

        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_ClearMsgSpace(memory_start, memory_end));
        NEW_STEP();
        TEST_ASSERT_EQUAL(100, memory_stats.buffer_occupation_ratio);
        NEW_STEP();
        TEST_ASSERT_EQUAL(1, memory_stats.msg_drop_number);
        NEW_STEP();
        TEST_ASSERT_NULL(used_msg);

        NEW_STEP();
        for (uint8_t i = 1; i < 0xFF; i++)
        {
            used_msg = (msg_t *)&msg_buffer[1];
            MsgAlloc_ClearMsgSpace(memory_start, memory_end);
            TEST_ASSERT_EQUAL(i + 1, memory_stats.msg_drop_number);
        }

        used_msg                     = (msg_t *)&msg_buffer[1];
        memory_stats.msg_drop_number = 255;
        MsgAlloc_ClearMsgSpace(memory_start, memory_end);

        NEW_STEP();
        TEST_ASSERT_EQUAL(255, memory_stats.msg_drop_number);
    }

    NEW_TEST_CASE("\t* Drop all messages from luos_tasks");
    MsgAlloc_Init(NULL);
    {
        //        All messages of luos_tasks will be dropped
        //
        //        Init state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------^----------^-----------------------------^+
        //        |                   |          |                             |
        //        used_msg            start      oldest_msg                    end
        //
        //               Luos_tasks
        //               +---------+
        //               | Task D1 |
        //               |---------|
        //               +---------+
        //               | Task D2 |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               | LastTask|
        //               |---------|

        //        Ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^------------------------------^------------------------------+
        //        |                              |
        //        used_msg                       oldest_msg
        //
        //               Luos_tasks
        //               +---------+
        //               |    0    |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|

        void *memory_start;
        void *memory_end;
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        for (uint16_t i = 0; i < MAX_MSG_NB - 2; i++)
        {
            luos_tasks[i].msg_pt          = (msg_t *)&msg_buffer[i + 2];
            luos_tasks[i].ll_container_pt = (ll_container_t *)&msg_buffer[i + 2];
        }
        used_msg            = (msg_t *)&msg_buffer[0];
        oldest_msg          = (msg_t *)&msg_buffer[2];
        memory_start        = (void *)&msg_buffer[1];
        memory_end          = (void *)&msg_buffer[MAX_MSG_NB - 1];
        luos_tasks_stack_id = MAX_MSG_NB;

        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_ClearMsgSpace(memory_start, memory_end));
        NEW_STEP();
        TEST_ASSERT_EQUAL(100, memory_stats.buffer_occupation_ratio);
        NEW_STEP();
        TEST_ASSERT_EQUAL(2, luos_tasks_stack_id);
        NEW_STEP();
        TEST_ASSERT_EQUAL(8, memory_stats.msg_drop_number);
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB - 2; i++)
        {
            TEST_ASSERT_EQUAL(0, luos_tasks[i].msg_pt);
            TEST_ASSERT_EQUAL(0, luos_tasks[i].ll_container_pt);
        }
    }

    NEW_TEST_CASE("\t* Drop all messages from msg_tasks");
    MsgAlloc_Init(NULL);
    {
        //        All messages of msg_tasks will be dropped
        //
        //        Init state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------^----------^-----------------------------^+
        //        |                   |          |                             |
        //        used_msg            start      oldest_msg                    end
        //
        //               msg_tasks
        //               +---------+
        //               | Task A1 |
        //               |---------|
        //               +---------+
        //               | Task A2 |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               | LastTask|
        //               |---------|

        //        Ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^------------------------------^------------------------------+
        //        |                              |
        //        used_msg                       oldest_msg
        //
        //               msg_tasks
        //               +---------+
        //               |    0    |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|

        void *memory_start;
        void *memory_end;
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        for (uint16_t i = 0; i < MAX_MSG_NB - 2; i++)
        {
            msg_tasks[i] = (msg_t *)&msg_buffer[i + 2];
        }
        used_msg           = (msg_t *)&msg_buffer[0];
        oldest_msg         = (msg_t *)&msg_buffer[2];
        memory_start       = (void *)&msg_buffer[1];
        memory_end         = (void *)&msg_buffer[MAX_MSG_NB - 1];
        msg_tasks_stack_id = MAX_MSG_NB;

        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_ClearMsgSpace(memory_start, memory_end));
        NEW_STEP();
        TEST_ASSERT_EQUAL(100, memory_stats.buffer_occupation_ratio);
        NEW_STEP();
        TEST_ASSERT_EQUAL(2, msg_tasks_stack_id);
        NEW_STEP();
        TEST_ASSERT_EQUAL(8, memory_stats.msg_drop_number);
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB - 2; i++)
        {
            TEST_ASSERT_EQUAL(0, msg_tasks[i]);
        }
    }

    NEW_TEST_CASE("\t* Drop all messages from tx_tasks");
    MsgAlloc_Init(NULL);
    {
        //        All messages of tx_tasks will be dropped
        //
        //        Init state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------^----------^-----------------------------^+
        //        |                   |          |                             |
        //        used_msg            start      oldest_msg                    end
        //
        //               tx_tasks
        //               +---------+
        //               | Task E1 |
        //               |---------|
        //               +---------+
        //               | Task E2 |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               | LastTask|
        //               |---------|

        //        Ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^------------------------------^------------------------------+
        //        |                              |
        //        used_msg                       oldest_msg
        //
        //               tx_tasks
        //               +---------+
        //               |    0    |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|

        void *memory_start;
        void *memory_end;
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        for (uint16_t i = 0; i < MAX_MSG_NB - 2; i++)
        {
            tx_tasks[i].data_pt = (uint8_t *)&msg_buffer[i + 2];
            tx_tasks[i].size    = (uint16_t)i;
        }
        used_msg          = (msg_t *)&msg_buffer[0];
        oldest_msg        = (msg_t *)&msg_buffer[2];
        memory_start      = (void *)&msg_buffer[1];
        memory_end        = (void *)&msg_buffer[MAX_MSG_NB - 1];
        tx_tasks_stack_id = MAX_MSG_NB;

        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_ClearMsgSpace(memory_start, memory_end));
        NEW_STEP();
        TEST_ASSERT_EQUAL(100, memory_stats.buffer_occupation_ratio);
        NEW_STEP();
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP();
        TEST_ASSERT_EQUAL(8, memory_stats.msg_drop_number);
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB - 2; i++)
        {
            TEST_ASSERT_EQUAL(0, tx_tasks[i].data_pt);
            TEST_ASSERT_EQUAL(0, tx_tasks[i].size);
        }
    }
}

void unittest_ClearMsgTask(void)
{
    NEW_TEST_CASE("\t* Clear Message Task");
    MsgAlloc_Init(NULL);
    {
        //        Last Message Task is cleared
        //
        //        Init state
        //               msg_tasks
        //               +---------+
        //               | Task A1 |
        //               |---------|
        //               +---------+
        //               | Task A2 |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               | LastTask|
        //               |---------|
        //
        //        Ending state
        //
        //               msg_tasks
        //               +---------+
        //               | Task A1 |
        //               |---------|
        //               +---------+
        //               | Task A2 |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|

        msg_t *expected_msg_tasks[MAX_MSG_NB];
        msg_tasks_stack_id = MAX_MSG_NB;

        // To avoid assert
        msg_tasks[0]         = (msg_t *)&msg_buffer[0];
        luos_tasks[0].msg_pt = (msg_t *)&msg_buffer[0];
        tx_tasks[0].data_pt  = (uint8_t *)&msg_buffer[0];

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            msg_tasks[i]          = (msg_t *)(&msg_buffer[0] + i);
            expected_msg_tasks[i] = (msg_t *)(&msg_buffer[0] + i + 1);
        }

        // Last Msg Task must be cleared
        expected_msg_tasks[MAX_MSG_NB - 1] = 0;
        MsgAlloc_ClearMsgTask();

        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            TEST_ASSERT_FALSE(is_assert());
            TEST_ASSERT_EQUAL(expected_msg_tasks[i], msg_tasks[i]);
        }
        TEST_ASSERT_EQUAL(MAX_MSG_NB - 1, msg_tasks_stack_id);
    }
}

void unittest_ClearLuosTask(void)
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
        uint16_t luos_task_id;

        NEW_STEP();
        for (uint16_t i = 0; i <= MAX_MSG_NB + 2; i++)
        {
            MsgAlloc_Init(NULL);
            luos_tasks_stack_id = i;
            for (uint16_t j = 0; j <= MAX_MSG_NB + 2; j++)
            {
                luos_task_id = j;
                reset_assert();
                if ((luos_task_id >= luos_tasks_stack_id) || (luos_tasks_stack_id > MAX_MSG_NB))
                {
                    MsgAlloc_ClearLuosTask(luos_task_id);
                    TEST_ASSERT_TRUE(is_assert());
                }
                else
                {
                    MsgAlloc_ClearLuosTask(luos_task_id);
                    TEST_ASSERT_FALSE(is_assert());
                }
            }
        }
    }

    NEW_TEST_CASE("\t* Clear Luos Tasks");
    MsgAlloc_Init(NULL);
    {
        //        Last Luos Task is cleared
        //
        //        Init state
        //               Luos_tasks
        //               +---------+
        //               | Task D1 |
        //               |---------|
        //               +---------+
        //               | Task D2 |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               | LastTask|
        //               |---------|
        //
        //        Ending state
        //
        //               Luos_tasks
        //               +---------+
        //               | Task D1 |
        //               |---------|
        //               +---------+
        //               | Task D2 |
        //               |---------|
        //               +---------+
        //               | etc...  |
        //               |---------|
        //               +---------+
        //               |    0    |
        //               |---------|

        luos_task_t expected_luos_tasks[MAX_MSG_NB];

        assert_activation(0);

        NEW_STEP();
        for (uint16_t task_id = 0; task_id < MAX_MSG_NB; task_id++)
        {
            for (uint16_t tasks_stack_id = task_id + 1; tasks_stack_id <= MAX_MSG_NB; tasks_stack_id++)
            {
                // Initialisation
                MsgAlloc_Init(NULL);
                luos_tasks_stack_id = tasks_stack_id;

                for (uint16_t pt_value = 0; pt_value < MAX_MSG_NB; pt_value++)
                {
                    // Init luos_tasks pointers
                    luos_tasks[pt_value].msg_pt          = (msg_t *)(&msg_buffer[0] + pt_value);
                    luos_tasks[pt_value].ll_container_pt = (ll_container_t *)(&msg_buffer[0] + pt_value);

                    // Init expected pointers
                    if (pt_value == (tasks_stack_id - 1))
                    {
                        expected_luos_tasks[pt_value].msg_pt          = 0;
                        expected_luos_tasks[pt_value].ll_container_pt = 0;
                    }
                    else if (pt_value < task_id)
                    {
                        expected_luos_tasks[pt_value].msg_pt          = (msg_t *)(&msg_buffer[0] + pt_value);
                        expected_luos_tasks[pt_value].ll_container_pt = (ll_container_t *)(&msg_buffer[0] + pt_value);
                    }
                    else
                    {
                        expected_luos_tasks[pt_value].msg_pt          = (msg_t *)(&msg_buffer[0] + pt_value + 1);
                        expected_luos_tasks[pt_value].ll_container_pt = (ll_container_t *)(&msg_buffer[0] + pt_value + 1);
                    }
                }

                // Launch test
                reset_assert();
                MsgAlloc_ClearLuosTask(task_id);

                // Analyze test results
                for (uint8_t i = 0; i < tasks_stack_id; i++)
                {
                    TEST_ASSERT_EQUAL(expected_luos_tasks[i].msg_pt, luos_tasks[i].msg_pt);
                    TEST_ASSERT_EQUAL(expected_luos_tasks[i].ll_container_pt, luos_tasks[i].ll_container_pt);
                }
            }
        }
        assert_activation(1);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    assert_activation(1);

    UNIT_TEST_RUN(unittest_DoWeHaveSpace);
    UNIT_TEST_RUN(unittest_CheckMsgSpace);
    UNIT_TEST_RUN(unittest_BufferAvailableSpaceComputation);
    UNIT_TEST_RUN(unittest_OldestMsgCandidate);
    UNIT_TEST_RUN(unittest_ClearMsgTask);
    UNIT_TEST_RUN(unittest_ClearLuosTask);
    UNIT_TEST_RUN(unittest_ClearMsgSpace);
    UNIT_TEST_RUN(unittest_ValidDataIntegrity);
    ////MsgAlloc_FindNewOldestMsg => this function doesn't need unit test

    UNITY_END();
}
