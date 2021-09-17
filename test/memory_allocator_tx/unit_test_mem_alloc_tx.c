#include "../test/unit_test.h"
#include "../src/msg_alloc.c"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MSG_START 10
#undef VERBOSE_LOCALHOST

/*******************************************************************************
 * Function
 ******************************************************************************/
void unittest_SetTxTask_buffer_full()
{
    //**************************************************************
    NEW_TEST_CASE("Check if TX message buffer stack is full");
    MsgAlloc_Init(NULL);
    {
        // tx_tasks_stack_id = MAX_MSG_NB
        //
        //          tx_tasks init state
        //
        //             +---------+
        //             |  Tx 1   |
        //             |---------|
        //             |  Tx 2   |
        //             |---------|
        //             |  etc... |
        //             |---------|
        //             |  Tx 10  |
        //             +---------+<--tx_tasks_stack_id

        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Init variables
        //---------------
        uint8_t dummy_data = 1; // To avoid assert
        tx_tasks_stack_id  = MAX_MSG_NB - 1;

        // Call function and Verify
        //---------------------------
        for (uint16_t i = MAX_MSG_NB - 1; i < MAX_MSG_NB + 2; i++)
        {
            RESET_ASSERT();
            NEW_STEP_IN_LOOP("Function returns FAILED when \"tx tasks stack id\" overflows", i - (MAX_MSG_NB - 1));
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, &dummy_data, crc, size, localhost, ack));
            NEW_STEP_IN_LOOP("Check NO assert has occured", i - (MAX_MSG_NB - 1));
            TEST_ASSERT_FALSE(IS_ASSERT());
        }
    }
}

void unittest_SetTxTask_Tx_too_long_1()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in msg buffer\n"
                  "Tx size > Rx size received \n"
                  "There is already a task in memory\n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size is greater than Rx size currently received.
        // Tx message doesn't fit in msg buffer.
        // There is already a Task , function should return FAILED
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|Rx|--------|Task|------|
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|        Tx                   |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|Rx|--------|Task|------|
        //        +-------------------------------------------------------------+
        //                                                          |
        //                                                          FAILED : there is a task

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];        // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)];    // There are only 25 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;               // Rx message already received = 15 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                         // Rx message = 20 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - sizeof(header_t)]; // There is a task
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED when there is already a task in memory");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }

    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in msg buffer\n"
                  "Rx size received > Tx size\n"
                  "There is already a task in memory\n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Rx size currently received size is greater than Tx message.
        // Tx message doesn't fit in msg buffer.
        // There is already a Task , function should return FAILED
        //
        //        msg_buffer init state (impossible state)
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------|Task|------|
        //        |-------------------------------------|Rx                             |
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|        Tx                |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------|Task|------|
        //        +-------------------------------------------------------------+
        //                                                          |
        //                                                          FAILED : there is a task

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 80;
        uint16_t rx_bytes_received = 51;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];        // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)];    // There are only 25 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;               // Rx message already received = 51 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                         // Rx message = 80 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - sizeof(header_t)]; // There is a task
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED when there is already a task in memory");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }

    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in msg buffer\n"
                  "Rx size received = Tx size\n"
                  "There is already a task in memory\n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size = Rx size currently received.
        // Tx message doesn't fit in msg buffer.
        // There is already a Task , function should return FAILED
        //
        //        msg_buffer init state (impossible state)
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------|Task|------|
        //        |-------------------------------------|        Rx                  |
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|        Tx                  |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------|Task|------|
        //        +-------------------------------------------------------------+
        //                                                          |
        //                                                          FAILED : there is a task

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 80;
        uint16_t rx_bytes_received = tx_size;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];        // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)];    // There are only 25 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;               // Rx message already received = 50 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                         // Rx message = 50 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - sizeof(header_t)]; // There is a task
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED when there is already a task in memory");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }
}

void unittest_SetTxTask_Tx_too_long_2()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in msg buffer\n"
                  "Tx size > Rx size received\n"
                  "There is already a task at begin of message buffer\n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size is greater than Rx size currently received.
        // Tx message doesn't fit in msg buffer.
        // There is already a Task at beginning, function should return FAILED
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-----|Task|-------------------------------|Rx|---------------|
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|        Tx                   |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-----|Task|-------------------------------|Rx|---------------|
        //        +-------------------------------------------------------------+
        //              |
        //              FAILED : there is a task

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];     // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)]; // There are only 25 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;            // Rx message already received = 15 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                      // Rx message = 20 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[MSG_START];                       // We need 70 bytes but there are only 10 bytes left in msg buffer
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED when there is already a task at begin of message buffer");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }

    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in msg buffer\n"
                  "Rx size received > Tx size\n"
                  "There is already a task at begin of message buffer\n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Rx size currently received is greater than Tx message size.
        // Tx message doesn't fit in msg buffer.
        // There is already a Task at beginning, function should return FAILED
        //
        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-----|Task|--------------------------------------------------|
        //        |-------------------------------------|Rx                             |
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|        Tx                |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-----|Task|--------------------------------------------------|
        //        +-------------------------------------------------------------+
        //              |
        //              FAILED : there is a task
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 80;
        uint16_t rx_bytes_received = 51;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];     // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)]; // There are only 25 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;            // Rx message already received = 51 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                      // Rx message = 80 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[MSG_START];                       // We need 70 bytes but there are only 10 bytes left in msg buffer
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED when there is already a task at begin of message buffer");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }

    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in msg buffer\n"
                  "Rx size received = Tx size\n"
                  "There is already a task at begin of message buffer\n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size = Rx size currently received.
        // Tx message doesn't fit in msg buffer.
        // There is already a Task , function should return FAILED
        //
        //        msg_buffer init state (impossible state)
        //        +-------------------------------------------------------------+
        //        |-----|Task|--------------------------------------------------|
        //        |-------------------------------------|Rx                             |
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |-------------------------------------|        Tx                  |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-----|Task|--------------------------------------------------|
        //        +-------------------------------------------------------------+
        //                                                          |
        //                                                          FAILED : there is a task

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 80;
        uint16_t rx_bytes_received = tx_size;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];     // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)]; // There are only 25 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;            // Rx message already received = 50 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                      // Rx message = 50 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[MSG_START];                       // We need 70 bytes but there are only 10 bytes left in msg buffer
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED when there is already a task at begin of message buffer");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }
}
void unittest_SetTxTask_Tx_too_long_3()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in message buffer\n"
                  "Tx size > Rx size received \n"
                  "There is space at begin of message buffer\n");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size > Rx size currently received.
        // Tx message doesn't fit in msg buffer.
        // There is space : move messages and create Tx Task
        //
        //        Rx message to copy
        //        +-------------------------------------------------------------+
        //        |--------------------------------------------------|Rx|-------|
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |--------------------------------------------------|        Tx        |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |        Tx        |Rx|---------------------------------------|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;
        ack                 = 0;
        localhost           = 0;
        ll_service_pt       = (ll_service_t *)msg_buffer;                            // Fake service value
        data                = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];     // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)]; // There are only 25 bytes left in msg buffer
        data_end_estimation = (uint8_t *)current_msg + rx_size;                      // Complete Rx message = 20 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received;            // Rx message already received = 15 bytes
        oldest_msg          = (msg_t *)&msg_buffer[80];                              // There is a task at beginning of msg_buffer
        tx_tasks_stack_id   = 1;

        expected_current_msg         = (msg_t *)((uint32_t)msg_buffer + tx_size); // Rx message will be copied after Tx message
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)msg_buffer;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"localhost\" is correctly computed");
        TEST_ASSERT_EQUAL(localhost, tx_tasks[tx_tasks_stack_id].localhost);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)msg_buffer, tx_tasks[tx_tasks_stack_id].data_pt);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, msg_buffer, tx_size - CRC_SIZE);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, msg_buffer + tx_size, rx_bytes_received);
    }
}
void unittest_SetTxTask_Rx_too_long_1()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message fits in msg buffer\n"
                  "Tx size + Rx size received doesn't fit msg buffer\n"
                  "Tx size > Rx size received \n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size is greater than Rx size currently received.
        // Tx message fits in msg buffer.
        // Tx message + Rx Tx message does not fit in msg buffer.
        //
        //        msg_buffer init state (impossible state)
        //        +-------------------------------------------------------------+
        //        |-------------------------------|  Current Rx  |---|Task|-----|
        //        +-------------------------------------------------------------+
        //
        //        Tx message + Full Rx Message
        //        +-------------------------------------------------------------+
        //        |-------------------------------|         Tx           |    Full Rx    |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------|  Current Rx  |---|Task|-----|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 30;
        uint16_t rx_bytes_received = 10;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];                         // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + rx_bytes_received + 1)]; // There are only 61 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;                                // Rx message already received = 10 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                                          // Rx message = 30 bytes
        oldest_msg                 = (msg_t *)((uint32_t)current_msg + tx_size);                                // There is a task
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }

    //**************************************************************
    NEW_TEST_CASE("Tx message fits in msg buffer\n"
                  "Tx size + Rx size received doesn't fit msg buffer\n"
                  "Rx size received > Tx size \n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Rx size currently receivedis greater than Tx message size.
        // Tx message fits in msg buffer.
        // Tx message + Rx Tx message does not fit in msg buffer.
        //
        //
        //        Tx message + Full Rx Message
        //        +-------------------------------------------------------------+
        //        |-----------------------------------|  Tx   |       FULL Rx       |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-----------------------------------|Current Rx|------------|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 100;
        uint16_t rx_bytes_received = 51;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];                         // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + rx_bytes_received + 1)]; // There are only 101 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;                                // Rx message already received = 51 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                                          // Rx message = 100 bytes
        oldest_msg                 = (msg_t *)((uint32_t)current_msg + rx_bytes_received);                      // There is a task
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }

    //**************************************************************
    NEW_TEST_CASE("Tx message fits in msg buffer\n"
                  "Tx size + Rx size received doesn't fit msg buffer\n"
                  "Rx size received = Tx size \n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Rx size currently receivedis greater than Tx message size.
        // Tx message fits in msg buffer.
        // Tx message + Rx Tx message does not fit in msg buffer.
        //
        //
        //        Tx message + Full Rx Message
        //        +-------------------------------------------------------------+
        //        |--------------------------------------------------|  Tx   |  Rx   |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |--------------------------------------------------|  Rx   |- |
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 100;
        uint16_t rx_bytes_received = tx_size;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];                         // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + rx_bytes_received + 1)]; // There are only 100 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;                                // Rx message already received = 50 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                                          // Rx message = 100 bytes
        oldest_msg                 = (msg_t *)((uint32_t)current_msg + tx_size);                                // There is a task
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns FAILED");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }
}
void unittest_SetTxTask_Rx_too_long_2()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message fits in msg buffer\n"
                  "Tx size + Rx size received doesn't fit msg buffer\n"
                  "There is a Task \n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        //
        //        msg_buffer init state (impossible state)
        //        +-------------------------------------------------------------+
        //        |-------------------------------|  Current Rx  |---|Task|-----|
        //        +-------------------------------------------------------------+
        //
        //        Tx message + Full Rx Message
        //        +-------------------------------------------------------------+
        //        |-------------------------------|      Tx        |    Full Rx    |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-------------------------------|  Current Rx  |---|Task|-----|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 30;
        uint16_t rx_bytes_received = 10;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];                         // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + rx_bytes_received + 1)]; // There are only 61 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;                                // Rx message already received = 10 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                                          // Rx message = 30 bytes
        oldest_msg                 = (msg_t *)((uint32_t)&msg_buffer[MSG_BUFFER_SIZE] - sizeof(header_t));      // There is a task
        tx_tasks_stack_id          = 1;
        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns FAILED when Tx + Rx size doesn't fit msg buffer and a task is in requested memory");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }
}

void unittest_SetTxTask_Rx_too_long_3()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message fits in msg buffer\n"
                  "Tx size + Rx size received doesn't fit msg buffer\n"
                  "There is already a task at begin of message buffer\n");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-----|Task|------------------------|     Current Rx     |----|
        //        +-------------------------------------------------------------+
        //
        //        Message to copy
        //        +-------------------------------------------------------------+
        //        |-----------------------------------|         Tx           |     Current Rx     |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |-----|Task|------------------------|     Current Rx     |----|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 30;
        uint16_t rx_bytes_received = 10;
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];     // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + 1)]; // There are only 51 bytes left in msg buffer
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;            // Rx message already received = 10 bytes
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                      // Rx message = 30 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[0];                               // There is a task at beginning of buffer
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns FAILED when Tx + Rx size doesn't fit msg buffer and a task is in memory beginning");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }
}
void unittest_SetTxTask_Rx_too_long_4()
{
    //**************************************************************
    NEW_TEST_CASE("Tx + Rx messages doesn't in message buffer\n"
                  "There is space at begin of message buffer\n");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |-----------------------------------|         Tx           |     Current Rx     |
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |     Current Rx     |--------------|         Tx           |--|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;
        ack                 = 0;
        localhost           = 0;
        ll_service_pt       = (ll_service_t *)msg_buffer;                            // Fake service value
        data                = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];     // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + 1)]; // There are only 51 bytes left in msg buffer
        data_end_estimation = (uint8_t *)current_msg + rx_size;                      // Complete Rx message = 20 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received;            // Rx message already received = 15 bytes
        oldest_msg          = (msg_t *)&msg_buffer[80];                              // No task
        tx_tasks_stack_id   = 1;

        expected_current_msg         = (msg_t *)((uint32_t)msg_buffer); // Rx message will be copied at beginning of message buffer
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)msg_buffer;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"localhost\" is correctly computed");
        TEST_ASSERT_EQUAL(localhost, tx_tasks[tx_tasks_stack_id].localhost);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + 1)], tx_tasks[tx_tasks_stack_id].data_pt);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, &msg_buffer[MSG_BUFFER_SIZE - (tx_size + 1)], tx_size - CRC_SIZE);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, msg_buffer, rx_bytes_received);
    }
}
void unittest_SetTxTask_Task_already_exists()
{
    //**************************************************************
    NEW_TEST_CASE("Tx + Rx messages fit in message buffer\n"
                  "There is already a task in memory\n");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        //        msg_buffer init state
        //        +-------------------------------------------------------------+
        //        |------------------------| Rx |------|Task|-------------------|
        //        +-------------------------------------------------------------+
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |------------------------|     Tx      |----------------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |------------------------| Rx |----|Task|---------------------|
        //        +-------------------------------------------------------------+
        //                                           |
        //                                           FAILED : there is a task

        // Init variables
        //---------------
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        ack                        = 0;
        localhost                  = 0;
        ll_service_pt              = (ll_service_t *)msg_buffer;                        // Fake service value
        data                       = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size]; // Tx message = 50 bytes
        current_msg                = (msg_t *)&msg_buffer[MSG_START];                   // msg buffer is almost empty
        data_end_estimation        = (uint8_t *)current_msg + rx_size;                  // Complete Rx message = 20 bytes
        data_ptr                   = (uint8_t *)current_msg + rx_bytes_received;        // Rx message already received = 15 bytes
        oldest_msg                 = (msg_t *)&msg_buffer[rx_size + 1];                 // There is a Task
        tx_tasks_stack_id          = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns FAILED when  a task is in memory");
        TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(ll_service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }
}
void unittest_SetTxTask_copy_OK()
{
    //**************************************************************
    NEW_TEST_CASE("Tx and Rx messages fit in message buffer\n"
                  "Tx size > Rx size received \n");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size > Rx size currently received.
        // Tx and Rx messages fit in msg buffer.
        // There is space : move messages and create Tx Task
        //
        //        Tx message + Rx message to copy
        //        +-------------------------------------------------------------+
        //        |----------------------------------| Rx |---------------------|
        //        |----------------------------------|   Tx   |-----------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Tx   | Rx |------------|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;
        ack                 = 0;
        localhost           = 0;
        ll_service_pt       = (ll_service_t *)msg_buffer;                 // Fake service value
        data                = (uint8_t *)tx_message;                      // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_START];            // msg buffer is almost empty
        data_end_estimation = (uint8_t *)current_msg + rx_size;           // Complete Rx message = 20 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received; // Rx message already received = 15 bytes
        oldest_msg          = (msg_t *)&msg_buffer[2 * tx_size + 1];      // No Task
        tx_tasks_stack_id   = 1;

        expected_current_msg         = (msg_t *)((uint32_t)current_msg + tx_size); // Rx message will be copied after Tx message
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)current_msg;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"localhost\" is correctly computed");
        TEST_ASSERT_EQUAL(localhost, tx_tasks[tx_tasks_stack_id].localhost);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[MSG_START], tx_tasks[tx_tasks_stack_id].data_pt);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, (uint8_t *)&msg_buffer[MSG_START], tx_size - CRC_SIZE);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, (uint8_t *)&msg_buffer[MSG_START] + tx_size, rx_bytes_received);
    }

    //**************************************************************
    NEW_TEST_CASE("Tx and Rx messages fit in message buffer\n"
                  "Rx size received > Tx size\n");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Rx size currently received > Tx message size.
        // Tx and Rx messages fit in msg buffer.
        // There is space : move messages and create Tx Task
        //
        //        Tx message + Rx message to copy
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Rx   |-----------------|
        //        |----------------------------------| Tx |---------------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |----------------------------------| Tx |   Rx   |------------|
        //        +-------------------------------------------------------------+
        //                                                |
        //                                                Padding Added

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 80;
        uint16_t rx_bytes_received = 51;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;
        uint16_t padding;
        ack                 = 0;
        localhost           = 0;
        ll_service_pt       = (ll_service_t *)msg_buffer;                 // Fake service value
        data                = (uint8_t *)tx_message;                      // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_START];            // msg buffer is almost empty
        data_end_estimation = (uint8_t *)current_msg + rx_size;           // Complete Rx message = 80 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received; // Rx message already received = 51 bytes
        oldest_msg          = (msg_t *)&msg_buffer[2 * rx_size + 1];      // No Task
        padding             = rx_bytes_received - tx_size;                // Add padding (1 byte)
        tx_tasks_stack_id   = 1;

        expected_current_msg         = (msg_t *)((uint32_t)current_msg + tx_size + padding); // Rx message will be copied after Tx message with padding
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)current_msg;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"localhost\" is correctly computed");
        TEST_ASSERT_EQUAL(localhost, tx_tasks[tx_tasks_stack_id].localhost);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[MSG_START], tx_tasks[tx_tasks_stack_id].data_pt);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, (uint8_t *)&msg_buffer[MSG_START], tx_size - CRC_SIZE);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, (uint8_t *)&msg_buffer[MSG_START] + tx_size + padding, rx_bytes_received);
    }

    //**************************************************************
    NEW_TEST_CASE("Tx and Rx messages fit in message buffer\n"
                  "Tx size = Rx size received\n");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size = Rx size currently received.
        // Tx and Rx messages fit in msg buffer.
        // There is space : move messages and create Tx Task
        //
        //        Tx message + Rx message to copy
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Rx   |-----------------|
        //        |----------------------------------|   Tx   |-----------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Tx   |   Rx   |--------|
        //        +-------------------------------------------------------------+
        //                                                |
        //                                                Padding Added

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 80;
        uint16_t rx_bytes_received = 50;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;
        uint16_t padding;
        ack                 = 0;
        localhost           = 0;
        ll_service_pt       = (ll_service_t *)msg_buffer;                 // Fake service value
        data                = (uint8_t *)tx_message;                      // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_START];            // msg buffer is almost empty
        data_end_estimation = (uint8_t *)current_msg + rx_size;           // Complete Rx message = 80 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received; // Rx message already received = 51 bytes
        oldest_msg          = (msg_t *)&msg_buffer[2 * rx_size + 1];      // No Task
        tx_tasks_stack_id   = 1;

        expected_current_msg         = (msg_t *)((uint32_t)current_msg + tx_size); // Rx message will be copied after Tx message
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)current_msg;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"localhost\" is correctly computed");
        TEST_ASSERT_EQUAL(localhost, tx_tasks[tx_tasks_stack_id].localhost);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[MSG_START], tx_tasks[tx_tasks_stack_id].data_pt);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        TEST_ASSERT_EQUAL_MEMORY(tx_message, (uint8_t *)&msg_buffer[MSG_START], tx_size - CRC_SIZE);
        TEST_ASSERT_EQUAL_MEMORY(rx_message, (uint8_t *)&msg_buffer[MSG_START] + tx_size, rx_bytes_received);
    }

    //**************************************************************
    NEW_TEST_CASE("Tx and Rx messages fit in message buffer\n"
                  "Rx size received less than a header\n");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Rx size sizeof(header_t)
        // Tx and Rx messages fit in msg buffer.
        // There is space : move messages and create Tx Task
        //
        //        Tx message + Rx message to copy
        //        +-------------------------------------------------------------+
        //        |----------------------------------| Rx |---------------------|
        //        |----------------------------------|   Tx   |-----------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Tx   | Rx |------------|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 30;
        uint16_t rx_bytes_received = sizeof(header_t) - 1;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;
        ack                 = 0;
        localhost           = 0;
        ll_service_pt       = (ll_service_t *)msg_buffer;                 // Fake service value
        data                = (uint8_t *)tx_message;                      // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_START];            // msg buffer is almost empty
        data_end_estimation = (uint8_t *)current_msg + rx_size;           // Complete Rx message = 20 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received; // Rx message already received < header
        oldest_msg          = (msg_t *)&msg_buffer[2 * tx_size + 1];      // No Task
        tx_tasks_stack_id   = 1;

        expected_current_msg         = (msg_t *)((uint32_t)current_msg + tx_size); // Rx message will be copied after Tx message
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)current_msg;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"localhost\" is correctly computed");
        TEST_ASSERT_EQUAL(localhost, tx_tasks[tx_tasks_stack_id].localhost);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[MSG_START], tx_tasks[tx_tasks_stack_id].data_pt);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, (uint8_t *)&msg_buffer[MSG_START], tx_size - CRC_SIZE);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, (uint8_t *)&msg_buffer[MSG_START] + tx_size, rx_bytes_received);
    }
}

void unittest_SetTxTask_ACK()
{
    //**************************************************************
    NEW_TEST_CASE("Ack transmission");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message size > Rx size currently received.
        // Tx and Rx messages fit in msg buffer.
        // There is space : move messages and create Tx Task
        // Add Ack
        //
        //        Tx message + Rx message to copy
        //        +-------------------------------------------------------------+
        //        |----------------------------------| Rx |---------------------|
        //        |----------------------------------|   Tx   |-----------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Tx  + Ack  | Rx |------|
        //        +-------------------------------------------------------------+

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50 + sizeof(ack);
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;
        ack                 = 55;
        localhost           = 0;
        ll_service_pt       = (ll_service_t *)msg_buffer;                 // Fake service value
        data                = (uint8_t *)tx_message;                      // Tx message = 51 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_START];            // msg buffer is almost empty
        data_end_estimation = (uint8_t *)current_msg + rx_size;           // Complete Rx message = 20 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received; // Rx message already received = 15 bytes
        oldest_msg          = (msg_t *)&msg_buffer[2 * tx_size + 1];      // No Task
        tx_tasks_stack_id   = 1;

        expected_current_msg         = (msg_t *)((uint32_t)current_msg + tx_size); // Rx message will be copied after Tx message
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)current_msg;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE - sizeof(ack); i++)
        {
            tx_message[i] = i;
        }
        tx_message[tx_size] = ack;

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"localhost\" is correctly computed");
        TEST_ASSERT_EQUAL(localhost, tx_tasks[tx_tasks_stack_id].localhost);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[MSG_START], tx_tasks[tx_tasks_stack_id].data_pt);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check ACK : good value in good memory position");
        TEST_ASSERT_EQUAL(ack, msg_buffer[MSG_START + tx_size - 1]); // Ack
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, (uint8_t *)&msg_buffer[MSG_START], tx_size - CRC_SIZE - 1);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, (uint8_t *)&msg_buffer[MSG_START] + tx_size, rx_bytes_received);
    }
}

void unittest_SetTxTask_internal_localhost()
{
    //**************************************************************
    NEW_TEST_CASE("Internal Localhost");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost;
        uint8_t ack;

        // Tx message size > Rx size currently received.
        // Tx and Rx messages fit in msg buffer.
        // LocalHost : Compute msg_tasks[0]
        //
        //        Tx message + Rx message to copy
        //        +-------------------------------------------------------------+
        //        |----------------------------------| Rx |---------------------|
        //        |----------------------------------|   Tx   |-----------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Tx   | Rx |------------|
        //        +----------------------------------^--------------------------+
        //                                           |
        //                                           msg_tasks[0]

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;

        ack                 = 0;
        localhost           = LOCALHOST;                                  // Localhost : to fill msg_task
        ll_service_pt       = (ll_service_t *)msg_buffer;                 // Fake service value
        data                = (uint8_t *)tx_message;                      // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_START];            // msg buffer is almost empty
        data_end_estimation = (uint8_t *)current_msg + rx_size;           // Complete Rx message = 20 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received; // Rx message already received = 15 bytes
        oldest_msg          = (msg_t *)&msg_buffer[2 * tx_size + 1];      // No Task
        tx_tasks_stack_id   = 1;
        msg_tasks_stack_id  = 0;

        expected_current_msg         = (msg_t *)((uint32_t)current_msg + tx_size); // Rx message will be copied after Tx message
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)current_msg;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" = 1");
        TEST_ASSERT_EQUAL(1, tx_tasks_stack_id);
        NEW_STEP("Check \"message tasks stack id\" = 1");
        TEST_ASSERT_EQUAL(1, msg_tasks_stack_id);

        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks is void
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_NULL(tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_NULL(tx_tasks[tx_tasks_stack_id].data_pt);
        NEW_STEP("Check Tx task \"size\" = 0");
        TEST_ASSERT_EQUAL(0, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check \"localhost\" is not allocated");
        TEST_ASSERT_EQUAL(0, tx_tasks[tx_tasks_stack_id].localhost);

        // Check Message Tasks
        NEW_STEP("Check \"message tasks\" points to expected message");
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[MSG_START], msg_tasks[msg_tasks_stack_id - 1]);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, (uint8_t *)&msg_buffer[MSG_START], tx_size - CRC_SIZE);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, (uint8_t *)&msg_buffer[MSG_START] + tx_size, rx_bytes_received);
    }
}

void unittest_SetTxTask_multihost()
{ //**************************************************************
    NEW_TEST_CASE("MultiHost");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        ll_service_t *ll_service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost;
        uint8_t ack;

        // Tx message size > Rx size currently received.
        // Tx and Rx messages fit in msg buffer.
        // MultiHost : Compute msg_tasks[0] + tx_tasks
        //
        //        Tx message + Rx message to copy
        //        +-------------------------------------------------------------+
        //        |----------------------------------| Rx |---------------------|
        //        |----------------------------------|   Tx   |-----------------|
        //        +-------------------------------------------------------------+
        //
        //        msg_buffer ending state
        //        +-------------------------------------------------------------+
        //        |----------------------------------|   Tx   | Rx |------------|
        //        +----------------------------------^--------------------------+
        //                                           |
        //                                           msg_tasks[0]
        //                                           tx_tasks[x]

        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        msg_t *expected_current_msg;
        uint8_t *expected_data_ptr;
        uint8_t *expected_data_end_estimation;
        uint16_t tx_size           = 50;
        uint16_t rx_size           = 20;
        uint16_t rx_bytes_received = 15;
        uint8_t tx_message[tx_size];
        uint8_t rx_message[tx_size];
        uint8_t *init_rx_message;

        ack                 = 0;
        localhost           = MULTIHOST;
        ll_service_pt       = (ll_service_t *)msg_buffer;                 // Fake service value
        data                = (uint8_t *)tx_message;                      // Tx message = 50 bytes
        current_msg         = (msg_t *)&msg_buffer[MSG_START];            // msg buffer is almost empty
        data_end_estimation = (uint8_t *)current_msg + rx_size;           // Complete Rx message = 20 bytes
        data_ptr            = (uint8_t *)current_msg + rx_bytes_received; // Rx message already received = 15 bytes
        oldest_msg          = (msg_t *)&msg_buffer[2 * tx_size + 1];      // No Task
        tx_tasks_stack_id   = 1;
        msg_tasks_stack_id  = 0;

        expected_current_msg         = (msg_t *)((uint32_t)current_msg + tx_size); // Rx message will be copied after Tx message
        expected_data_ptr            = (uint8_t *)((uint32_t)expected_current_msg + rx_bytes_received);
        expected_data_end_estimation = (uint8_t *)((uint32_t)expected_current_msg + rx_size);

        expected_tx_task.size          = tx_size;
        expected_tx_task.data_pt       = (uint8_t *)current_msg;
        expected_tx_task.ll_service_pt = ll_service_pt;
        expected_tx_task.localhost     = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Init Rx message
        init_rx_message = (uint8_t *)current_msg;
        for (size_t i = 0; i < rx_bytes_received; i++)
        {
            rx_message[i]    = i + 100;
            *init_rx_message = i + 100;
            init_rx_message++;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(ll_service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is double incremented");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id); // 2 incrementations : 1 for tx_tasks + 1 for message_tasks
        NEW_STEP("Check \"message tasks stack id\" = 1");
        TEST_ASSERT_EQUAL(1, msg_tasks_stack_id);

        NEW_STEP("Check \"current message\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_current_msg, current_msg);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);
        NEW_STEP("Check \"data end estimation\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_end_estimation, data_end_estimation);

        // Check Tx Tasks AND Message Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(ll_service_pt, tx_tasks[tx_tasks_stack_id].ll_service_pt);
        NEW_STEP("Check \"localhost\" value is set to LOCALHOST");
        TEST_ASSERT_EQUAL(LOCALHOST, tx_tasks[tx_tasks_stack_id].localhost); //Mutlihost must be seen as localhost
        NEW_STEP("Check Tx task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((uint8_t *)&msg_buffer[MSG_START], tx_tasks[tx_tasks_stack_id].data_pt);
        NEW_STEP("Check message task \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL((msg_t *)&msg_buffer[MSG_START], msg_tasks[msg_tasks_stack_id - 1]);

#ifdef UNIT_TEST_DEBUG
        printf("\n");
        for (uint16_t i = 0; i < MSG_BUFFER_SIZE; i++)
        {
            printf("%d - ", msg_buffer[i]);
        }
        printf("\n");
#endif

        // Check messages
        NEW_STEP("Check Tx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(tx_message, (uint8_t *)&msg_buffer[MSG_START], tx_size - CRC_SIZE);
        NEW_STEP("Check Rx message integrity : correct values in correct memory position");
        TEST_ASSERT_EQUAL_MEMORY(rx_message, (uint8_t *)&msg_buffer[MSG_START] + tx_size, rx_bytes_received);
    }
}

/*******************************************************************************
 * MAIN
 ******************************************************************************/
int main(int argc, char **argv)
{
    UNITY_BEGIN();
    ASSERT_ACTIVATION(1);

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
