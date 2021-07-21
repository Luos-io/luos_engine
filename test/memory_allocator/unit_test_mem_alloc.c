#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../test/unit_test.h"
#include "../Robus/inc/robus_struct.h"
#include "../src/msg_alloc.c"

/*******************************************************************************
 * Function
 ******************************************************************************/
void unittest_MsgAlloc_loop()
{
    NEW_TEST_CASE("\t* Verify Rx message task stat computing");
    MsgAlloc_Init(NULL);
    {
        // Init variables
        //---------------
        uint16_t expected_stat;
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        // Call function and Verify
        //---------------------------
        // Tx and Rx stat ratio are incremented
        for (msg_tasks_stack_id = 0; msg_tasks_stack_id <= MAX_MSG_NB / 2; msg_tasks_stack_id++)
        {
            tx_tasks_stack_id = msg_tasks_stack_id;
            MsgAlloc_loop();
            expected_stat = (msg_tasks_stack_id * 100) / (MAX_MSG_NB);
            NEW_STEP();
            TEST_ASSERT_TRUE(memory_stats.tx_msg_stack_ratio == expected_stat);
            NEW_STEP();
            TEST_ASSERT_TRUE(memory_stats.rx_msg_stack_ratio == expected_stat);
        }
        // Tx and Rx stat ratio are not incremented
        for (msg_tasks_stack_id = MAX_MSG_NB; msg_tasks_stack_id > MAX_MSG_NB / 2; msg_tasks_stack_id--)
        {
            tx_tasks_stack_id = msg_tasks_stack_id;
            MsgAlloc_loop();
            NEW_STEP();
            TEST_ASSERT_TRUE(memory_stats.tx_msg_stack_ratio == 100);
            NEW_STEP();
            TEST_ASSERT_TRUE(memory_stats.rx_msg_stack_ratio == 100);
        }
    }

    NEW_TEST_CASE("\t* Verify buffer occupation rate stat computing");
    MsgAlloc_Init(NULL);
    {
        // Init variables
        //---------------
        uint32_t expected_buffer_occupation_ratio;
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);
        current_msg = (msg_t *)&msg_buffer[0];

        // Call function and Verify
        //---------------------------
        //Empty buffer
        data_end_estimation              = (uint8_t *)(current_msg);
        expected_buffer_occupation_ratio = 0;
        MsgAlloc_loop();

        NEW_STEP();
        TEST_ASSERT_EQUAL(0, memory_stats.buffer_occupation_ratio);

        //Buffer occupation from 0 -> 100%
        NEW_STEP();
        for (uint16_t i = sizeof(header_t); i < MSG_BUFFER_SIZE; i++)
        {
            data_end_estimation              = (uint8_t *)(current_msg) + i;
            expected_buffer_occupation_ratio = (i * 100) / (MSG_BUFFER_SIZE);

            MsgAlloc_loop();

            TEST_ASSERT_EQUAL(expected_buffer_occupation_ratio, memory_stats.buffer_occupation_ratio);
        }
    }
}

void unittest_MsgAlloc_ValidHeader()
{
#define DATA_SIZE 64
    NEW_TEST_CASE("\t* Invalid header");
    MsgAlloc_Init(NULL);
    {
        uint8_t valid      = false;
        uint16_t data_size = 0;

        data_ptr    = (uint8_t *)&msg_buffer[0];
        current_msg = (msg_t *)&msg_buffer[1];

        MsgAlloc_ValidHeader(valid, data_size);

        NEW_STEP();
        TEST_ASSERT_EQUAL((uint8_t *)current_msg, data_ptr);
    }

    NEW_TEST_CASE("\t* Drop message");
    MsgAlloc_Init(NULL);
    {
        uint8_t valid      = true;
        uint16_t data_size = DATA_SIZE;
        uint8_t *ptr       = (uint8_t *)&msg_buffer[0];

        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        current_msg = (msg_t *)ptr++;
        used_msg    = (msg_t *)ptr;

        MsgAlloc_ValidHeader(valid, data_size);

        NEW_STEP();
        TEST_ASSERT_EQUAL(100, memory_stats.buffer_occupation_ratio);
        NEW_STEP();
        TEST_ASSERT_EQUAL(1, memory_stats.msg_drop_number);
    }

    NEW_TEST_CASE("\t* There is no space in msg_buffer");
    MsgAlloc_Init(NULL);
    {
        uint8_t valid                        = true;
        uint16_t data_size                   = DATA_SIZE;
        current_msg                          = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - 1];
        header_t *expected_copy_task_pointer = (header_t *)current_msg;
        uint8_t *expected_data_ptr           = (uint8_t *)&msg_buffer[0] + sizeof(header_t);
        uint8_t *expected_data_end           = expected_data_ptr + data_size + CRC_SIZE;

        MsgAlloc_ValidHeader(valid, data_size);
        NEW_STEP();
        TEST_ASSERT_EQUAL(true, mem_clear_needed);
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_data_end, data_end_estimation);
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_copy_task_pointer, copy_task_pointer);
    }

    NEW_TEST_CASE("\t* There is enough space : save the end position and raise the clear flag");
    MsgAlloc_Init(NULL);
    {
        uint8_t *expected_data_end;
        uint8_t valid      = true;
        uint16_t data_size = DATA_SIZE;

        current_msg       = (msg_t *)&msg_buffer[0];
        expected_data_end = (uint8_t *)current_msg + sizeof(header_t) + data_size + CRC_SIZE;

        MsgAlloc_ValidHeader(valid, data_size);
        NEW_STEP();
        TEST_ASSERT_EQUAL(true, mem_clear_needed);
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_data_end, data_end_estimation);
    }
}

void unittest_MsgAlloc_InvalidMsg()
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
#define DATA_END_LIMIT (MSG_BUFFER_SIZE - sizeof(header_t) - CRC_SIZE)
        NEW_STEP();
        for (uint16_t i = 0; i < DATA_END_LIMIT; i++)
        {
            // Init variables
            //---------------
            current_msg = (msg_t *)&msg_buffer[i];
            reset_assert();

            // Call function
            //---------------
            MsgAlloc_InvalidMsg();

            // Verify
            //---------------
            TEST_ASSERT_FALSE(is_assert());
        }

        NEW_STEP();
        for (uint16_t i = DATA_END_LIMIT; i < MSG_BUFFER_SIZE; i++)
        {
            // Init variables
            //---------------
            current_msg = (msg_t *)&msg_buffer[i];
            reset_assert();

            // Call function
            //---------------
            MsgAlloc_InvalidMsg();

            // Verify
            //---------------
            TEST_ASSERT_TRUE(is_assert());
        }
        reset_assert();
    }

    NEW_TEST_CASE("\t* Check pointers values after invaliding a message");
    MsgAlloc_Init(NULL);
    {
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^--------------^----------------------------------------------+
        //        |              |
        //        current_msg    data_ptr
        //
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^---------------------^---------------------------------------+
        //        |                     |
        //        current_msg           data_end_estimation
        //        data_ptr
        //        |                     |
        //         <----Header + CRC---->
        //

#define DATA_END (sizeof(header_t) + CRC_SIZE)

        // Init variables
        //---------------
        current_msg = (msg_t *)&msg_buffer[0];
        data_ptr    = &msg_buffer[10];
        // Call function
        //---------------
        MsgAlloc_InvalidMsg();

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(current_msg, data_ptr);
        NEW_STEP();
        TEST_ASSERT_EQUAL(data_end_estimation, ((uint8_t *)current_msg + DATA_END));
        NEW_STEP();
        TEST_ASSERT_NULL(copy_task_pointer);

        // Init variables
        //---------------
        current_msg       = (msg_t *)&msg_buffer[100];
        data_ptr          = &msg_buffer[110];
        copy_task_pointer = (header_t *)data_ptr;

        // Call function
        //---------------
        MsgAlloc_InvalidMsg();

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(current_msg, data_ptr);
        NEW_STEP();
        TEST_ASSERT_EQUAL(data_end_estimation, ((uint8_t *)current_msg + DATA_END));
        NEW_STEP();
        TEST_ASSERT_NOT_NULL(copy_task_pointer);
    }

    NEW_TEST_CASE("\t* Clean memory");
    MsgAlloc_Init(NULL);
    {
        //        msg_buffer init
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        +------^-----------------^------^-------^---------------------+
        //               |                 |      |       |
        //          oldest_msg             |   data_ptr   |
        //                               Msg 1.......... Msg 10
        //
        //         msg_tasks init state          msg_tasks end state
        //             +---------+                  +---------+
        //             |  Msg 1  |                  |  Msg 6  |
        //             |---------|                  |---------|
        //             |  Msg 2  |                  |  Msg 7  |
        //             |---------|                  |---------|
        //             |  Msg 3  |                  |  Msg 8  |
        //             |---------|                  |---------|
        //             |  Msg 4  |                  |  Msg 9  |
        //             |---------|                  |---------|
        //             |  Msg 5  |                  |  Msg 10 |
        //             |---------|                  |---------|
        //             |  Msg 6  |                  |    0    |
        //             |---------|                  |---------|
        //             |  Msg 7  |                  |    0    |
        //             |---------|                  |---------|
        //             |  Msg 8  |                  |    0    |
        //             |---------|                  |---------|
        //             |  Msg 9  |                  |    0    |
        //             |---------|                  |---------|
        //             |  Msg 10 |                  |    0    |
        //             +---------+                  +---------+
        //

#define DATA_INDEX 15
        // Init variables
        //---------------
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        msg_t *expected_msg_tasks[MAX_MSG_NB];
        oldest_msg         = (msg_t *)&msg_buffer[DATA_INDEX - (MAX_MSG_NB / 2)];
        current_msg        = (msg_t *)oldest_msg;
        data_ptr           = &msg_buffer[DATA_INDEX - 1];
        mem_clear_needed   = true; // TRUE to clean memory
        msg_tasks_stack_id = MAX_MSG_NB;

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            msg_tasks[i] = (msg_t *)&msg_buffer[i + DATA_INDEX - (MAX_MSG_NB / 2)];
        }

        for (uint16_t i = 0; i < (MAX_MSG_NB / 2); i++)
        {
            expected_msg_tasks[i] = (msg_t *)msg_tasks[(i + (MAX_MSG_NB / 2))]; // 5 tasks must be shifted
        }

        for (uint16_t i = (MAX_MSG_NB / 2); i < MAX_MSG_NB; i++)
        {
            expected_msg_tasks[i] = 0; // other msg_tasks must be cleared
        }

        // Call function
        //---------------
        MsgAlloc_InvalidMsg();

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(false, mem_clear_needed);
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            NEW_STEP();
            TEST_ASSERT_EQUAL(expected_msg_tasks[i], msg_tasks[i]);
        }
    }
}

void unittest_MsgAlloc_EndMsg()
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
#define assert_nb 6
        typedef struct
        {
            bool expected_asserts;
            uint16_t stack_id;
            msg_t *tasks;
            msg_t *old_message;
        } assert_scenario;

        assert_scenario assert_sc[assert_nb];

        // Expected Values
        assert_sc[0].expected_asserts = true;
        assert_sc[0].tasks            = (msg_t *)&msg_buffer[0];
        assert_sc[0].stack_id         = 0;
        assert_sc[0].old_message      = 0;

        assert_sc[1].expected_asserts = false;
        assert_sc[1].tasks            = 0;
        assert_sc[1].stack_id         = 0;
        assert_sc[1].old_message      = (msg_t *)&msg_buffer[0];

        assert_sc[2].expected_asserts = false;
        assert_sc[2].tasks            = 0;
        assert_sc[2].stack_id         = 1;
        assert_sc[2].old_message      = (msg_t *)&msg_buffer[0];

        assert_sc[3].expected_asserts = false;
        assert_sc[3].tasks            = 0;
        assert_sc[3].stack_id         = 0;
        assert_sc[3].old_message      = (msg_t *)&msg_buffer[MAX_MSG_NB * sizeof(msg_t)];

        assert_sc[4].expected_asserts = true;
        assert_sc[4].tasks            = 0;
        assert_sc[4].stack_id         = 1;
        assert_sc[4].old_message      = (msg_t *)&msg_buffer[MAX_MSG_NB * sizeof(msg_t)];

        assert_sc[5].expected_asserts = true;
        assert_sc[5].tasks            = 0;
        assert_sc[5].stack_id         = 1;
        assert_sc[5].old_message      = (msg_t *)&luos_tasks;

        // Launch test
        for (uint8_t i = 0; i < assert_nb; i++)
        {
            reset_assert();
            // Init variables
            //---------------
            current_msg                   = (msg_t *)&msg_buffer[0];
            msg_tasks_stack_id            = assert_sc[i].stack_id;
            msg_tasks[0]                  = assert_sc[i].old_message;
            msg_tasks[msg_tasks_stack_id] = assert_sc[i].tasks;

            // Call function
            //---------------
            MsgAlloc_EndMsg();

            // Verify
            //---------------
            NEW_STEP();
            TEST_ASSERT_EQUAL(assert_sc[i].expected_asserts, is_assert());
        }
        reset_assert();
    }
    reset_assert();

    NEW_TEST_CASE("\t* Prepare the next message : update pointers");
    MsgAlloc_Init(NULL);
    {
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;

        // Init variables
        //---------------
        if ((uint32_t)&msg_buffer[10] % 2 == 0)
        {
            data_ptr = &msg_buffer[10]; // Data is already aligned
        }
        else
        {
            data_ptr = &msg_buffer[9]; // Align data
        }
        expected_data_ptr            = (uint8_t *)(data_ptr - CRC_SIZE);
        expected_data_end_estimation = (uint8_t *)(data_ptr + sizeof(header_t));

        // Call function
        //---------------
        reset_assert();
        MsgAlloc_EndMsg();

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(false, is_assert());
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP();
        TEST_ASSERT_EQUAL(data_ptr, current_msg);
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);
        NEW_STEP();
        TEST_ASSERT_EQUAL(true, mem_clear_needed);
        NEW_STEP();
        TEST_ASSERT_EQUAL(1, msg_tasks_stack_id);
    }

    NEW_TEST_CASE("\t* Data are not aligned");
    MsgAlloc_Init(NULL);
    {
        uint8_t *expected_data_ptr;

        // Init variables
        //---------------
        if ((uint32_t)&msg_buffer[10] % 2 == 0)
        {
            data_ptr = &msg_buffer[9]; // Data pointer's address is not aligned
        }
        else
        {
            data_ptr = &msg_buffer[10]; // Data pointer's address is not aligned
        }

        expected_data_ptr = (uint8_t *)(data_ptr - CRC_SIZE + 1);

        // Call function
        //---------------
        MsgAlloc_EndMsg();

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
    }

    NEW_TEST_CASE("\t* Verify Message task allocation");
    MsgAlloc_Init(NULL);
    {
        MsgAlloc_EndMsg();
        NEW_STEP();
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[0], msg_tasks[0]);
        NEW_STEP();
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[0], oldest_msg);
    }

    NEW_TEST_CASE("\t* Not enough space for the next message");
    MsgAlloc_Init(NULL);
    {
        // Init variables
        //---------------
        data_ptr = &msg_buffer[MSG_BUFFER_SIZE - 1];

        // Call function
        //---------------
        MsgAlloc_EndMsg();

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[0], data_ptr);
        NEW_STEP();
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[0], current_msg);
    }

    NEW_TEST_CASE("\t* Remove oldest msg if message task buffer is full");
    MsgAlloc_Init(NULL);
    {
        msg_t *expected_msg_tasks[MAX_MSG_NB];
        // Init variables
        //---------------
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

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
        msg_tasks_stack_id = MAX_MSG_NB;

        // Call function
        //---------------
        MsgAlloc_EndMsg();

        // Verify
        //---------------
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB - 1; i++)
        {
            TEST_ASSERT_EQUAL(expected_msg_tasks[i], msg_tasks[i]);
        }
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[0], msg_tasks[MAX_MSG_NB - 1]);
    }
}

#define COPY_LENGTH 128
void unittest_MsgAlloc_SetData()
{
    NEW_TEST_CASE("\t* Set Data");
    MsgAlloc_Init(NULL);
    {
        uint8_t expected_datas[COPY_LENGTH];

        // Init variables
        //---------------
        data_ptr = (uint8_t *)&msg_buffer[0];
        memset((void *)&msg_buffer[0], 0xFF, COPY_LENGTH);
        for (uint8_t i = 0; i < COPY_LENGTH; i++)
        {
            expected_datas[i] = i;
        }

        // Call function
        //---------------
        for (uint8_t i = 0; i < COPY_LENGTH; i++)
        {
            MsgAlloc_SetData(i);
        }

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_EQUAL_MEMORY(expected_datas, (uint8_t *)&msg_buffer[0], COPY_LENGTH);
    }
}

void unittest_MsgAlloc_IsEmpty()
{
    NEW_TEST_CASE("\t* Is Empty");
    MsgAlloc_Init(NULL);
    {
        data_ptr = &msg_buffer[0];
        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_IsEmpty());

        NEW_STEP();
        for (uint16_t i = 1; i < MSG_BUFFER_SIZE; i++)
        {
            data_ptr = &msg_buffer[i];
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_IsEmpty());
        }
    }
}

void unittest_MsgAlloc_UsedMsgEnd()
{
    NEW_TEST_CASE("\t* Used Message End");
    MsgAlloc_Init(NULL);
    {
        used_msg = (msg_t *)&msg_buffer[0];
        MsgAlloc_UsedMsgEnd();

        NEW_STEP();
        TEST_ASSERT_NULL(used_msg);
    }
}

void unittest_MsgAlloc_PullMsgToInterpret()
{
    NEW_TEST_CASE("\t* Case FAILED");
    MsgAlloc_Init(NULL);
    {
        msg_t *returned_msg = NULL;
        msg_tasks_stack_id  = 0;

        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_PullMsgToInterpret(&returned_msg));
    }

    NEW_TEST_CASE("\t* Case SUCCEED");
    MsgAlloc_Init(NULL);
    {
        msg_t *expected_message[MAX_MSG_NB];
        msg_t *returned_msg;
        error_return_t result;

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            expected_message[i] = (msg_t *)&msg_buffer[i];
            msg_tasks[i]        = (msg_t *)&msg_buffer[i];
        }

        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            msg_tasks_stack_id = MAX_MSG_NB;
            reset_assert();
            result = MsgAlloc_PullMsgToInterpret(&returned_msg);
            TEST_ASSERT_FALSE(is_assert());
            TEST_ASSERT_EQUAL(SUCCEED, result);
            TEST_ASSERT_EQUAL(expected_message[i], returned_msg);
        }
    }
}

void unittest_MsgAlloc_LuosTaskAlloc()
{
    NEW_TEST_CASE("\t* No more space in luos_tasks");
    MsgAlloc_Init(NULL);
    {
        //
        //         luos_tasks init state         luos_tasks end state
        //             +---------+                  +---------+
        //             |   D 1   |                  |    0    |
        //             |---------|                  |---------|
        //             |   D 2   |                  |   D 2   |
        //             |---------|                  |---------|
        //             |  etc... |                  |  etc... |
        //             |---------|                  |---------|
        //             |  Last   |                  |  Last   |
        //             +---------+                  +---------+
        //

        luos_task_t expected_luos_task;
        ll_container_t container;

        // Init variables
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};

        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        luos_tasks_stack_id = MAX_MSG_NB;
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].msg_pt = (msg_t *)&msg_buffer[i];
        }
        luos_tasks[0].msg_pt = NULL;

        // Launch Test
        MsgAlloc_LuosTaskAlloc(&container, (msg_t *)&msg_buffer[0]);

        // Verify
        NEW_STEP();
        TEST_ASSERT_EQUAL(100, memory_stats.luos_stack_ratio);
        NEW_STEP();
        TEST_ASSERT_EQUAL(1, memory_stats.msg_drop_number);
        NEW_STEP();
        TEST_ASSERT_EQUAL(&msg_buffer[1], luos_tasks[0].msg_pt);
    }

    NEW_TEST_CASE("\t* Allocation");
    MsgAlloc_Init(NULL);
    {
        //
        //         luos_tasks init state                           luos_tasks end state
        //
        //             +---------+<--luos_tasks_stack_id              +---------+
        //             |    0    |                                    |   D 1   | (msg_pt & ll_container_pt are allocated)
        //             |---------|                                    |---------|
        //             |    0    |                                    |   D 2   | (msg_pt & ll_container_pt are allocated)
        //             |---------|                                    |---------|
        //             |  etc... |                                    |  etc... |
        //             |---------|                                    |---------|
        //             |    0    |                                    |  Last   | (msg_pt & ll_container_pt are allocated)
        //             +---------+              luos_tasks_stack_id-->+---------+
        //

        msg_t *message;
        ll_container_t *container_concerned;
        uint16_t expected_luos_tasks_stack_id;
        uint8_t expected_mem_stat;

        // Init variables
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .luos_stack_ratio        = 0,
                                       .tx_msg_stack_ratio      = 0,
                                       .buffer_occupation_ratio = 0,
                                       .msg_drop_number         = 0};
        memset(&memory_stats, 0, sizeof(memory_stats));
        MsgAlloc_Init(&memory_stats);

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            // Init variables
            luos_tasks_stack_id          = i;
            expected_luos_tasks_stack_id = i + 1;
            expected_mem_stat            = (i + 1) * 10;
            message                      = (msg_t *)&msg_buffer[0];
            container_concerned          = (ll_container_t *)&msg_buffer[0];

            // Launch Test
            MsgAlloc_LuosTaskAlloc(container_concerned, message);

            // Verify
            NEW_STEP();
            TEST_ASSERT_EQUAL(message, luos_tasks[i].msg_pt);
            NEW_STEP();
            TEST_ASSERT_EQUAL(container_concerned, luos_tasks[i].ll_container_pt);
            NEW_STEP();
            TEST_ASSERT_EQUAL(expected_luos_tasks_stack_id, luos_tasks_stack_id);
            NEW_STEP();
            TEST_ASSERT_EQUAL(luos_tasks[0].msg_pt, oldest_msg);
            NEW_STEP();
            TEST_ASSERT_EQUAL(expected_mem_stat, memory_stats.luos_stack_ratio);
        }
    }
}

void unittest_MsgAlloc_PullMsg()
{
    NEW_TEST_CASE("\t* Case FAILED : no message for the container");
    MsgAlloc_Init(NULL);
    {
        msg_t *returned_message;
        ll_container_t *module = (ll_container_t *)0xFFFF;

        // Init variables
        luos_tasks_stack_id = MAX_MSG_NB - 1;
        for (uint32_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].ll_container_pt = (ll_container_t *)i;
        }

        // Launch Test & Verify
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_PullMsg(module, &returned_message));
    }

    NEW_TEST_CASE("\t* Case SUCCEED : retunr oldest message for the container");
    MsgAlloc_Init(NULL);
    {
        msg_t *returned_message;
        msg_t *msg_to_clear;
        ll_container_t *module;
        luos_task_t expected_luos_tasks[MAX_MSG_NB];

        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            // Init variables
            MsgAlloc_Init(NULL);
            luos_tasks_stack_id = MAX_MSG_NB;
            for (uint16_t j = 0; j < MAX_MSG_NB; j++)
            {
                luos_tasks[j].ll_container_pt = (ll_container_t *)&msg_buffer[j];
                luos_tasks[j].msg_pt          = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
                expected_luos_tasks[j].msg_pt = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
            }
            module       = luos_tasks[i].ll_container_pt;
            msg_to_clear = luos_tasks[i].msg_pt;

            // Launch Test & Verify
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_PullMsg(module, &returned_message));
            TEST_ASSERT_EQUAL(expected_luos_tasks[i].msg_pt, returned_message);
            TEST_ASSERT_EQUAL(returned_message, used_msg);
            // Verify required message has been deleted
            for (uint16_t k = 0; k < MAX_MSG_NB; k++)
            {
                TEST_ASSERT_NOT_EQUAL(msg_to_clear, luos_tasks[k].msg_pt);
            }
        }
    }
}

void unittest_MsgAlloc_PullMsgFromLuosTask()
{
    NEW_TEST_CASE("\t* Case FAILED : no message for ID");
    MsgAlloc_Init(NULL);
    {
        msg_t *returned_msg = NULL;
        luos_tasks_stack_id = 5;

        for (uint16_t task_id = luos_tasks_stack_id; task_id < luos_tasks_stack_id + 5; task_id++)
        {
            NEW_STEP();
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_PullMsgFromLuosTask(task_id, &returned_msg));
        }
    }

    NEW_TEST_CASE("\t* Case SUCCEED : pull all messages from Luos Tasks");
    MsgAlloc_Init(NULL);
    {
        uint16_t task_id;
        msg_t *returned_message;
        msg_t *msg_to_clear;
        ll_container_t *module;
        luos_task_t expected_luos_tasks[MAX_MSG_NB];

        NEW_STEP();
        for (uint16_t task_id = 0; task_id < MAX_MSG_NB; task_id++)
        {
            // Init variables
            MsgAlloc_Init(NULL);
            luos_tasks_stack_id = MAX_MSG_NB;
            for (uint16_t j = 0; j < MAX_MSG_NB; j++)
            {
                luos_tasks[j].msg_pt          = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
                expected_luos_tasks[j].msg_pt = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
            }
            msg_to_clear = luos_tasks[task_id].msg_pt;

            // Launch Test & Verify
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_PullMsgFromLuosTask(task_id, &returned_message));
            TEST_ASSERT_EQUAL(expected_luos_tasks[task_id].msg_pt, returned_message);
            TEST_ASSERT_EQUAL(returned_message, used_msg);
            // Verify required message has been deleted
            for (uint16_t k = 0; k < MAX_MSG_NB; k++)
            {
                TEST_ASSERT_NOT_EQUAL(msg_to_clear, luos_tasks[k].msg_pt);
            }
        }
    }
}

void unittest_MsgAlloc_LookAtLuosTask()
{
    NEW_TEST_CASE("\t* Case FAILED");
    MsgAlloc_Init(NULL);
    {
        uint16_t task_id;
        ll_container_t **allocated_module;

        // Init variables
        luos_tasks_stack_id = 0;
        task_id             = luos_tasks_stack_id;

        // Call function & Verify
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_LookAtLuosTask(task_id, allocated_module));
        task_id++;
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_LookAtLuosTask(task_id, allocated_module));
    }

    NEW_TEST_CASE("\t* Case SUCCEED");
    MsgAlloc_Init(NULL);
    {
        // Init variables
        ll_container_t *oldest_ll_container = NULL;
        luos_tasks_stack_id                 = MAX_MSG_NB;

        for (uint32_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].ll_container_pt = (ll_container_t *)i;
        }

        // Call function & Verify
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_LookAtLuosTask(i, &oldest_ll_container));
            TEST_ASSERT_EQUAL(i, oldest_ll_container);
        }
    }
}

void unittest_MsgAlloc_GetLuosTaskSourceId()
{
    NEW_TEST_CASE("\t* Case FAILED");
    MsgAlloc_Init(NULL);
    {
        uint16_t task_id;
        uint16_t task_id_2;

        // Init variables
        luos_tasks_stack_id = MAX_MSG_NB;
        task_id             = luos_tasks_stack_id;
        task_id_2           = luos_tasks_stack_id + 1;

        // Call function & Verify
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskSourceId(task_id, NULL));
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskSourceId(task_id_2, NULL));
    }

    NEW_TEST_CASE("\t* Case SUCCEED");
    MsgAlloc_Init(NULL);
    {
        // Init variables
        msg_t message;
        uint16_t source_id  = 0;
        luos_tasks_stack_id = MAX_MSG_NB;

        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            message.header.source = i;
            luos_tasks[i].msg_pt  = &message;

            // Call function & Verify
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetLuosTaskSourceId(i, &source_id));
            TEST_ASSERT_EQUAL(i, source_id);
        }
    }
}

void unittest_MsgAlloc_GetLuosTaskCmd()
{
    NEW_TEST_CASE("\t* FAILED");
    MsgAlloc_Init(NULL);
    {
        uint16_t task_id;
        uint8_t command = 0xFF;

        luos_tasks_stack_id = MAX_MSG_NB;
        task_id             = luos_tasks_stack_id;

        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskCmd(task_id, &command));
        NEW_STEP();
        TEST_ASSERT_EQUAL(0xFF, command);

        task_id++;
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskCmd(task_id, &command));
        NEW_STEP();
        TEST_ASSERT_EQUAL(0xFF, command);
    }

    NEW_TEST_CASE("\t* SUCCEED");
    MsgAlloc_Init(NULL);
    {
        msg_t message;
        uint16_t task_id         = 0;
        uint8_t command          = 0;
        uint8_t expected_command = 1;

        luos_tasks_stack_id = MAX_MSG_NB;

        message.header.cmd         = expected_command;
        luos_tasks[task_id].msg_pt = &message;

        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetLuosTaskCmd(task_id, &command));
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_command, command);
    }
}

void unittest_MsgAlloc_GetLuosTaskSize()
{
    NEW_TEST_CASE("\t* FAILED");
    MsgAlloc_Init(NULL);
    {
        uint16_t task_id;
        uint16_t size       = 0xFF;
        luos_tasks_stack_id = MAX_MSG_NB;
        task_id             = luos_tasks_stack_id;

        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskSize(task_id, &size));
        NEW_STEP();
        TEST_ASSERT_EQUAL(0xFF, size);
    }

    NEW_TEST_CASE("\t* SUCCEED");
    MsgAlloc_Init(NULL);
    {
        msg_t message;
        uint16_t expected_size = 128;
        uint16_t task_id       = 0;
        uint16_t size          = 0;

        luos_tasks_stack_id = MAX_MSG_NB;

        message.header.size        = expected_size;
        luos_tasks[task_id].msg_pt = &message;

        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetLuosTaskSize(task_id, &size));
        NEW_STEP();
        TEST_ASSERT_EQUAL(expected_size, size);
    }
}

void unittest_MsgAlloc_ClearMsgFromLuosTasks()
{
    NEW_TEST_CASE("\t* No message to clear");
    MsgAlloc_Init(NULL);
    {
        //      Message to clean is not in Luos_tasks : nothing is done
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        +----------^^................^---------------^----------------+
        //                   ||                |               |
        //                   ||   Luos_tasks   |         msg  to clean
        //                   ||  +---------+   |
        //                   +|->|    D1   |   |
        //                    |  |---------|   |
        //                    +->|    D2   |   |
        //                       |---------|   |
        //                       |  etc... |   |
        //                       |---------|   |
        //                       |  Last   |<--+
        //                       +---------+
        //

        luos_task_t expected_luos_tasks[MAX_MSG_NB];
        msg_t message;
        ll_container_t container;

        // Init variables
        luos_tasks_stack_id = 0;
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].msg_pt                   = &message;
            luos_tasks[i].ll_container_pt          = &container;
            expected_luos_tasks[i].msg_pt          = &message;
            expected_luos_tasks[i].ll_container_pt = &container;
            // Call function
            MsgAlloc_ClearMsgFromLuosTasks(luos_tasks[i].msg_pt);

            TEST_ASSERT_EQUAL(expected_luos_tasks[i].msg_pt, luos_tasks[i].msg_pt);
            TEST_ASSERT_EQUAL(expected_luos_tasks[i].ll_container_pt, luos_tasks[i].ll_container_pt);
        }
    }

    NEW_TEST_CASE("\t* Clear a specific Luos Task");
    MsgAlloc_Init(NULL);
    {
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        +----------^^.......^..................^----------------------+
        //                   ||       |                  |
        //                   || msg to clean = D2        |
        //                   ||                          |
        //                   ||   Luos_tasks init state  |
        //                   ||  +---------+             |
        //                   +|->|    D1   |             |
        //                    |  |---------|             |
        //                    +->|    D2   |             |
        //                       |---------|             |
        //                       |  etc... |             |
        //                       |---------|             |
        //                       |  Last   |<------------+
        //                       +---------+
        //
        //                   Luos_tasks azerty ending state
        //                       +---------+
        //                       |    D1   |
        //                       |---------|
        //                       |    D3   |
        //                       |---------|
        //                       |  etc... |
        //                       |---------|
        //                       |  Last   |
        //                       |---------|
        //                       |    0    |
        //                       +---------+
        //

        msg_t *msg_to_clear;

        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            // Init variables
            MsgAlloc_Init(NULL);
            for (uint16_t j = 0; j < MAX_MSG_NB; j++)
            {
                luos_tasks[j].msg_pt = (msg_t *)&msg_buffer[j];
            }

            msg_tasks[0]        = 0;
            tx_tasks[0].data_pt = 0;
            luos_tasks_stack_id = MAX_MSG_NB;
            msg_to_clear        = luos_tasks[i].msg_pt;
            // Call function
            MsgAlloc_ClearMsgFromLuosTasks(luos_tasks[i].msg_pt);

            // Verify required message has been deleted
            for (uint16_t k = 0; k < MAX_MSG_NB; k++)
            {
                TEST_ASSERT_NOT_EQUAL(msg_to_clear, luos_tasks[k].msg_pt);
            }
        }
    }
}

void unittest_MsgAlloc_PullMsgFromTxTask()
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB + 2; i++)
        {
            MsgAlloc_Init(NULL);
            reset_assert();
            tx_tasks_stack_id = i;

            if (tx_tasks_stack_id == 0)
            {
                MsgAlloc_PullMsgFromTxTask();
                TEST_ASSERT_TRUE(is_assert());
            }
            else if (tx_tasks_stack_id > MAX_MSG_NB)
            {
                MsgAlloc_PullMsgFromTxTask();
                TEST_ASSERT_TRUE(is_assert());
            }
            else
            {
                MsgAlloc_PullMsgFromTxTask();
                TEST_ASSERT_FALSE(is_assert());
            }
        }
        reset_assert();
    }

    NEW_TEST_CASE("\t* Pull Tx message task");
    MsgAlloc_Init(NULL);
    {
        tx_task_t expected_tx_tasks[MAX_MSG_NB];

        NEW_STEP();
        for (uint16_t task_id = 0; task_id < MAX_MSG_NB; task_id++)
        {
            for (uint16_t tasks_stack_id = task_id + 1; tasks_stack_id <= MAX_MSG_NB; tasks_stack_id++)
            {
                // Initialisation
                MsgAlloc_Init(NULL);
                tx_tasks_stack_id = tasks_stack_id;

                for (uint16_t pt_value = 0; pt_value < MAX_MSG_NB; pt_value++)
                {
                    // Init tx_tasks pointers
                    tx_tasks[pt_value].data_pt = (uint8_t *)(&msg_buffer[0] + pt_value);
                    tx_tasks[pt_value].size    = pt_value + 100;

                    // Init expected pointers
                    if (pt_value == (tasks_stack_id - 1))
                    {
                        expected_tx_tasks[pt_value].data_pt = 0;
                        expected_tx_tasks[pt_value].size    = 0;
                    }
                    else if (pt_value < task_id)
                    {
                        expected_tx_tasks[pt_value].data_pt = (uint8_t *)(&msg_buffer[0] + pt_value + 1);
                        expected_tx_tasks[pt_value].size    = pt_value + 100 + 1;
                    }
                }

                // Launch test
                MsgAlloc_PullMsgFromTxTask();

                // Analyze test results
                for (uint8_t i = 0; i < task_id; i++)
                {
                    TEST_ASSERT_EQUAL(expected_tx_tasks[i].data_pt, tx_tasks[i].data_pt);
                    TEST_ASSERT_EQUAL(expected_tx_tasks[i].size, tx_tasks[i].size);
                }
            }
        }
    }
}

void unittest_MsgAlloc_PullContainerFromTxTask()
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
        uint16_t container_id = 0;

        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB + 2; i++)
        {
            MsgAlloc_Init(NULL);
            reset_assert();
            tx_tasks_stack_id = i;

            if (tx_tasks_stack_id == 0)
            {
                MsgAlloc_PullContainerFromTxTask(container_id);
                TEST_ASSERT_TRUE(is_assert());
            }
            else if (tx_tasks_stack_id > MAX_MSG_NB)
            {
                MsgAlloc_PullContainerFromTxTask(container_id);
                TEST_ASSERT_TRUE(is_assert());
            }
        }
        reset_assert();
    }

    NEW_TEST_CASE("\t* Remove Tx tasks from a container");
    MsgAlloc_Init(NULL);
    {
        tx_task_t expected_tx_tasks[MAX_MSG_NB];
        uint16_t container_id;

        // Init variables
        //---------------
        tx_tasks_stack_id = 10;

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            msg_tasks[i]                = (msg_t *)&msg_buffer[i * 20];
            msg_tasks[i]->header.target = i + 1;
            tx_tasks[i].data_pt         = (uint8_t *)msg_tasks[i];
        }

        // Remove all messages from container 15 (position 0,2,4 in buffer)
        container_id                = 15;
        msg_tasks[0]->header.target = container_id;
        msg_tasks[2]->header.target = container_id;
        msg_tasks[4]->header.target = container_id;

        expected_tx_tasks[0].data_pt = tx_tasks[1].data_pt;
        expected_tx_tasks[1].data_pt = tx_tasks[3].data_pt;
        for (uint16_t i = 2; i < MAX_MSG_NB - 3; i++)
        {
            expected_tx_tasks[i].data_pt = tx_tasks[i + 3].data_pt;
        }
        expected_tx_tasks[MAX_MSG_NB - 1].data_pt = 0;
        expected_tx_tasks[MAX_MSG_NB - 2].data_pt = 0;
        expected_tx_tasks[MAX_MSG_NB - 3].data_pt = 0;

        // Call function
        //---------------
        MsgAlloc_PullContainerFromTxTask(container_id);

        // Verify
        //---------------
        NEW_STEP();
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            TEST_ASSERT_EQUAL(expected_tx_tasks[i].data_pt, tx_tasks[i].data_pt);
        }
    }
}

void unittest_MsgAlloc_GetTxTask()
{
    NEW_TEST_CASE("\t* Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
        ll_container_t *ll_container;
        uint8_t *data;
        uint16_t *size;
        uint8_t *localhost;

        // Init variables
        //---------------
        reset_assert();
        tx_tasks_stack_id = MAX_MSG_NB;

        // Call function
        //---------------
        MsgAlloc_GetTxTask(&ll_container, &data, size, localhost);

        NEW_STEP();
        TEST_ASSERT_TRUE(is_assert());

        // Init variables
        //---------------
        reset_assert();
        tx_tasks_stack_id = MAX_MSG_NB + 1;

        // Call function
        //---------------
        MsgAlloc_GetTxTask(&ll_container, &data, size, localhost);

        // Verify
        //---------------
        NEW_STEP();
        TEST_ASSERT_TRUE(is_assert());

        reset_assert();
    }

    NEW_TEST_CASE("\t* Verify there are no messages");
    MsgAlloc_Init(NULL);
    {
        ll_container_t *ll_container;
        uint8_t *data;
        uint16_t *size;
        uint8_t *localhost;

        // Init variables
        //---------------
        tx_tasks_stack_id = 0;

        // Call function & Verify
        //--------------------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetTxTask(&ll_container, &data, size, localhost));
    }

    NEW_TEST_CASE("\t* Verify there is a message");
    MsgAlloc_Init(NULL);
    {
        ll_container_t *ll_container;
        uint8_t *data;
        uint16_t size;
        uint8_t localhost;

        // Init variables
        //---------------
        tx_tasks_stack_id = 1;

        tx_tasks[0].data_pt         = (uint8_t *)16;
        tx_tasks[0].ll_container_pt = (ll_container_t *)32;
        tx_tasks[0].size            = 128;
        tx_tasks[0].localhost       = 1;

        // Call function & Verify
        //--------------------------
        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetTxTask(&ll_container, &data, &size, &localhost));
        NEW_STEP();
        TEST_ASSERT_EQUAL(tx_tasks[0].ll_container_pt, ll_container);
        NEW_STEP();
        TEST_ASSERT_EQUAL(tx_tasks[0].data_pt, data);
        NEW_STEP();
        TEST_ASSERT_EQUAL(tx_tasks[0].size, size);
        NEW_STEP();
        TEST_ASSERT_EQUAL(tx_tasks[0].localhost, localhost);
    }
}

void unittest_MsgAlloc_TxAllComplete()
{
    NEW_TEST_CASE("\t* Tx All Complete");
    MsgAlloc_Init(NULL);
    {
        tx_tasks_stack_id = 0;
        NEW_STEP();
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_TxAllComplete());

        NEW_STEP();
        while (tx_tasks_stack_id < MSG_BUFFER_SIZE)
        {
            tx_tasks_stack_id++;
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_TxAllComplete());
        }
    }
}

/*******************************************************************************
 * MAIN
 ******************************************************************************/
int main(int argc, char **argv)
{
    UNITY_BEGIN();

    assert_activation(1);

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
    UNIT_TEST_RUN(unittest_MsgAlloc_PullMsg);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullMsgFromLuosTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_LookAtLuosTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_ClearMsgFromLuosTasks);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullMsgFromTxTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_PullContainerFromTxTask);
    UNIT_TEST_RUN(unittest_MsgAlloc_GetTxTask);

    //MsgAlloc_Init         => this function doesn't need unit test
    //MsgAlloc_LuosTasksNbr => this function doesn't need unit test
    UNITY_END();
}
