#include "main.h"
#include "unit_test.h"
#include "msg_alloc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MSG_START 10
#undef VERBOSE_LOCALHOST

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
    uint8_t localhost;     /*!< is this message a localhost one? */
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
extern volatile header_t *copy_task_pointer;
extern volatile msg_t *msg_tasks[MAX_MSG_NB];
extern volatile uint16_t msg_tasks_stack_id;
extern volatile luos_task_t luos_tasks[MAX_MSG_NB];
extern volatile uint16_t luos_tasks_stack_id;
extern volatile tx_task_t tx_tasks[MAX_MSG_NB];
extern volatile uint16_t tx_tasks_stack_id;

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
        service_t *service_pt;
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
            TEST_ASSERT_EQUAL(FAILED, MsgAlloc_SetTxTask(service_pt, &dummy_data, crc, size, localhost, ack));
            NEW_STEP_IN_LOOP("Check NO assert has occured", i - (MAX_MSG_NB - 1));
            TEST_ASSERT_FALSE(IS_ASSERT());
        }
    }
}

void unittest_SetTxTask_Tx_too_long()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in msg buffer");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        service_t *service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message doesn't fit in msg buffer function should assert
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |---|other msgs|-----------------------------|        Tx                   |
        //        +-------------------------------------------------------------+
        //

        // Init variables
        //---------------
        uint16_t tx_size  = 50;
        data              = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];       // Tx message = 50 bytes
        data_ptr          = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)]; // There are only 25 bytes left in msg buffer
        oldest_msg        = (msg_t *)&msg_buffer[10];                                // There is a task
        tx_tasks_stack_id = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        MsgAlloc_SetTxTask(service_pt, data, crc, tx_size, localhost, ack);
        NEW_STEP("Check assert has occured");
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}
void unittest_SetTxTask_Tx_overflow()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message doesn't fit in message buffer\n"
                  "There is space at begin of message buffer");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        service_t *service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message doesn't fit in msg buffer.
        // There is space in the begining of the buffer: move messages and create Tx Task
        //
        //
        //        Tx message to copy
        //        +-------------------------------------------------------------+
        //        |--------------------------------------------------|        Tx        |
        //        +-------------------------------------------------------------+
        //
        // Init variables
        //---------------
        tx_task_t expected_tx_task;
        uint8_t *expected_data_ptr;
        uint16_t tx_size = 50;
        uint8_t tx_message[tx_size];
        ack               = 0;
        localhost         = 0;
        service_pt        = (service_t *)msg_buffer;                                 // Fake service value
        data              = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];       // Tx message = 50 bytes
        data_ptr          = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size / 2)]; // There are only 25 bytes left in msg buffer
        oldest_msg        = (msg_t *)&msg_buffer[80];
        tx_tasks_stack_id = 1;

        expected_data_ptr = (uint8_t *)((uintptr_t)msg_buffer + tx_size);

        expected_tx_task.size       = tx_size;
        expected_tx_task.data_pt    = (uint8_t *)msg_buffer;
        expected_tx_task.service_pt = service_pt;
        expected_tx_task.localhost  = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(service_pt, tx_tasks[tx_tasks_stack_id].service_pt);
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
    }
}
void unittest_SetTxTask_Tx_fit()
{
    //**************************************************************
    NEW_TEST_CASE("Tx message fit in msg buffer");
    MsgAlloc_Init(NULL);
    {
        error_return_t result;
        uint8_t *data;
        service_t *service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        // Tx message fits in msg buffer.
        //
        //        Tx message  fit
        //        +-------------------------------------------------------------+
        //        |-------------------------------|         Tx           |------|
        //        +-------------------------------------------------------------+
        //

        // Init variables
        //---------------
        uint16_t tx_size  = 50;
        data              = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size];        // Tx message = 50 bytes
        data_ptr          = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - (tx_size + 20)]; // There are 70 bytes left in msg buffer                                      // Rx message = 30 bytes
        oldest_msg        = (msg_t *)((uintptr_t)data_ptr - tx_size);                 // There is a task before
        tx_tasks_stack_id = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(service_pt, data, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
    }
}
void unittest_SetTxTask_Task_already_exists()
{
    //**************************************************************
    NEW_TEST_CASE("Tx messages fit in message buffer, but there is already a task in memory");
    MsgAlloc_Init(NULL);
    memset((void *)msg_buffer, 0, sizeof(msg_buffer));
    {
        error_return_t result;
        uint8_t *data;
        service_t *service_pt;
        uint16_t crc;
        uint16_t size;
        luos_localhost_t localhost = EXTERNALHOST;
        uint8_t ack;

        //
        //        Tx message hit the next task
        //        +-------------------------------------------------------------+
        //        |------------------------|     Tx   |Task|--------------------|
        //        +-------------------------------------------------------------+
        //

        // Init variables
        //---------------
        uint16_t tx_size  = 50;
        ack               = 0;
        localhost         = 0;
        service_pt        = (service_t *)msg_buffer;                           // Fake service value
        data              = (uint8_t *)&msg_buffer[MSG_BUFFER_SIZE - tx_size]; // Tx message = 50 bytes
        data_ptr          = (uint8_t *)&msg_buffer[MSG_START];                 // msg buffer is almost empty
        oldest_msg        = (msg_t *)&msg_buffer[(uintptr_t)data_ptr + 1];     // There is a Task
        tx_tasks_stack_id = 1;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        MsgAlloc_SetTxTask(service_pt, data, crc, tx_size, localhost, ack);
        NEW_STEP("Check assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());
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
        service_t *service_pt;
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
        uint8_t *expected_data_ptr;
        uint16_t tx_size = 50 + sizeof(ack);
        uint8_t tx_message[tx_size];
        ack               = 55;
        localhost         = 0;
        service_pt        = (service_t *)msg_buffer;               // Fake service value
        data              = (uint8_t *)tx_message;                 // Tx message = 51 bytes
        data_ptr          = (uint8_t *)&msg_buffer[MSG_START];     // msg buffer is almost empty
        oldest_msg        = (msg_t *)&msg_buffer[2 * tx_size + 1]; // No Task
        tx_tasks_stack_id = 1;

        expected_data_ptr = (uint8_t *)((uintptr_t)data_ptr + tx_size); // Rx message will be copied after Tx message

        expected_tx_task.size       = tx_size;
        expected_tx_task.data_pt    = (uint8_t *)data_ptr;
        expected_tx_task.service_pt = service_pt;
        expected_tx_task.localhost  = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE - sizeof(ack); i++)
        {
            tx_message[i] = i;
        }
        tx_message[tx_size] = ack;

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is incremented to 2");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id);
        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);

        // Check Tx Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(service_pt, tx_tasks[tx_tasks_stack_id].service_pt);
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
        service_t *service_pt;
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
        uint8_t *expected_data_ptr;
        uint16_t tx_size = 50;
        uint8_t tx_message[tx_size];

        ack                = 0;
        localhost          = LOCALHOST;                             // Localhost : to fill msg_task
        service_pt         = (service_t *)msg_buffer;               // Fake service value
        data               = (uint8_t *)tx_message;                 // Tx message = 50 bytes
        data_ptr           = (uint8_t *)&msg_buffer[MSG_START];     // msg buffer is almost empty
        oldest_msg         = (msg_t *)&msg_buffer[2 * tx_size + 1]; // No Task
        tx_tasks_stack_id  = 1;
        msg_tasks_stack_id = 0;

        expected_data_ptr = (uint8_t *)((uintptr_t)data_ptr + tx_size);

        expected_tx_task.size       = tx_size;
        expected_tx_task.data_pt    = (uint8_t *)data_ptr;
        expected_tx_task.service_pt = service_pt;
        expected_tx_task.localhost  = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" = 1");
        TEST_ASSERT_EQUAL(1, tx_tasks_stack_id);
        NEW_STEP("Check \"message tasks stack id\" = 1");
        TEST_ASSERT_EQUAL(1, msg_tasks_stack_id);

        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);

        // Check Tx Tasks is void
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_NULL(tx_tasks[tx_tasks_stack_id].service_pt);
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
        service_t *service_pt;
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
        uint8_t *expected_data_ptr;
        uint16_t tx_size = 50;
        uint8_t tx_message[tx_size];

        ack                = 0;
        localhost          = MULTIHOST;
        service_pt         = (service_t *)msg_buffer;               // Fake service value
        data               = (uint8_t *)tx_message;                 // Tx message = 50 bytes
        data_ptr           = (uint8_t *)&msg_buffer[MSG_START];     // msg buffer is almost empty
        oldest_msg         = (msg_t *)&msg_buffer[2 * tx_size + 1]; // No Task
        tx_tasks_stack_id  = 1;
        msg_tasks_stack_id = 0;

        expected_data_ptr = (uint8_t *)((uintptr_t)data_ptr + tx_size);

        expected_tx_task.size       = tx_size;
        expected_tx_task.data_pt    = (uint8_t *)data_ptr;
        expected_tx_task.service_pt = service_pt;
        expected_tx_task.localhost  = localhost;

        // Init Tx message
        for (size_t i = 0; i < tx_size - CRC_SIZE; i++)
        {
            tx_message[i] = i;
        }

        // Call function and Verify
        //---------------------------
        RESET_ASSERT();
        NEW_STEP("Check function returns SUCCEED");
        TEST_ASSERT_EQUAL(SUCCEED, MsgAlloc_SetTxTask(service_pt, tx_message, crc, tx_size, localhost, ack));
        NEW_STEP("Check NO assert has occured");
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Check pointers
        NEW_STEP("Check \"tx tasks stack id\" is double incremented");
        TEST_ASSERT_EQUAL(2, tx_tasks_stack_id); // 2 incrementations : 1 for tx_tasks + 1 for message_tasks
        NEW_STEP("Check \"message tasks stack id\" = 1");
        TEST_ASSERT_EQUAL(1, msg_tasks_stack_id);

        NEW_STEP("Check \"data pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(expected_data_ptr, data_ptr);

        // Check Tx Tasks AND Message Tasks
        tx_tasks_stack_id--;
        NEW_STEP("Check Tx task \"size\" is correctly computed");
        TEST_ASSERT_EQUAL(tx_size, tx_tasks[tx_tasks_stack_id].size);
        NEW_STEP("Check Tx task \"service pointer\" is correctly computed");
        TEST_ASSERT_EQUAL(service_pt, tx_tasks[tx_tasks_stack_id].service_pt);
        NEW_STEP("Check \"localhost\" value is set to LOCALHOST");
        TEST_ASSERT_EQUAL(LOCALHOST, tx_tasks[tx_tasks_stack_id].localhost); // Mutlihost must be seen as localhost
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
    }
}
