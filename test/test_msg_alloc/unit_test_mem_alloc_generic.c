#include "main.h"
#include "unit_test.h"
#include "msg_alloc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    msg_t *msg_pt;         /*!< Start pointer of the msg on msg_buffer. */
    service_t *service_pt; /*!< Pointer to the concerned service. */
} luos_task_t;

typedef struct
{
    uint8_t *data_pt;      /*!< Start pointer of the data on msg_buffer. */
    uint16_t size;         /*!< size of the data. */
    service_t *service_pt; /*!< Pointer to the transmitting service. */
} tx_task_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern memory_stats_t *mem_stat;
extern volatile bool reset_needed;
extern volatile uint8_t msg_buffer[MSG_BUFFER_SIZE];
extern volatile uint8_t *data_ptr;
extern volatile msg_t *oldest_msg;
extern volatile msg_t *used_msg;
extern volatile msg_t *msg_tasks[MAX_MSG_NB];
extern volatile uint16_t msg_tasks_stack_id;
extern volatile luos_task_t luos_tasks[MAX_MSG_NB];
extern volatile uint16_t luos_tasks_stack_id;
extern volatile tx_task_t tx_tasks[MAX_MSG_NB];
extern volatile uint16_t tx_tasks_stack_id;

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
                                       .engine_msg_stack_ratio  = 0,
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
        expected_buffer_occupation_ratio = 0;
        MsgAlloc_loop();

        NEW_STEP("Check buffer is empty");
        TEST_ASSERT_EQUAL(0, memory_stats.buffer_occupation_ratio);

        // Buffer occupation from 0 -> 100%
        NEW_STEP("Check buffer is full in all cases");
        oldest_msg = (msg_t *)&msg_buffer[0];
        for (uint16_t i = sizeof(header_t); i < MSG_BUFFER_SIZE; i++)
        {
            data_ptr                         = &msg_buffer[0] + i;
            expected_buffer_occupation_ratio = (i * 100) / (MSG_BUFFER_SIZE);

            MsgAlloc_loop();

            TEST_ASSERT_EQUAL(expected_buffer_occupation_ratio, memory_stats.buffer_occupation_ratio);
        }
    }
}

void unittest_MsgAlloc_Alloc() {}

#define COPY_LENGTH 128
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

        msg_tasks_stack_id = MAX_MSG_NB;
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
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
        service_t service;

        // Init variables
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .engine_msg_stack_ratio  = 0,
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
        TEST_ASSERT_EQUAL(100, memory_stats.engine_msg_stack_ratio);
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
        //             |    0    |                                    |   D 1   | (msg_pt & service_pt are allocated)
        //             |---------|                                    |---------|
        //             |    0    |                                    |   D 2   | (msg_pt & service_pt are allocated)
        //             |---------|                                    |---------|
        //             |  etc... |                                    |  etc... |
        //             |---------|                                    |---------|
        //             |    0    |                                    |  Last   | (msg_pt & service_pt are allocated)
        //             +---------+              luos_tasks_stack_id-->+---------+
        //

        msg_t *message;
        service_t *service_concerned;
        uint16_t expected_luos_tasks_stack_id;
        uint8_t expected_mem_stat;

        // Init variables
        memory_stats_t memory_stats = {.rx_msg_stack_ratio      = 0,
                                       .engine_msg_stack_ratio  = 0,
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
            expected_mem_stat            = ((i + 1) * 100 / MAX_MSG_NB);
            message                      = (msg_t *)&msg_buffer[0];
            service_concerned            = (service_t *)&msg_buffer[0];

            // Launch Test
            MsgAlloc_LuosTaskAlloc(service_concerned, message);

            // Verify
            NEW_STEP_IN_LOOP("Check message pointer is allocated", i);
            TEST_ASSERT_EQUAL(message, luos_tasks[i].msg_pt);
            NEW_STEP_IN_LOOP("Check service pointer is allocated", i);
            TEST_ASSERT_EQUAL(service_concerned, luos_tasks[i].service_pt);
            NEW_STEP_IN_LOOP("Check \"luos tasks stack id\" is updated", i);
            TEST_ASSERT_EQUAL(expected_luos_tasks_stack_id, luos_tasks_stack_id);
            NEW_STEP_IN_LOOP("Check \"oldest message\" points to first luos task", i);
            TEST_ASSERT_EQUAL(luos_tasks[0].msg_pt, oldest_msg);
            NEW_STEP_IN_LOOP("Check luos stack ratio computation", i);
            TEST_ASSERT_EQUAL(expected_mem_stat, memory_stats.engine_msg_stack_ratio);
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
        service_t *service = (service_t *)0xFFFF;

        // Init variables
        luos_tasks_stack_id = MAX_MSG_NB - 1;
        for (uintptr_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].service_pt = (service_t *)i;
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
        service_t *service;
        luos_task_t expected_luos_tasks[MAX_MSG_NB];

        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {

            // Init variables
            MsgAlloc_Init(NULL);
            luos_tasks_stack_id = MAX_MSG_NB;
            for (uint16_t j = 0; j < MAX_MSG_NB; j++)
            {
                luos_tasks[j].service_pt      = (service_t *)&msg_buffer[j];
                luos_tasks[j].msg_pt          = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
                expected_luos_tasks[j].msg_pt = (msg_t *)(&msg_buffer[0] + MAX_MSG_NB + j);
            }
            service      = luos_tasks[i].service_pt;
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
        service_t *service;
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
        service_t **allocated_service;

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
        service_t *oldest_service = NULL;
        luos_tasks_stack_id       = MAX_MSG_NB;

        for (uintptr_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].service_pt = (service_t *)i;
        }

        // Call function & Verify
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            NEW_STEP_IN_LOOP("Check function returns SUCCEED", i);
            TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_LookAtLuosTask(i, &oldest_service));
            NEW_STEP_IN_LOOP("Check if function return the service concerned by the oldest message", i);
            TEST_ASSERT_EQUAL(i, oldest_service);
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
        service_t service;

        // Init variables
        luos_tasks_stack_id = 0;
        for (uint16_t i = 0; i < MAX_MSG_NB; i++)
        {
            luos_tasks[i].msg_pt              = &message;
            luos_tasks[i].service_pt          = &service;
            expected_luos_tasks[i].msg_pt     = &message;
            expected_luos_tasks[i].service_pt = &service;
            // Call function
            MsgAlloc_ClearMsgFromLuosTasks(luos_tasks[i].msg_pt);

            NEW_STEP_IN_LOOP("Check luos message pointer is not cleared", i);
            TEST_ASSERT_EQUAL(expected_luos_tasks[i].msg_pt, luos_tasks[i].msg_pt);
            NEW_STEP_IN_LOOP("Check luos service pointer is not cleared", i);
            TEST_ASSERT_EQUAL(expected_luos_tasks[i].service_pt, luos_tasks[i].service_pt);
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
        //             +---------+
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

            if (tx_tasks_stack_id > MAX_MSG_NB)
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

    NEW_TEST_CASE("Verify when Tx task is empty");
    MsgAlloc_Init(NULL);
    {
        //              tx_tasks
        //             +---------+<--tx_tasks_stack_id = 0 ==> return
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
        //

        NEW_STEP("Check nothing happened when \"tx tasks stack id\" = 0");

        // Init
        tx_tasks_stack_id = 0;

        for (uint8_t i = 0; i < MAX_MSG_NB; i++)
        {
            tx_tasks[i].data_pt = (uint8_t *)&msg_buffer[i];
            tx_tasks[i].size    = i;
        }

        // Call function
        //---------------------------
        MsgAlloc_PullMsgFromTxTask();

        // Verify
        TEST_ASSERT_FALSE(IS_ASSERT());
        TEST_ASSERT_EQUAL(0, tx_tasks_stack_id);
        for (uint8_t i = 0; i < MAX_MSG_NB; i++)
        {
            TEST_ASSERT_EQUAL(&msg_buffer[i], tx_tasks[i].data_pt);
            TEST_ASSERT_EQUAL(i, tx_tasks[i].size);
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
        const int task_number = 10;
        tx_tasks_stack_id     = task_number;

        for (uint16_t i = 0; i < task_number; i++)
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
        for (uint16_t i = 2; i < task_number - 3; i++)
        {
            expected_tx_tasks[i].data_pt = tx_tasks[i + 3].data_pt;
        }
        expected_tx_tasks[task_number - 1].data_pt = 0;
        expected_tx_tasks[task_number - 2].data_pt = 0;
        expected_tx_tasks[task_number - 3].data_pt = 0;

        // Call function
        //---------------
        MsgAlloc_PullServiceFromTxTask(service_id);

        // Verify
        //---------------
        for (uint16_t i = 0; i < task_number; i++)
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
            service_t *service;
            uint8_t *data;
            uint16_t size = 128;

            tx_tasks[0].data_pt    = (uint8_t *)16;
            tx_tasks[0].service_pt = (service_t *)32;
            tx_tasks[0].size       = 128;

            tx_tasks_stack_id = MAX_MSG_NB + i;

            // Call function
            //---------------
            MsgAlloc_GetTxTask(&service, &data, &size);

            // Verify
            //---------------
            NEW_STEP("Check assert has occured when \"tx tasks stack id\" = max value");
            TEST_ASSERT_TRUE(IS_ASSERT());

            // Call function
            //---------------
            MsgAlloc_GetTxTask(&service, &data, &size);

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

        service_t *service;
        uint8_t *data;
        uint16_t *size;

        // Init variables
        //---------------
        tx_tasks_stack_id = 0;

        // Call function & Verify
        //--------------------------
        NEW_STEP("Check function returns FAILED when tx task is empty");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_GetTxTask(&service, &data, size));
    }

    NEW_TEST_CASE("Verify there is a message");
    MsgAlloc_Init(NULL);
    {
        //
        //             luos_tasks
        //             +---------+
        //             |   D 1   |
        //             |---------|<--luos_tasks_stack_id  : tx_tasks[0] is filled with pointers (service, data, size)
        //             |   D 2   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |   LAST  |
        //             +---------+
        //

        service_t *service;
        uint8_t *data;
        uint16_t size;

        // Init variables
        //---------------
        tx_tasks_stack_id = 1;

        tx_tasks[0].data_pt    = (uint8_t *)16;
        tx_tasks[0].service_pt = (service_t *)32;
        tx_tasks[0].size       = 128;

        // Call function & Verify
        //--------------------------
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_GetTxTask(&service, &data, &size));
        NEW_STEP("Check function returns expected service pointer");
        TEST_ASSERT_EQUAL(tx_tasks[0].service_pt, service);
        NEW_STEP("Check function returns expected data");
        TEST_ASSERT_EQUAL(tx_tasks[0].data_pt, data);
        NEW_STEP("Check function returns expected size");
        TEST_ASSERT_EQUAL(tx_tasks[0].size, size);
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
