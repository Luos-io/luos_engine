#include "../test/unit_test.h"
#include "../src/msg_alloc.c"

/*******************************************************************************
 * Function
 ******************************************************************************/
void unittest_MsgAlloc_loop()
{
    NEW_TEST_CASE("Verify Tx & Rx message tasks stat computing");
    MsgAlloc_Init(NULL);
    {
        // tx_tasks_stack_id is incremented : buffer occupation computing is verified
        //
        //        tx/rx buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------------^---------------------^-------------+
        //        |                         |                     |
        //  tx/rx_tasks_stack_id    tx/rx_tasks_stack_id    tx/rx_tasks_stack_id
        //    (0% occupation)          (X% occupation)         (etc...)
        //

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
            NEW_STEP_IN_LOOP("Check Tx message stack ratio is correct", msg_tasks_stack_id);
            TEST_ASSERT_TRUE(memory_stats.tx_msg_stack_ratio == expected_stat);
            NEW_STEP_IN_LOOP("Check Rx message stack ratio is correct", msg_tasks_stack_id);
            TEST_ASSERT_TRUE(memory_stats.rx_msg_stack_ratio == expected_stat);
        }
        // Tx and Rx stat ratio are not incremented
        for (msg_tasks_stack_id = MAX_MSG_NB; msg_tasks_stack_id > MAX_MSG_NB / 2; msg_tasks_stack_id--)
        {
            tx_tasks_stack_id = msg_tasks_stack_id;
            MsgAlloc_loop();
            NEW_STEP_IN_LOOP("Check Tx message stack ratio is not incremented", msg_tasks_stack_id);
            TEST_ASSERT_TRUE(memory_stats.tx_msg_stack_ratio == 100);
            NEW_STEP_IN_LOOP("Check Rx message stack ratio is not incremented", msg_tasks_stack_id);
            TEST_ASSERT_TRUE(memory_stats.rx_msg_stack_ratio == 100);
        }
    }

    NEW_TEST_CASE("Verify buffer occupation rate stat computing");
    MsgAlloc_Init(NULL);
    {
        // data_end_estimation is incremented : buffer occupation computing is verified
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------------^---------------------^-------------+
        //        |                         |                     |
        //    data_end_estimation       data_end_estimation   data_end_estimation
        //      (0% occupation)          (X% occupation)         (etc...)
        //

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

        NEW_STEP("Check buffer is empty");
        TEST_ASSERT_EQUAL(0, memory_stats.buffer_occupation_ratio);

        //Buffer occupation from 0 -> 100%
        NEW_STEP("Check buffer is full in all cases");
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
    NEW_TEST_CASE("Invalid header");
    MsgAlloc_Init(NULL);
    {
        // valid = FALSE => the header is invalidated
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-------------------|  Header  |------------------------------|
        //        +-------------------^----------^------------------------------+
        //                            |          |
        //                      current_msg     data_ptr
        //
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-------------------|  ******  |------------------------------|
        //        +-------------------^-----------------------------------------+
        //                            |
        //                      current_msg
        //                      data_ptr
        //

        uint8_t valid      = false;
        uint16_t data_size = 0;

        data_ptr    = (uint8_t *)&msg_buffer[0];
        current_msg = (msg_t *)&msg_buffer[1];

        MsgAlloc_ValidHeader(valid, data_size);

        NEW_STEP("Check header is invalid: \"data pointer\" is reseted to \"current message\"");
        TEST_ASSERT_EQUAL((uint8_t *)current_msg, data_ptr);
    }

    NEW_TEST_CASE("Drop message");
    MsgAlloc_Init(NULL);
    {
        //  There is a used message pending in current message memory
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-------------------|  Header  | Datas to be received |-------|
        //        |----------------------------------------| An old message |---|
        //        +-------------------^----------^---------^--------------------+
        //                            |          |         |
        //                      current_msg     data_ptr  used_msg
        //
        //
        //        msg_buffer ending state : old message is cleared (used_msg = NULL)
        //        +-------------------------------------------------------------+
        //        |-------------------|  Header  | Datas to be received |-------|
        //        +-------------------^----------^------------------------------+
        //                            |          |
        //                      current_msg     data_ptr
        //

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

        NEW_STEP("Check buffer is full");
        TEST_ASSERT_EQUAL(100, memory_stats.buffer_occupation_ratio);
        NEW_STEP("Check there is 1 message dropped");
        TEST_ASSERT_EQUAL(1, memory_stats.msg_drop_number);
    }

    NEW_TEST_CASE("There is no space in msg_buffer");
    MsgAlloc_Init(NULL);
    {
        //  There is a no space left
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |------------------------------------|  Header  | Datas to be received |
        //        +------------------------------------^----------^-------------+        ^
        //                                             |          |                      |
        //                                       current_msg     data_ptr         data_end_estimation
        //
        //
        //        msg_buffer ending state :
        //        +-------------------------------------------------------------+
        //        |------------------------------------|  Header  | Datas to be received |
        //        +----------^-------------------------^------------------------+
        //        |          |                         |
        //     current_msg  data_ptr             copy_task_pointer
        //

        uint8_t valid                        = true;
        uint16_t data_size                   = DATA_SIZE;
        current_msg                          = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - 1];
        header_t *expected_copy_task_pointer = (header_t *)current_msg;
        uint8_t *expected_data_ptr           = (uint8_t *)&msg_buffer[0] + sizeof(header_t);
        uint8_t *expected_data_end           = expected_data_ptr + data_size + CRC_SIZE;

        MsgAlloc_ValidHeader(valid, data_size);
        NEW_STEP("Check mem cleared flag is raised");
        TEST_ASSERT_EQUAL(true, mem_clear_needed);
        NEW_STEP("Check \"data pointer\" has been computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" has been computed");
        TEST_ASSERT_EQUAL(expected_data_end, data_end_estimation);
        NEW_STEP("Check \"copy task pointer\" points to message to copy");
        TEST_ASSERT_EQUAL(expected_copy_task_pointer, copy_task_pointer);
    }

    NEW_TEST_CASE("There is enough space : save the end position and raise the clear flag");
    MsgAlloc_Init(NULL);
    {
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |----------------------|  Header  |---------------------------|
        //        +----------------------^----------^---------------------------+
        //                               |          |
        //                         current_msg     data_ptr
        //
        //
        //        msg_buffer ending state : MEM_CLEAR_NEEDED = True
        //        +-------------------------------------------------------------+
        //        |----------------------|  Header  | Datas to be received |----|
        //        +----------------------^----------^----------------------^----+
        //                               |          |                      |
        //                         current_msg     data_ptr         data_end_estimation
        //

        uint8_t *expected_data_end;
        uint8_t valid      = true;
        uint16_t data_size = DATA_SIZE;

        current_msg       = (msg_t *)&msg_buffer[0];
        expected_data_end = (uint8_t *)current_msg + sizeof(header_t) + data_size + CRC_SIZE;

        MsgAlloc_ValidHeader(valid, data_size);
        NEW_STEP("Check mem cleared flag is raised");
        TEST_ASSERT_EQUAL(true, mem_clear_needed);
        NEW_STEP("Check \"data end estimation\" has been computed");
        TEST_ASSERT_EQUAL(expected_data_end, data_end_estimation);
    }
}

void unittest_MsgAlloc_InvalidMsg()
{
    NEW_TEST_CASE("Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
#define DATA_END_LIMIT (MSG_BUFFER_SIZE - sizeof(header_t) - CRC_SIZE)

        //  current_msg is in limit position : no left memory for a message : ASSERT
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |---------------------------------------------|   Message     |
        //        +---------------------------------------------^---------------^
        //                                                      |               |
        //                                                   current_msg     data_ptr
        //

        NEW_STEP("Forbidden values are injected -> Verify function is asserting");
        for (uint16_t i = DATA_END_LIMIT; i < MSG_BUFFER_SIZE; i++)
        {
            // Init variables
            //---------------
            current_msg = (msg_t *)&msg_buffer[i];
            RESET_ASSERT();

            // Call function
            //---------------
            MsgAlloc_InvalidMsg();

            // Verify
            //---------------
            TEST_ASSERT_TRUE(IS_ASSERT());
        }
        RESET_ASSERT();

        //  current_msg is OK : enough memory for a message : NO ASSERT
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |----------------------------------------|   Message     |----|
        //        +----------------------------------------^---------------^----+
        //                                                 |               |
        //                                              current_msg     data_ptr
        //

        NEW_STEP("Correct values are injected   -> Verify function is NOT asserting");
        for (uint16_t i = 0; i < DATA_END_LIMIT; i++)
        {
            // Init variables
            //---------------
            current_msg = (msg_t *)&msg_buffer[i];
            RESET_ASSERT();

            // Call function
            //---------------
            MsgAlloc_InvalidMsg();

            // Verify
            //---------------
            TEST_ASSERT_FALSE(IS_ASSERT());
        }
    }

    NEW_TEST_CASE("Check pointers values after invaliding a message");
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
        NEW_STEP("Check message is invalid: \"data pointer\" is reseted to \"current message\"");
        TEST_ASSERT_EQUAL(current_msg, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(data_end_estimation, ((uint8_t *)current_msg + DATA_END));
        NEW_STEP("Check no copy is needed : \"copy task pointer\" is reseted");
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
        NEW_STEP("Check message is invalid: \"data pointer\" is reseted to \"current message\"");
        TEST_ASSERT_EQUAL(current_msg, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(data_end_estimation, ((uint8_t *)current_msg + DATA_END));
        NEW_STEP("Check \"copy task pointer\" is not cleared");
        TEST_ASSERT_NOT_NULL(copy_task_pointer);
    }

    NEW_TEST_CASE("Clean memory");
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
        NEW_STEP("Check there is no need to clear memory");
        TEST_ASSERT_EQUAL(false, mem_clear_needed);
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            NEW_STEP_IN_LOOP("Clean message task", i);
            TEST_ASSERT_EQUAL(expected_msg_tasks[i], msg_tasks[i]);
        }
    }
}

void unittest_MsgAlloc_EndMsg()
{
    NEW_TEST_CASE("Verify assertion cases");
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
            RESET_ASSERT();
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
            NEW_STEP_IN_LOOP("Verify function is asserting when forbidden values are injected", i);
            TEST_ASSERT_EQUAL(assert_sc[i].expected_asserts, IS_ASSERT());
        }
        RESET_ASSERT();
    }
    RESET_ASSERT();

    NEW_TEST_CASE("Prepare the next message : update pointers");
    MsgAlloc_Init(NULL);
    {
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        | Header | Data | CRC |---------------------------------------+
        //        +---------------------^---------------------------------------+
        //                              |
        //                           data_ptr
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        // OLD    | Header | Data | CRC |---------------------------------------+
        // FUTURE |---------------| Header |------------------------------------+
        //        +---------------^--------^------------------------------------+
        //                        |        |
        //                     data_ptr   data_end_estimation
        //                    current_msg
        //

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
        RESET_ASSERT();
        MsgAlloc_EndMsg();

        // Verify
        //---------------
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
        NEW_STEP("Check \"data pointer\" alignment");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(data_ptr, current_msg);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);
        NEW_STEP("Check mem cleared flag is raised");
        TEST_ASSERT_EQUAL(true, mem_clear_needed);
        NEW_STEP("Check \"message tasks stack id\" is incremented to 1");
        TEST_ASSERT_EQUAL(1, msg_tasks_stack_id);
    }

    NEW_TEST_CASE("Data are not aligned");
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
        NEW_STEP("Check \"data pointer\" alignment");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
    }

    NEW_TEST_CASE("Verify task allocation");
    MsgAlloc_Init(NULL);
    {
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        | Header | Data | CRC |---------------------------------------+
        //        +---------------------^---------------------------------------+
        //                              |
        //                           data_ptr
        //
        //        msg_buffer ending state after calling "MsgAlloc_EndMsg"
        //        +-------------------------------------------------------------+
        // OLD    | Header | Data | CRC |---------------------------------------+
        //        ^-------------------------------------------------------------+
        //        |
        //    oldest_message
        //     msg_tasks[0]

        MsgAlloc_EndMsg();
        NEW_STEP("Check first \"message task\" points to beginning of \"message buffer\"");
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[0], msg_tasks[0]);
        NEW_STEP("Check \"oldest message\" points to beginning of \"message buffer\"");
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[0], oldest_msg);
    }

    NEW_TEST_CASE("Not enough space for the next message");
    MsgAlloc_Init(NULL);
    {
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |---------------------------------------------| Header | Data | CRC |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |---------------------------------------------| Header | Data | CRC |
        //        ^-------------------------------------------------------------+
        //        |
        //    data_ptr
        //    current_mag

        // Init variables
        //---------------
        data_ptr = &msg_buffer[MSG_BUFFER_SIZE - 1];

        // Call function
        //---------------
        MsgAlloc_EndMsg();

        // Verify
        //---------------
        NEW_STEP("Check \"data pointer\" points to beginning of \"message buffer\"");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[0], data_ptr);
        NEW_STEP("Check \"current message\" points to beginning of \"message buffer\"");
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[0], current_msg);
    }

    NEW_TEST_CASE("Remove oldest msg if message task buffer is full");
    MsgAlloc_Init(NULL);
    {
        // msg_tasks_stack_id = MAX_MSG_NB => oldest message must be deleted
        //
        //          msg_tasks init state                           msg_tasks end state
        //
        //             +---------+                                    +---------+
        //             |   D 1   |                                    |   D 2   |<--Oldest message "D 1" is deleted
        //             |---------|                                    |---------|
        //             |   D 2   |                                    |   D 3   |
        //             |---------|                                    |---------|
        //             |  etc... |                                    |  etc... |
        //             |---------|                                    |---------|
        //             |  D10    |                                    |   NEW   |
        //             +---------+<--luos_tasks_stack_id              +---------+<--luos_tasks_stack_id
        //

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
        expected_msg_tasks[MAX_MSG_NB - 1] = (msg_t *)(current_msg);

        // Last Msg Task must be cleared
        msg_tasks_stack_id = MAX_MSG_NB;

        // Call function
        //---------------
        MsgAlloc_EndMsg();

        // Verify
        //---------------
        NEW_STEP_IN_LOOP("Check oldest message task is cleared", 0);
        TEST_ASSERT_EQUAL(expected_msg_tasks[0], msg_tasks[0]);

        for (uint16_t i = 1; i < MAX_MSG_NB - 1; i++)
        {
            NEW_STEP_IN_LOOP("Check message task", i);
            TEST_ASSERT_EQUAL(expected_msg_tasks[i], msg_tasks[i]);
        }

        NEW_STEP_IN_LOOP("Check last message task is filled with current message", MAX_MSG_NB - 1);
        TEST_ASSERT_EQUAL(expected_msg_tasks[MAX_MSG_NB - 1], msg_tasks[MAX_MSG_NB - 1]);
    }
}

#define COPY_LENGTH 128
void unittest_MsgAlloc_SetData()
{
    NEW_TEST_CASE("Set Data");
    MsgAlloc_Init(NULL);
    {
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------------------------------------------------+
        //        |
        //      data_ptr
        //
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |0 1 2 3 ....... (COPY_LENGTH -1) ----------------------------|
        //        -------------------------^------------------------------------+
        //                                 |
        //                         data_ptr
        //

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
        NEW_STEP("Check message buffer is correctly filled");
        TEST_ASSERT_EQUAL_MEMORY(expected_datas, (uint8_t *)&msg_buffer[0], COPY_LENGTH);
    }
}

void unittest_MsgAlloc_IsEmpty()
{
    NEW_TEST_CASE("Is Empty");
    MsgAlloc_Init(NULL);
    {
        //
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        ^-------------------------------------------------------------+
        //        |
        //     data_ptr  <= return SUCCED only if data_ptr is here (empty buffer)
        //

        data_ptr = &msg_buffer[0];
        NEW_STEP("Check NO message has been received since initialization");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_IsEmpty());

        NEW_STEP("Check message(s) has been received since initialization");
        for (uint16_t i = 1; i < MSG_BUFFER_SIZE; i++)
        {
            data_ptr = &msg_buffer[i];
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_IsEmpty());
        }
    }
}

void unittest_MsgAlloc_UsedMsgEnd()
{
    NEW_TEST_CASE("Used Message End");
    MsgAlloc_Init(NULL);
    {
        used_msg = (msg_t *)&msg_buffer[0];
        MsgAlloc_UsedMsgEnd();

        NEW_STEP("Check \"used message\" is reseted");
        TEST_ASSERT_NULL(used_msg);
    }
}

void unittest_MsgAlloc_PullMsgToInterpret()
{
    NEW_TEST_CASE("Case FAILED");
    MsgAlloc_Init(NULL);
    {
        //
        //             msg_tasks
        //             +---------+<--msg_tasks_stack_id = 0 ==> Assert
        //             |   A 1   |
        //             |---------|
        //             |   A 2   |
        //             |---------|
        //             |   A 3   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //

        msg_t *returned_msg = NULL;
        msg_tasks_stack_id  = 0;

        NEW_STEP("Check function returns FAILED when \"message tasks stack id\" = 0 ");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_PullMsgToInterpret(&returned_msg));
        NEW_STEP("Check RETURNED MESSAGE is not filled");
        TEST_ASSERT_NULL(returned_msg);
    }

    NEW_TEST_CASE("Case SUCCEED");
    MsgAlloc_Init(NULL);
    {
        //
        //         msg_tasks init state              msg_tasks end state
        //             +---------+                      +---------+
        //             |  Msg 1  |                      |  Msg 2  |<--"returned_msg" points to 1st message of msg_tasks
        //             |---------|                      |---------|
        //             |  Msg 2  |                      |  Msg 3  |
        //             |---------|                      |---------|
        //             |  etc... |                      |  etc... |
        //             |---------|                      |---------|<--msg_tasks_stack_id
        //             |  LAST   |                      |    0    |
        //             +---------+<--msg_tasks_stack_id +---------+
        //

        msg_t *expected_message[MAX_MSG_NB];
        msg_t *returned_msg;
        error_return_t result;

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            expected_message[i] = (msg_t *)&msg_buffer[i];
            msg_tasks[i]        = (msg_t *)&msg_buffer[i];
        }

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            msg_tasks_stack_id = MAX_MSG_NB;
            RESET_ASSERT();
            result = MsgAlloc_PullMsgToInterpret(&returned_msg);
            NEW_STEP_IN_LOOP("Check NO assert has occured", i);
            TEST_ASSERT_FALSE(IS_ASSERT());
            NEW_STEP_IN_LOOP("Check function returns SUCCEED", i);
            TEST_ASSERT_EQUAL(SUCCEED, result);
            NEW_STEP_IN_LOOP("Check function fills RETURNED MESSAGE pointer", i);
            TEST_ASSERT_EQUAL(expected_message[i], returned_msg);
        }
    }
}

void unittest_MsgAlloc_LuosTaskAlloc()
{
    NEW_TEST_CASE("No more space in luos_tasks");
    MsgAlloc_Init(NULL);
    {
        //
        //         luos_tasks init state                 luos_tasks end state
        //             +---------+                          +---------+
        //             |   D 1   |                          |   D 2   |<--"D 1" is removed
        //             |---------|                          |---------|
        //             |   D 2   |                          |   D 3   |
        //             |---------|                          |---------|
        //             |  etc... |                          |  etc... |
        //             |---------|                          |---------|
        //             |  Last   |                          |    0    |
        //             +---------+<--luos_tasks_stack_id    +---------+
        //

        luos_task_t expected_luos_task;
        ll_service_t service;

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
        MsgAlloc_LuosTaskAlloc(&service, (msg_t *)&msg_buffer[0]);

        // Verify
        NEW_STEP("Check Luos stack occupation is 100\%");
        TEST_ASSERT_EQUAL(100, memory_stats.luos_stack_ratio);
        NEW_STEP("Check there is 1 dropped message");
        TEST_ASSERT_EQUAL(1, memory_stats.msg_drop_number);
        NEW_STEP("Check Oldest Message is removed");
        TEST_ASSERT_EQUAL(&msg_buffer[1], luos_tasks[0].msg_pt);
    }

    NEW_TEST_CASE("Allocation");
    MsgAlloc_Init(NULL);
    {
        //
        //         luos_tasks init state                           luos_tasks end state
        //
        //             +---------+<--luos_tasks_stack_id              +---------+
        //             |    0    |                                    |   D 1   | (msg_pt & ll_service_pt are allocated)
        //             |---------|                                    |---------|
        //             |    0    |                                    |   D 2   | (msg_pt & ll_service_pt are allocated)
        //             |---------|                                    |---------|
        //             |  etc... |                                    |  etc... |
        //             |---------|                                    |---------|
        //             |    0    |                                    |  Last   | (msg_pt & ll_service_pt are allocated)
        //             +---------+              luos_tasks_stack_id-->+---------+
        //

        msg_t *message;
        ll_service_t *service_concerned;
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
            service_concerned            = (ll_service_t *)&msg_buffer[0];

            // Launch Test
            MsgAlloc_LuosTaskAlloc(service_concerned, message);

            // Verify
            NEW_STEP_IN_LOOP("Check message pointer is allocated", i);
            TEST_ASSERT_EQUAL(message, luos_tasks[i].msg_pt);
            NEW_STEP_IN_LOOP("Check service pointer is allocated", i);
            TEST_ASSERT_EQUAL(service_concerned, luos_tasks[i].ll_service_pt);
            NEW_STEP_IN_LOOP("Check \"luos tasks stack id\" is updated", i);
            TEST_ASSERT_EQUAL(expected_luos_tasks_stack_id, luos_tasks_stack_id);
            NEW_STEP_IN_LOOP("Check \"oldest message\" points to first luos task", i);
            TEST_ASSERT_EQUAL(luos_tasks[0].msg_pt, oldest_msg);
            NEW_STEP_IN_LOOP("Check luos stack ratio computation", i);
            TEST_ASSERT_EQUAL(expected_mem_stat, memory_stats.luos_stack_ratio);
        }
    }
}

void unittest_MsgAlloc_LuosTasksNbr(void)
{
    NEW_TEST_CASE("Verify getter for Luos Tasks number");
    MsgAlloc_Init(NULL);
    {
        // Init variables
        //---------------
        luos_tasks_stack_id = 0;

        // Call function and Verify
        //---------------------------
        for (uint8_t i = 0; i < MAX_MSG_NB; i++)
        {
            NEW_STEP_IN_LOOP("Check Luos task number", i);
            MsgAlloc_LuosTasksNbr();
            TEST_ASSERT_EQUAL(luos_tasks_stack_id, i);
            luos_tasks_stack_id++;
        }
    }
}

void unittest_MsgAlloc_PullMsg()
{
    NEW_TEST_CASE("Case FAILED : no message for the service");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |<--luos_tasks_stack_id
        //             |---------|
        //             |   D 3   |\
        //             |---------| |
        //             |  etc... | |  <-- search these IDs
        //             |---------| |  (function return FAILED if ID > luos_tasks_stack_id)
        //             |  Last   | |
        //             +---------+/
        //

        msg_t *returned_message;
        ll_service_t *service = (ll_service_t *)0xFFFF;

        // Init variables
        luos_tasks_stack_id = MAX_MSG_NB - 1;
        for (uint32_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].ll_service_pt = (ll_service_t *)i;
        }

        // Launch Test & Verify
        NEW_STEP("Function returns FAILED when wanted service doesn't exists");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_PullMsg(service, &returned_message));
    }

    NEW_TEST_CASE("Case SUCCEED : return oldest message for the service");
    MsgAlloc_Init(NULL);
    {
        //
        //   Pull a message from a specific service (for example service is in task D3)
        //   luos_tasks_stack_id = 3 : function will search in messages D1, D2 & D3
        //
        //
        //        msg_buffer                                 msg_buffer after pull
        //        +------------------------+                +------------------------+
        //        |------------------------|                |------------------------|
        //        +--^---^---^-------------+                +--^---^---^-------------+
        //           |   |   |                                 |   |   |
        //          D1  D2   D3    ...                        D1  D2  used_msg   ...
        //                                                            returned_msg
        //
        //
        //             luos_tasks                                luos_tasks
        //             +---------+                               +---------+
        //             |   D 1   |\                              |   D 1   |
        //             |---------| |                             |---------|
        //             |   D 2   | |                             |   D 2   |
        //             |---------| |                             |---------|
        //             |   D 3   |/                              |    0    |<-- messaged pulled is cleared
        //             |---------|<--luos_tasks_stack_id         |---------|
        //             |  etc... |                               |  etc... |
        //             |---------|                               |---------|
        //             |   LAST  |                               |   LAST  |
        //             +---------+                               +---------+
        //

        msg_t *returned_message;
        msg_t *msg_to_clear;
        ll_service_t *service;
        luos_task_t expected_luos_tasks[MAX_MSG_NB];

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {

            // Init variables
            MsgAlloc_Init(NULL);
            luos_tasks_stack_id = MAX_MSG_NB;
            for (uint16_t j = 0; j < MAX_MSG_NB; j++)
            {
                luos_tasks[j].ll_service_pt   = (ll_service_t *)&msg_buffer[j];
                luos_tasks[j].msg_pt          = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
                expected_luos_tasks[j].msg_pt = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
            }
            service      = luos_tasks[i].ll_service_pt;
            msg_to_clear = luos_tasks[i].msg_pt;

            // Launch Test & Verify
            NEW_STEP_IN_LOOP("Check function returns SUCCEED", i);
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_PullMsg(service, &returned_message));
            NEW_STEP_IN_LOOP("Check message pointer is allocated", i);
            TEST_ASSERT_EQUAL(expected_luos_tasks[i].msg_pt, returned_message);
            NEW_STEP_IN_LOOP("Check \"used message\" is updated", i);
            TEST_ASSERT_EQUAL(returned_message, used_msg);
            NEW_STEP_IN_LOOP("Check luos task is cleared for required message", i);
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
    NEW_TEST_CASE("Case FAILED : no message for ID");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |<--luos_tasks_stack_id
        //             |---------|
        //             |   D 3   |\
        //             |---------| |
        //             |  etc... | |  <-- search these IDs
        //             |---------| |  (function return FAILED if ID > luos_tasks_stack_id)
        //             |  Last   | |
        //             +---------+/
        //

        msg_t *returned_msg = NULL;
        luos_tasks_stack_id = 5;

        for (uint16_t task_id = luos_tasks_stack_id; task_id < luos_tasks_stack_id + 5; task_id++)
        {
            NEW_STEP_IN_LOOP("Check function returns FAILED when \"luos tasks stack id\" points to a void message", task_id);
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_PullMsgFromLuosTask(task_id, &returned_msg));
        }
    }

    NEW_TEST_CASE("Case SUCCEED : pull all messages from Luos Tasks");
    MsgAlloc_Init(NULL);
    {
        //
        //        msg_buffer                        msg_buffer after 1st pull         msg_buffer after 2nd pull
        //        +------------------------+        +------------------------+        +------------------------+
        //        |------------------------|        |------------------------|        |------------------------|        etc...
        //        +--^-------^-------^-----+        +--^-------^-------^-----+        +----------^-------^-----+
        //           |       |       |                 |       |       |                         |       |
        //          D1       D2  ... LAST             D1       D2  ... LAST                      D2  ... LAST
        //                                         used_msg                                   used_msg
        //                                       returned_msg                               returned_msg
        //
        //             luos_tasks                       luos_tasks                       luos_tasks
        //             +---------+                      +---------+                      +---------+
        //             |   D 1   |                      |   D 2   |                      |   D 3   |
        //             |---------|                      |---------|                      |---------|
        //             |   D 2   |                      |   D 3   |                      |   D 4   |
        //             |---------|                      |---------|                      |---------|
        //             |   D 3   |                      |   D 4   |                      |   D 5   |
        //             |---------|                      |---------|                      |---------|
        //             |  etc... |                      |  etc... |                      |  etc... |
        //             |---------|                      |---------|                      |---------|
        //             |   LAST  |                      |    0    |                      |    0    |
        //             +---------+                      +---------+                      +---------+

        uint16_t task_id;
        msg_t *returned_message;
        msg_t *msg_to_clear;
        ll_service_t *service;
        luos_task_t expected_luos_tasks[MAX_MSG_NB];

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
            NEW_STEP_IN_LOOP("Check function returns SUCCEED", task_id);
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_PullMsgFromLuosTask(task_id, &returned_message));
            NEW_STEP_IN_LOOP("Check luos task message pointer is allocated", task_id);
            TEST_ASSERT_EQUAL(expected_luos_tasks[task_id].msg_pt, returned_message);
            NEW_STEP_IN_LOOP("Check \"used message\" is updated", task_id);
            TEST_ASSERT_EQUAL(returned_message, used_msg);
            NEW_STEP_IN_LOOP("Check luos task is cleared for required message", task_id);
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
    NEW_TEST_CASE("Case FAILED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |<--luos_tasks_stack_id
        //             |---------|
        //             |   D 3   |\
        //             |---------| |
        //             |  etc... | |  <-- search these IDs
        //             |---------| |  (function return FAILED if ID > luos_tasks_stack_id)
        //             |  Last   | |
        //             +---------+/
        //

        uint16_t task_id;
        ll_service_t **allocated_service;

        // Init variables
        luos_tasks_stack_id = 0;
        task_id             = luos_tasks_stack_id;

        // Call function & Verify
        NEW_STEP("Check function returns FAILED when \"luos tasks stack id\" points to a void message");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_LookAtLuosTask(task_id, allocated_service));
        task_id++;
        NEW_STEP("Check function returns FAILED when \"luos tasks stack id\" points to another void message");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_LookAtLuosTask(task_id, allocated_service));
    }

    NEW_TEST_CASE("Case SUCCEED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |<-- search this ID  (function will fill service pointer associated to D 2 Luos Task)
        //             |---------|
        //             |   D 3   |<--luos_tasks_stack_id
        //             |---------|
        //             |    0    |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |    0    |
        //             +---------+
        //

        // Init variables
        ll_service_t *oldest_ll_service = NULL;
        luos_tasks_stack_id             = MAX_MSG_NB;

        for (uint32_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].ll_service_pt = (ll_service_t *)i;
        }

        // Call function & Verify
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            NEW_STEP_IN_LOOP("Check function returns SUCCEED", i);
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_LookAtLuosTask(i, &oldest_ll_service));
            NEW_STEP_IN_LOOP("Check if function return the service concerned by the oldest message", i);
            TEST_ASSERT_EQUAL(i, oldest_ll_service);
        }
    }
}

void unittest_MsgAlloc_GetLuosTaskSourceId()
{
    NEW_TEST_CASE("Case FAILED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |<--luos_tasks_stack_id
        //             |---------|
        //             |   D 3   |\
        //             |---------| |
        //             |  etc... | |  <-- search these IDs
        //             |---------| |  (function return FAILED if ID > luos_tasks_stack_id)
        //             |  Last   | |
        //             +---------+/
        //

        uint16_t task_id;
        uint16_t task_id_2;

        // Init variables
        luos_tasks_stack_id = MAX_MSG_NB;
        task_id             = luos_tasks_stack_id;
        task_id_2           = luos_tasks_stack_id + 1;

        // Call function & Verify
        NEW_STEP("Check function returns FAILED when required task ID points to a void message");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskSourceId(task_id, NULL));
        NEW_STEP("Check function returns FAILED when required task ID points to another void message");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskSourceId(task_id_2, NULL));
    }

    NEW_TEST_CASE("Case SUCCEED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|<--luos_tasks_stack_id : fills SOURCE header pointer
        //             |   D 2   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //

        // Init variables
        msg_t message;
        uint16_t source_id  = 0;
        luos_tasks_stack_id = MAX_MSG_NB;

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            message.header.source = i;
            luos_tasks[i].msg_pt  = &message;

            // Call function & Verify
            NEW_STEP_IN_LOOP("Check function returns SUCCEED", i);
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetLuosTaskSourceId(i, &source_id));
            NEW_STEP_IN_LOOP("Check function fills message SOURCE ID", i);
            TEST_ASSERT_EQUAL(i, source_id);
        }
    }
}

void unittest_MsgAlloc_GetLuosTaskCmd()
{
    NEW_TEST_CASE("FAILED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |<--luos_tasks_stack_id
        //             |---------|
        //             |   D 3   |\
        //             |---------| |
        //             |  etc... | |  <-- search these IDs
        //             |---------| |  (function return FAILED if ID > luos_tasks_stack_id)
        //             |  Last   | |
        //             +---------+/
        //

        uint16_t task_id;
        uint8_t command = 0xFF;

        luos_tasks_stack_id = MAX_MSG_NB;
        task_id             = luos_tasks_stack_id;

        NEW_STEP("Check function returns FAILED when required task ID points to a void message");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskCmd(task_id, &command));
        NEW_STEP("Check COMMAND is not filled when required task ID points to a void message");
        TEST_ASSERT_EQUAL(0xFF, command);

        task_id++;
        NEW_STEP("Check function returns FAILED when required task ID points to another void message");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskCmd(task_id, &command));
        NEW_STEP("Check COMMAND is not filled when required task ID points to a void message");
        TEST_ASSERT_EQUAL(0xFF, command);
    }

    NEW_TEST_CASE("SUCCEED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|<--luos_tasks_stack_id : fills COMMAND header pointer
        //             |   D 2   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //

        msg_t message;
        uint16_t task_id         = 0;
        uint8_t command          = 0;
        uint8_t expected_command = 1;

        luos_tasks_stack_id = MAX_MSG_NB;

        message.header.cmd         = expected_command;
        luos_tasks[task_id].msg_pt = &message;

        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetLuosTaskCmd(task_id, &command));
        NEW_STEP("Check function fills message COMMAND");
        TEST_ASSERT_EQUAL(expected_command, command);
    }
}

void unittest_MsgAlloc_GetLuosTaskSize()
{
    NEW_TEST_CASE("FAILED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |<--luos_tasks_stack_id
        //             |---------|
        //             |   D 3   |\
        //             |---------| |
        //             |  etc... | |  <-- search these IDs
        //             |---------| |  (function return FAILED if ID > luos_tasks_stack_id)
        //             |  Last   | |
        //             +---------+/
        //

        uint16_t task_id;
        uint16_t size       = 0xFF;
        luos_tasks_stack_id = MAX_MSG_NB;
        task_id             = luos_tasks_stack_id;

        NEW_STEP("Check function returns FAILED when required task ID points to a void message");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetLuosTaskSize(task_id, &size));
        NEW_STEP("Check SIZE is not filled when required task ID points to a void message");
        TEST_ASSERT_EQUAL(0xFF, size);
    }

    NEW_TEST_CASE("SUCCEED");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|<--luos_tasks_stack_id : fills SIZE header pointer
        //             |   D 2   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //

        msg_t message;
        uint16_t expected_size = 128;
        uint16_t task_id       = 0;
        uint16_t size          = 0;

        luos_tasks_stack_id = MAX_MSG_NB;

        message.header.size        = expected_size;
        luos_tasks[task_id].msg_pt = &message;

        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetLuosTaskSize(task_id, &size));
        NEW_STEP("Check function fills message SIZE");
        TEST_ASSERT_EQUAL(expected_size, size);
    }
}

void unittest_MsgAlloc_ClearMsgFromLuosTasks()
{
    NEW_TEST_CASE("No message to clear");
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
        ll_service_t service;

        // Init variables
        luos_tasks_stack_id = 0;
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].msg_pt                 = &message;
            luos_tasks[i].ll_service_pt          = &service;
            expected_luos_tasks[i].msg_pt        = &message;
            expected_luos_tasks[i].ll_service_pt = &service;
            // Call function
            MsgAlloc_ClearMsgFromLuosTasks(luos_tasks[i].msg_pt);

            NEW_STEP_IN_LOOP("Check luos message pointer is not cleared", i);
            TEST_ASSERT_EQUAL(expected_luos_tasks[i].msg_pt, luos_tasks[i].msg_pt);
            NEW_STEP_IN_LOOP("Check luos service pointer is not cleared", i);
            TEST_ASSERT_EQUAL(expected_luos_tasks[i].ll_service_pt, luos_tasks[i].ll_service_pt);
        }
    }

    NEW_TEST_CASE("Clear a specific Luos Task");
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
        //                   Luos_tasks ending state
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
                NEW_STEP_IN_LOOP("Check expected message is cleared for all cases", (MAX_MSG_NB * i) + k);
                TEST_ASSERT_NOT_EQUAL(msg_to_clear, luos_tasks[k].msg_pt);
            }
        }
    }
}

void unittest_MsgAlloc_PullMsgFromTxTask()
{
    NEW_TEST_CASE("Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
        //              tx_tasks
        //             +---------+<--tx_tasks_stack_id = 0 ==> Assert
        //             |   Tx1   |
        //             |---------|
        //             |   Tx2   |
        //             |---------|
        //             |   Tx3   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |  LAST   |
        //             +---------+
        //                        <--tx_tasks_stack_id overflows ==> Assert

        for (uint16_t i = 0; i < MAX_MSG_NB + 2; i++)
        {
            MsgAlloc_Init(NULL);
            RESET_ASSERT();
            tx_tasks_stack_id = i;

            if (tx_tasks_stack_id == 0)
            {
                MsgAlloc_PullMsgFromTxTask();
                NEW_STEP_IN_LOOP("Check assert has occured when \"tx tasks stack id\" = 0", i);
                TEST_ASSERT_TRUE(IS_ASSERT());
            }
            else if (tx_tasks_stack_id > MAX_MSG_NB)
            {
                MsgAlloc_PullMsgFromTxTask();
                NEW_STEP_IN_LOOP("Check assert has occured when \"tx tasks stack id\" overflows", i);
                TEST_ASSERT_TRUE(IS_ASSERT());
            }
            else
            {
                MsgAlloc_PullMsgFromTxTask();
                NEW_STEP_IN_LOOP("Check NO assert has occured", i);
                TEST_ASSERT_FALSE(IS_ASSERT());
            }
        }
        RESET_ASSERT();
    }

    NEW_TEST_CASE("Pull Tx message task");
    MsgAlloc_Init(NULL);
    {
        //
        //
        //                      tx_tasks                         tx_tasks                         tx_tasks
        //                     +---------+                      +---------+                      +---------+
        //                     |   Tx1   |                      |   Tx2   |                      |   Tx3   |
        //                     |---------|                      |---------|                      |---------|
        //                     |   Tx2   |                      |   Tx3   |                      |   Tx4   |
        //                     |---------|                      |---------|                      |---------|
        //                     |   Tx3   |                      |   Tx4   |                      |   Tx5   |
        //                     |---------|                      |---------|                      |---------|
        //                     |  etc... |                      |  etc... |                      |  etc... |        etc...
        //                     |---------|                      |---------|  tx_tasks_stack_id-->|---------|
        //                     |  etc... |                      |  etc... |                      |    0    |
        //                     |---------|  tx_tasks_stack_id-->|---------|                      |---------|
        //                     |  LAST   |                      |    0    |                      |    0    |
        // tx_tasks_stack_id-->+---------+                      +---------+                      +---------+

        tx_task_t expected_tx_tasks[MAX_MSG_NB];

        NEW_STEP("Check Tx task data pointer are correctly allocated after pulling oldest tx task");
        NEW_STEP("Check Tx task size is correctly allocated after pulling oldest tx task");
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

void unittest_MsgAlloc_PullServiceFromTxTask()
{
    NEW_TEST_CASE("Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
        //
        //             tx_tasks
        //             +---------+<--tx_tasks_stack_id = 0 ==> Assert
        //             |   Tx1   |
        //             |---------|
        //             |   Tx2   |
        //             |---------|
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //                        <--tx_tasks_stack_id overflows ==> Assert
        //
        uint16_t service_id = 1;

        for (uint16_t i = 0; i <= MAX_MSG_NB; i++)
        {
            MsgAlloc_Init(NULL);
            RESET_ASSERT();

            // Init variables
            //---------------
            tx_tasks_stack_id = i;

            for (uint16_t i = 0; i < MAX_MSG_NB; i++)
            {
                msg_tasks[i]                = (msg_t *)&msg_buffer[i * 20];
                msg_tasks[i]->header.target = 0;
                tx_tasks[i].data_pt         = (uint8_t *)msg_tasks[i];
            }

            // Call function and Verify
            //---------------------------
            if (tx_tasks_stack_id == 0)
            {
                NEW_STEP("Check assert has occured when \"tx tasks stack id\" = 0");
                MsgAlloc_PullServiceFromTxTask(service_id);
                TEST_ASSERT_TRUE(IS_ASSERT());
            }
            else
            {
                NEW_STEP_IN_LOOP("Check correct cases", i - 1);
                MsgAlloc_PullServiceFromTxTask(service_id);
                TEST_ASSERT_FALSE(IS_ASSERT());
            }
        }
        RESET_ASSERT();
    }

    NEW_TEST_CASE("Remove Tx tasks from a service");
    MsgAlloc_Init(NULL);
    {
        //
        //   Remove a Tx message from a specific service by analizing "target" in header (for example service is in tx task Tx2)
        //   tx_tasks_stack_id = 3 : function will search in messages Tx1, Tx2 & Tx3
        //
        //             tx_tasks                                  tx_tasks
        //             +---------+                               +---------+
        //             |   Tx1   |\                              |   Tx1   |
        //             |---------| |                             |---------|
        //             |   Tx2   | |                             |   Tx3   |<-- messaged Tx2 has been is cleared
        //             |---------| |                             |---------|
        //             |   Tx3   |/                              |   Tx4   |
        //             |---------|<--tx_tasks_stack_id           |---------|
        //             |  etc... |                               |  etc... |
        //             |---------|                               |---------|
        //             |   LAST  |                               |    0    |
        //             +---------+                               +---------+
        //

        tx_task_t expected_tx_tasks[MAX_MSG_NB];
        uint16_t service_id;

        // Init variables
        //---------------
        tx_tasks_stack_id = 10;

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            msg_tasks[i]                = (msg_t *)&msg_buffer[i * 20];
            msg_tasks[i]->header.target = i + 1;
            tx_tasks[i].data_pt         = (uint8_t *)msg_tasks[i];
        }

        // Remove all messages from service 15 (position 0,2,4 in buffer)
        service_id                  = 15;
        msg_tasks[0]->header.target = service_id;
        msg_tasks[2]->header.target = service_id;
        msg_tasks[4]->header.target = service_id;

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
        MsgAlloc_PullServiceFromTxTask(service_id);

        // Verify
        //---------------
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            NEW_STEP_IN_LOOP("Check Tx task message pointer are correctly allocated after pulling expected service tx task", i);
            TEST_ASSERT_EQUAL(expected_tx_tasks[i].data_pt, tx_tasks[i].data_pt);
        }
    }
}

void unittest_MsgAlloc_GetTxTask()
{
    NEW_TEST_CASE("Verify assertion cases");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |
        //             |---------|
        //             |    0    |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //                        <--luos_tasks_stack_id (overflows ==> Assert)
        //

        for (uint8_t i = 0; i < 2; i++)
        {
            RESET_ASSERT();

            // Init variables
            //---------------
            ll_service_t *ll_service;
            uint8_t *data;
            uint16_t size     = 128;
            uint8_t localhost = 1;

            tx_tasks[0].data_pt       = (uint8_t *)16;
            tx_tasks[0].ll_service_pt = (ll_service_t *)32;
            tx_tasks[0].size          = 128;
            tx_tasks[0].localhost     = 1;

            tx_tasks_stack_id = MAX_MSG_NB + i;

            // Call function
            //---------------
            MsgAlloc_GetTxTask(&ll_service, &data, &size, &localhost);

            // Verify
            //---------------
            NEW_STEP("Check assert has occured when \"tx tasks stack id\" = max value");
            TEST_ASSERT_TRUE(IS_ASSERT());

            // Call function
            //---------------
            MsgAlloc_GetTxTask(&ll_service, &data, &size, &localhost);

            // Verify
            //---------------
            NEW_STEP("Check assert has occured when \"tx tasks stack id\" overflows");
            TEST_ASSERT_TRUE(IS_ASSERT());
        }
        RESET_ASSERT();
    }

    NEW_TEST_CASE("Verify there are no messages");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+<--luos_tasks_stack_id  (no message, function return FAILED)
        //             |   D 1   |
        //             |---------|
        //             |   D 2   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //

        ll_service_t *ll_service;
        uint8_t *data;
        uint16_t *size;
        uint8_t *localhost;

        // Init variables
        //---------------
        tx_tasks_stack_id = 0;

        // Call function & Verify
        //--------------------------
        NEW_STEP("Check function returns FAILED when tx task is empty");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetTxTask(&ll_service, &data, size, localhost));
    }

    NEW_TEST_CASE("Verify there is a message");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|<--luos_tasks_stack_id  : tx_tasks[0] is filled with pointers (service, data, size & localhost)
        //             |   D 2   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //

        ll_service_t *ll_service;
        uint8_t *data;
        uint16_t size;
        uint8_t localhost;

        // Init variables
        //---------------
        tx_tasks_stack_id = 1;

        tx_tasks[0].data_pt       = (uint8_t *)16;
        tx_tasks[0].ll_service_pt = (ll_service_t *)32;
        tx_tasks[0].size          = 128;
        tx_tasks[0].localhost     = 1;

        // Call function & Verify
        //--------------------------
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetTxTask(&ll_service, &data, &size, &localhost));
        NEW_STEP("Check function returns expected service pointer");
        TEST_ASSERT_EQUAL(tx_tasks[0].ll_service_pt, ll_service);
        NEW_STEP("Check function returns expected data");
        TEST_ASSERT_EQUAL(tx_tasks[0].data_pt, data);
        NEW_STEP("Check function returns expected size");
        TEST_ASSERT_EQUAL(tx_tasks[0].size, size);
        NEW_STEP("Check function returns expected localhost");
        TEST_ASSERT_EQUAL(tx_tasks[0].localhost, localhost);
    }
}

void unittest_MsgAlloc_TxAllComplete()
{
    NEW_TEST_CASE("Tx All Complete");
    MsgAlloc_Init(NULL);
    {
        //
        //              Tx_tasks
        //             +---------+<--tx_tasks_stack_id  = return SUCCESS (all messages have been transmitted)
        //             |  Tx 1   |\
        //             |---------| |
        //             |  Tx 1   | |
        //             |---------| |  <--tx_tasks_stack_id  = return FAILED (at least one message has not been transmitted)
        //             |  etc... | |
        //             |---------| |
        //             |  Last   | |
        //             +---------+/
        //

        tx_tasks_stack_id = 0;
        NEW_STEP("Check function returns SUCCEED when \"tx tasks stack id\" = 0 ");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_TxAllComplete());

        NEW_STEP("Check function returns FAILED when \"tx tasks stack id\" is not 0 ");
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

    ASSERT_ACTIVATION(1);

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
    //MsgAlloc_Init         => this function doesn't need unit test
    UNITY_END();
}