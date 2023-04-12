/******************************************************************************
 * @file msgAlloc.c
 * @brief Message allocation manager
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

/******************************************************************************
 * Here is a description of the allocator. Letters on arrow represent events
 * described below
 *
 *         msg_buffer
 *        +-------------------------------------------------------------+
 *        |hhhhhhhdddd|-------------------------------------------------|
 *        +------^---^--------------------------------------------------+
 *               |   |
 *               A   B    msg_tasks          Luos_tasks      tx_tasks
 *                   |   +---------+        +---------+     +---------+
 *                   +-->|  Msg B  |---C--->| Task D1 |     | Task E1 |
 *                       |---------|<id     | Task D2 |     |---------|<id
 *                       |---------|        |---------|<id  |---------|
 *                       |---------|        |---------|     |---------|
 *                       +---------+        +---------+     +---------+
 *
 *  - Event A : This event is called by IT and represent the end of reception of
 *              the header. In this event we get the size of the complete message
 *              so we can check if we are at the end of the msg_buffer and report
 *              writing pointer to the begin of msg_buffer
 *  - Event B : This event is called by IT and represent the end of a good message.
 *              In this event we have to save the message into a msg_tasks
 *              called "Msg B" on this example. we have to check if there is validated
 *              tasks at this space in memory and clear the memory space use by
 *              msg_tasks or Luos_tasks. Also we have to prepare the reception of
 *              the next header.
 *  - Event C : This event represent phy_loop and it is executed outside of IT.
 *              This event pull msg_tasks tasks and interpret all messages to
 *              create one or more Luos_tasks.
 *  - Task D  : This is all msg trait by Luos Library interpret in Luos_loop. Msg can be
 *              for Luos Library or for service. this is executed outside of IT.
 *  - Task E  : Msg_buffer can also save some TX tasks and list them into tx_task tasks
 *
 * After all of it Luos_tasks are ready to be managed by luos_loop execution.
 ******************************************************************************/

#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "msg_alloc.h"
#include "luos_hal.h"
#include "luos_utils.h"
#include "node.h"
#include "filter.h"
#include "struct_luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct luos_task_t
 * @brief Message allocator loger structure.
 *
 * This structure is used to link services and messages into the allocator.
 *
 ******************************************************************************/
typedef struct
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
memory_stats_t *mem_stat   = NULL;
volatile bool reset_needed = false;

// msg buffering
volatile uint8_t msg_buffer[MSG_BUFFER_SIZE]; /*!< Memory space used to save and alloc messages. */
volatile uint8_t *data_ptr;                   /*!< Pointer to the next data able to be written into msgbuffer. */

volatile msg_t *oldest_msg = NULL; /*!< The oldest message among all the stacks. */
volatile msg_t *used_msg   = NULL; /*!< Message curently used by luos loop. */

// msg interpretation task stack
volatile msg_t *msg_tasks[MAX_MSG_NB]; /*!< ready message table. */
volatile uint16_t msg_tasks_stack_id;  /*!< Next writen msg_tasks id. */

// Luos task stack
volatile luos_task_t luos_tasks[MAX_MSG_NB]; /*!< Message allocation table. */
volatile uint16_t luos_tasks_stack_id;       /*!< Next writen luos_tasks id. */

// Tx task stack
volatile tx_task_t tx_tasks[MAX_MSG_NB]; /*!< Message to transmit allocation table. */
volatile uint16_t tx_tasks_stack_id;     /*!< Next writen tx_tasks id. */

/*******************************************************************************
 * Functions
 ******************************************************************************/

// msg buffering
_CRITICAL static inline error_return_t MsgAlloc_DoWeHaveSpace(void *to);

// Allocator task stack
_CRITICAL static inline error_return_t MsgAlloc_ClearMsgSpace(void *from, void *to);

// Allocator task stack TX check space
static inline error_return_t MsgAlloc_CheckMsgSpace(void *from, void *to);

// msg interpretation task stack
_CRITICAL static inline void MsgAlloc_ClearMsgTask(void);

// Luos task stack
_CRITICAL static inline void MsgAlloc_ClearLuosTask(uint16_t luos_task_id);

// Available buffer space evaluation
static inline uint32_t MsgAlloc_BufferAvailableSpaceComputation(void);

// Check if this message is the oldest
_CRITICAL static inline void MsgAlloc_OldestMsgCandidate(msg_t *oldest_stack_msg_pt);

// Find the oldest message curretly stored
_CRITICAL static inline void MsgAlloc_FindNewOldestMsg(void);

/*******************************************************************************
 * Functions --> generic
 ******************************************************************************/

/******************************************************************************
 * @brief Init the allocator.
 * @param Pointer to Node statistics
 * @return None
 ******************************************************************************/
void MsgAlloc_Init(memory_stats_t *memory_stats)
{
    //******** Init global vars pointers **********
    data_ptr           = (uint8_t *)&msg_buffer[0];
    msg_tasks_stack_id = 0;
    memset((void *)msg_tasks, 0, sizeof(msg_tasks));
    luos_tasks_stack_id = 0;
    memset((void *)luos_tasks, 0, sizeof(luos_tasks));
    tx_tasks_stack_id = 0;
    memset((void *)tx_tasks, 0, sizeof(tx_tasks));
    used_msg   = NULL;
    oldest_msg = (msg_t *)INT_MAX;
    if (memory_stats != NULL)
    {
        mem_stat = memory_stats;
    }
    Filter_IdInit(); // Mask filter for service ID
    // Reset have been made
    reset_needed = false;
}
/******************************************************************************
 * @brief execute some things out of IRQ
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_loop(void)
{
    // Compute memory stats for msg task memory usage
    uint8_t stat = 0;
    // Compute memory stats for rx msg task memory usage
    stat = (uint8_t)(((uintptr_t)msg_tasks_stack_id * 100) / (MAX_MSG_NB));
    if (stat > mem_stat->rx_msg_stack_ratio)
    {
        mem_stat->rx_msg_stack_ratio = stat;
    }
    // Compute memory stats for tx msg task memory usage
    stat = (uint8_t)(((uintptr_t)tx_tasks_stack_id * 100) / (MAX_MSG_NB));
    if (stat > mem_stat->tx_msg_stack_ratio)
    {
        mem_stat->tx_msg_stack_ratio = stat;
    }
    // Compute buffer occupation rate
    stat = (uint8_t)(((MSG_BUFFER_SIZE - MsgAlloc_BufferAvailableSpaceComputation()) * 100) / (MSG_BUFFER_SIZE));
    if (stat > mem_stat->buffer_occupation_ratio)
    {
        mem_stat->buffer_occupation_ratio = stat;
    }
}
/******************************************************************************
 * @brief compute remaing space on msg_buffer.
 * @param None
 * @return Available space in bytes
 ******************************************************************************/
static inline uint32_t MsgAlloc_BufferAvailableSpaceComputation(void)
{
    uint32_t stack_free_space = 0;

    LuosHAL_SetIrqState(false);
    if ((uintptr_t)oldest_msg != INT_MAX)
    {
        LUOS_ASSERT(((uintptr_t)oldest_msg >= (uintptr_t)&msg_buffer[0]) && ((uintptr_t)oldest_msg < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE]));
        // There is some tasks
        if ((uintptr_t)oldest_msg > (uintptr_t)data_ptr)
        {
            // The oldest task is between `data_ptr` and the end of the buffer
            //        msg_buffer
            //        +-------------------------------------------------------------+
            //        |-------------------------------------------------------------|
            //        +------^---------------------^--------------------------------+
            //               |                     |
            //               |<-----Free space---->|
            //               |                     |
            //               data_ptr              oldest_task
            //
            stack_free_space = (uintptr_t)oldest_msg - (uintptr_t)data_ptr;
            LuosHAL_SetIrqState(true);
        }
        else
        {
            // The oldest task is between the begin of the buffer and `current_msg`
            //
            //        msg_buffer
            //        +-------------------------------------------------------------+
            //        |-------------------------------------------------------------|
            //        +-------------^---------------------------------^-------------+
            //                      |                                 |
            //        <-Free space->|                                 |<-Free space->
            //                      |                                 |
            //                      |                                 |
            //                      oldest_task                       data_ptr
            //
            stack_free_space = ((uintptr_t)oldest_msg - (uintptr_t)&msg_buffer[0]) + ((uintptr_t)&msg_buffer[MSG_BUFFER_SIZE] - (uintptr_t)data_ptr);
            LuosHAL_SetIrqState(true);
        }
    }
    else
    {
        // There is no task yet just compute the actual reception
        stack_free_space = MSG_BUFFER_SIZE;
        LuosHAL_SetIrqState(true);
    }
    return stack_free_space;
}
/******************************************************************************
 * @brief save the given msg as oldest if it is
 * @param oldest_stack_msg_pt : the oldest message of a stack
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL static inline void MsgAlloc_OldestMsgCandidate(msg_t *oldest_stack_msg_pt)
{
    if ((uintptr_t)oldest_stack_msg_pt > 0)
    {
        LUOS_ASSERT(((uintptr_t)oldest_stack_msg_pt >= (uintptr_t)&msg_buffer[0]) && ((uintptr_t)oldest_stack_msg_pt < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE]));
        // recompute oldest_stack_msg_pt into delta byte from current message
        uint32_t stack_delta_space;
        if ((uintptr_t)oldest_stack_msg_pt > (uintptr_t)data_ptr)
        {
            // The oldest task is between `data_ptr` and the end of the buffer
            LuosHAL_SetIrqState(false);
            stack_delta_space = (uintptr_t)oldest_stack_msg_pt - (uintptr_t)data_ptr;
            LuosHAL_SetIrqState(true);
        }
        else
        {
            // The oldest task is between the begin of the buffer and `data_ptr`
            // we have to decay it to be able to define delta
            LuosHAL_SetIrqState(false);
            stack_delta_space = ((uintptr_t)oldest_stack_msg_pt - (uintptr_t)&msg_buffer[0]) + ((uintptr_t)&msg_buffer[MSG_BUFFER_SIZE] - (uintptr_t)data_ptr);
            LuosHAL_SetIrqState(true);
        }
        // recompute oldest_msg into delta byte from current message
        uintptr_t oldest_msg_delta_space;
        if ((uintptr_t)oldest_msg > (uintptr_t)data_ptr)
        {
            // The oldest msg is between `data_ptr` and the end of the buffer
            LuosHAL_SetIrqState(false);
            oldest_msg_delta_space = (uintptr_t)oldest_msg - (uintptr_t)data_ptr;
            LuosHAL_SetIrqState(true);
        }
        else
        {
            // The oldest msg is between the begin of the buffer and `data_ptr`
            // we have to decay it to be able to define delta
            LuosHAL_SetIrqState(false);
            oldest_msg_delta_space = ((uintptr_t)oldest_msg - (uintptr_t)&msg_buffer[0]) + ((uintptr_t)&msg_buffer[MSG_BUFFER_SIZE] - (uintptr_t)data_ptr);
            LuosHAL_SetIrqState(true);
        }
        // Compare deltas
        if (stack_delta_space < oldest_msg_delta_space)
        {
            // This one is the new oldest message
            oldest_msg = oldest_stack_msg_pt;
        }
    }
}
/******************************************************************************
 * @brief update the new oldest message if we need to
 * @param removed_msg : the freshly oldest removed message of the stack
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL static inline void MsgAlloc_FindNewOldestMsg(void)
{
    // Reinit the value
    oldest_msg = (msg_t *)INT_MAX;
    MSGALLOC_MUTEX_LOCK
    // start parsing tasks to find the oldest message
    // check it on msg_tasks
    MsgAlloc_OldestMsgCandidate((msg_t *)msg_tasks[0]);
    // check it on luos_tasks
    MsgAlloc_OldestMsgCandidate(luos_tasks[0].msg_pt);
    // check it on tx_tasks
    MsgAlloc_OldestMsgCandidate((msg_t *)tx_tasks[0].data_pt);
    MSGALLOC_MUTEX_UNLOCK
}

/*******************************************************************************
 * Functions --> msg buffering
 ******************************************************************************/

/******************************************************************************
 * @brief check if there is enought space to store this data into buffer
 * @param from : start of the memory space to clean
 * @param to : start of the memory space to clean
 * @return error_return_t
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL static inline error_return_t MsgAlloc_DoWeHaveSpace(void *to)
{
    if ((uintptr_t)to > ((uintptr_t)&msg_buffer[MSG_BUFFER_SIZE - 1]))
    {
        // We reach msg_buffer end.
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        |-------------------------------------------------------------+  ^
        //                                                                         |
        //                                                                      pointer
        // Return as FAILED
        //
        return FAILED;
    }
    return SUCCEED;
}
/******************************************************************************
 * @brief Allocate a new message
 * @param uint16_t data_size
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL uint8_t *MsgAlloc_Alloc(uint16_t data_size)
{
    uint8_t *returned_ptr;
    // Check data ptr alignement
    if ((uintptr_t)data_ptr % 2 == 1)
    {
        data_ptr++;
    }
    // Check if we have space for the message
    if (MsgAlloc_DoWeHaveSpace((void *)(data_ptr + data_size)) == FAILED)
    {
        //
        // We don't have the space to store the message :
        //        +-------------------------------------------------------------+
        //        |------------------------------------|      Datas to be received       |
        //        +------------------------------------^------------------------+        ^
        //                                             |                                 |
        //                                          data_ptr                     data_end
        //        move data_ptr to the beginning of the buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------------------------------------------|
        //        |      Datas to be received       |---------------------------|
        //        ^---------------------------------^---------------------------+
        //        |                                 |
        //    data_ptr                              data_end
        //

        // We don't have the space to store the message, move data_ptr to the beginning of the buffer
        returned_ptr = (uint8_t *)&msg_buffer[0];
    }
    else
    {
        returned_ptr = (uint8_t *)data_ptr;
    }
    // Check if we have space for the message, assert if we don't
    LUOS_ASSERT(MsgAlloc_CheckMsgSpace((void *)returned_ptr, (void *)((uintptr_t)returned_ptr + data_size)) == SUCCEED);

    // We consider this space as occupied, move data to the next available space
    data_ptr = (uint8_t *)((uintptr_t)returned_ptr + data_size);
    return returned_ptr;
}
/******************************************************************************
 * @brief No message in buffer receive since initialization
 * @param None
 * @return msg_t* sucess or fail if good init
 ******************************************************************************/
error_return_t MsgAlloc_IsEmpty(void)
{
    if (data_ptr == &msg_buffer[0])
    {
        return SUCCEED;
    }
    else
    {
        return FAILED;
    }
}
/******************************************************************************
 * @brief Reset msg_alloc tx_tasks to avoid sending messages
 * @param None
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void MsgAlloc_Reset(void)
{
    // We will need to reset

    MSGALLOC_MUTEX_LOCK
    reset_needed      = true;
    tx_tasks_stack_id = 0;
    memset((void *)tx_tasks, 0, sizeof(tx_tasks));
    MSGALLOC_MUTEX_UNLOCK
}
/******************************************************************************
 * @brief Check if we need to reset Msg alloc
 * @param None
 * @return SUCCEED or FAILED if msg alloc is reset or not
 ******************************************************************************/
error_return_t MsgAlloc_IsReseted(void)
{
    // Check if we need to reset everything due to detection reset
    if (reset_needed)
    {
        if (Node_Get()->node_id != 0)
        {
            Node_Get()->node_id = 0;
            // We need to reset MsgAlloc
            MsgAlloc_Init(NULL);
        }
        reset_needed = false;
        return SUCCEED;
    }
    return FAILED;
}

/*******************************************************************************
 * Functions --> Allocator task stack
 ******************************************************************************/

/******************************************************************************
 * @brief prepare a buffer space to be usable by cleaning remaining messages and prepare pointers
 * @param from : start of the memory space to clean
 * @param to : start of the memory space to clean
 * @return error_return_t
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL static inline error_return_t MsgAlloc_ClearMsgSpace(void *from, void *to)
{
    //******** Check if there is sufficient space on the buffer **********
    if (MsgAlloc_DoWeHaveSpace(to) == FAILED)
    {
        // We reach msg_buffer end return an error
        return FAILED;
    }
    //******** Prepare a memory space to be writable **********

    // check if there is a msg traitement pending
    if (((uintptr_t)used_msg >= (uintptr_t)from) && ((uintptr_t)used_msg <= (uintptr_t)to))
    {
        used_msg = NULL;
        // This message is in the space we want to use, clear the task
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
            mem_stat->buffer_occupation_ratio = 100;
        }
    }
    // check if there is a msg in the space we need
    // Start by checking if the oldest message is out of scope
    if (((uintptr_t)oldest_msg >= (uintptr_t)from) && ((uintptr_t)oldest_msg <= (uintptr_t)to))
    {
        // We have to drop some messages for sure
        mem_stat->buffer_occupation_ratio = 100;
        while (((uintptr_t)luos_tasks[0].msg_pt >= (uintptr_t)from) && ((uintptr_t)luos_tasks[0].msg_pt <= (uintptr_t)to) && (luos_tasks_stack_id > 0))
        {
            // This message is in the space we want to use, clear all the Luos task
            MsgAlloc_ClearLuosTask(0);
            if (mem_stat->msg_drop_number < 0xFF)
            {
                mem_stat->msg_drop_number++;
                mem_stat->buffer_occupation_ratio = 100;
            }
        }
        // check if there is no msg between from and to on msg_tasks
        while (((uintptr_t)msg_tasks[0] >= (uintptr_t)from) && ((uintptr_t)msg_tasks[0] <= (uintptr_t)to) && (msg_tasks_stack_id > 0))
        {
            // This message is in the space we want to use, clear all the message task
            MsgAlloc_ClearMsgTask();
            if (mem_stat->msg_drop_number < 0xFF)
            {
                mem_stat->msg_drop_number++;
                mem_stat->buffer_occupation_ratio = 100;
            }
        }
        // check if there is no msg between from and to on tx_tasks
        while (((uintptr_t)tx_tasks[0].data_pt >= (uintptr_t)from) && ((uintptr_t)tx_tasks[0].data_pt <= (uintptr_t)to) && (tx_tasks_stack_id > 0))
        {
            // This message is in the space we want to use, clear all the Tx task
            MsgAlloc_PullMsgFromTxTask();
            if (mem_stat->msg_drop_number < 0xFF)
            {
                mem_stat->msg_drop_number++;
                mem_stat->buffer_occupation_ratio = 100;
            }
        }
    }
    // if we go here there is no reason to continue because newest messages can't overlap the memory zone.
    return SUCCEED;
}
/******************************************************************************
 * @brief Check if there is space in buffer
 * @param from : start of the memory space to check
 * @param to : stop of the memory space to check
 * @return error_return_t
 ******************************************************************************/
static inline error_return_t MsgAlloc_CheckMsgSpace(void *from, void *to)
{
    if ((((uintptr_t)used_msg >= (uintptr_t)from) && ((uintptr_t)used_msg <= (uintptr_t)to))
        || (((uintptr_t)oldest_msg >= (uintptr_t)from) && ((uintptr_t)oldest_msg <= (uintptr_t)to)))
    {
        // FAILED CASES :
        //
        //        msg_buffer
        //        +-------------------------------------------------------------+
        //        |-------------------------|   MESSAGES...   |-----------------|
        //        |--------------^----------^---^-------------------------------+
        //                       |          |   |
        //                     from         |   to
        //                                  |
        //                             "used_msg"
        //                         or  "oldest_msg"
        //
        return FAILED;
    }
    return SUCCEED;
}
/*******************************************************************************
 * Functions --> msg interpretation task stack
 ******************************************************************************/

/******************************************************************************
 * @brief Clear a slot. This action is due to an error
 * @param None
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL static inline void MsgAlloc_ClearMsgTask(void)
{
    LUOS_ASSERT((msg_tasks_stack_id <= MAX_MSG_NB) && (msg_tasks_stack_id > 0));

    //
    //     msg_tasks init state               msg_tasks ending state
    //     +---------+				          +---------+
    //     |  MSG_1  |				          |  MSG_2  |
    //     +---------+				          +---------+
    //     |  MSG_2  |				          |  MSG_3  |
    //     +---------+				          +---------+
    //     | etc...  |				          | etc...  |<--msg_tasks_stack_id
    //     +---------+				          +---------+
    //     |  Last   |<--msg_tasks_stack_id	  |    0    |   <--- Last message is cleared
    //     +---------+				          +---------+
    //     | etc...  |				          | etc...  |
    //     +---------+				          +---------+
    //
    MSGALLOC_MUTEX_LOCK
    for (uint16_t rm = 0; rm < msg_tasks_stack_id; rm++)
    {
        LuosHAL_SetIrqState(true);
        LuosHAL_SetIrqState(false);
        LUOS_ASSERT((msg_tasks[rm] != 0));
        msg_tasks[rm] = msg_tasks[rm + 1];
    }

    msg_tasks_stack_id--;
    msg_tasks[msg_tasks_stack_id] = 0;

    LuosHAL_SetIrqState(true);
    MSGALLOC_MUTEX_UNLOCK
    MsgAlloc_FindNewOldestMsg();
}
/******************************************************************************
 * @brief Pull a message that is not interpreted by the phy yet
 * @param returned_msg : The message pointer.
 * @return error_return_t
 ******************************************************************************/
error_return_t MsgAlloc_PullMsgToInterpret(msg_t **returned_msg)
{
    if (msg_tasks_stack_id > 0)
    {
        // Case SUCCEED
        //
        //         msg_tasks init state                msg_tasks ending state
        //             +---------+                        +---------+
        //             |  MSG_1  |                        |  MSG_2  |<--"returned_msg" points to 1st message of msg_tasks
        //             |---------|                        |---------|
        //             |  MSG_2  |                        |  MSG_3  |
        //             |---------|                        |---------|
        //             |  etc... |                        |  etc... |
        //             |---------|                        |---------|<--msg_tasks_stack_id
        //             |  LAST   |                        |    0    |
        //             +---------+<--msg_tasks_stack_id   +---------+
        //
        *returned_msg = (msg_t *)msg_tasks[0];
        LUOS_ASSERT(((uintptr_t)*returned_msg >= (uintptr_t)&msg_buffer[0]) && ((uintptr_t)*returned_msg < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE]));
        MsgAlloc_ClearMsgTask();
        return SUCCEED;
    }
    // At this point we don't find any message for this service
    return FAILED;
}

/*******************************************************************************
 * Functions --> Luos task stack
 ******************************************************************************/

/******************************************************************************
 * @brief Notify the end of the usage of the message.
 * @param returned_msg : The message pointer.
 * @return None
 ******************************************************************************/
void MsgAlloc_UsedMsgEnd(void)
{
    used_msg = NULL;
}
/******************************************************************************
 * @brief Clear a slot. This action is due to an error
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL static inline void MsgAlloc_ClearLuosTask(uint16_t luos_task_id)
{
    LUOS_ASSERT((luos_task_id < luos_tasks_stack_id) && (luos_tasks_stack_id <= MAX_MSG_NB));
    //
    // Start to clear from "luos_task_id"
    //
    //         Luos_tasks init state                Luos_tasks ending state
    //             +---------+                        +---------+
    //             |  MSG_1  |                        |  MSG_1  |
    //             +---------+                        +---------+
    //             |  MSG_2  |                        |  MSG_2  |
    //             +---------+                        +---------+
    //             |  etc... |                        |  etc... |
    //             +---------+                        +---------+
    //             |  Msg X  |<--luos_task_id         |    0    |<--luos_tasks_stack_id
    //             +---------+                        +---------+
    //             |  etc... |                        |    0    |
    //             |---------|                        |---------|
    //             |  LAST   |                        |    0    |
    //             +---------+                        +---------+
    //             |    0    |<--luos_tasks_stack_id  |    0    |
    //             +---------+                        |---------|
    //
    MSGALLOC_MUTEX_LOCK
    for (uint16_t rm = luos_task_id; rm < luos_tasks_stack_id; rm++)
    {
        LuosHAL_SetIrqState(false);
        luos_tasks[rm] = luos_tasks[rm + 1];
        LuosHAL_SetIrqState(true);
    }
    LuosHAL_SetIrqState(false);
    if (luos_tasks_stack_id != 0)
    {
        luos_tasks_stack_id--;
        luos_tasks[luos_tasks_stack_id].msg_pt     = 0;
        luos_tasks[luos_tasks_stack_id].service_pt = 0;
    }
    LuosHAL_SetIrqState(true);
    MSGALLOC_MUTEX_UNLOCK
    MsgAlloc_FindNewOldestMsg();
}
/******************************************************************************
 * @brief Alloc luos task
 * @param service_concerned_by_current_msg concerned services
 * @param service_concerned_by_current_msg concerned msg
 * @return None
 ******************************************************************************/
void MsgAlloc_LuosTaskAlloc(service_t *service_concerned_by_current_msg, msg_t *concerned_msg)
{
    // Find a free slot
    if (luos_tasks_stack_id == MAX_MSG_NB)
    {
        // There is no more space on the luos_tasks, remove the oldest msg.
        MsgAlloc_ClearLuosTask(0);
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
            mem_stat->engine_msg_stack_ratio = 100;
        }
    }
    // Fill the informations of the message in this slot
    MSGALLOC_MUTEX_LOCK
    LuosHAL_SetIrqState(false);
    LUOS_ASSERT(luos_tasks_stack_id < MAX_MSG_NB);
    luos_tasks[luos_tasks_stack_id].msg_pt     = concerned_msg;
    luos_tasks[luos_tasks_stack_id].service_pt = service_concerned_by_current_msg;
    if (luos_tasks_stack_id == 0)
    {
        // This is the first message in the stack, so it could be the oldest one.
        MsgAlloc_OldestMsgCandidate(luos_tasks[0].msg_pt);
    }
    luos_tasks_stack_id++;
    LuosHAL_SetIrqState(true);
    MSGALLOC_MUTEX_UNLOCK
    // Luos task memory usage
    uint8_t stat = (uint8_t)(((uintptr_t)luos_tasks_stack_id * 100) / (MAX_MSG_NB));
    if (stat > mem_stat->engine_msg_stack_ratio)
    {
        mem_stat->engine_msg_stack_ratio = stat;
    }
}

/*******************************************************************************
 * Functions --> Luos tasks find and consume
 ******************************************************************************/

/******************************************************************************
 * @brief Pull a message allocated to a specific service
 * @param target_service : The service concerned by this message
 * @param returned_msg : The message pointer.
 * @return error_return_t
 ******************************************************************************/
error_return_t MsgAlloc_PullMsg(service_t *target_service, msg_t **returned_msg)
{
    //
    //   Pull a message from a specific service
    //
    //   For example, there are 4 messages in buffer and required service is in task 3 :
    //   luos_tasks_stack_id = 3 : function will search in messages 1, 2 & 3
    //
    //
    //        msg_buffer                                 msg_buffer after pull
    //        +------------------------+                +------------------------+
    //        |------------------------|                |------------------------|
    //        +--^---^---^---^---------+                +--^---^---^---^---------+
    //           |   |   |   |                             |   |   |   |
    //   Msg:    1   2   3   4                             1   2   |   4
    //                                                            used_msg
    //                                                            returned_msg
    //
    //
    //             luos_tasks                                luos_tasks
    //             +---------+                               +---------+
    //             |  MSG_1  |\                              |  MSG_1  |
    //             |---------| |                             |---------|
    //             |  MSG_2  | |                             |  MSG_2  |
    //             |---------| |                             |---------|
    //             |  MSG_3  | |                             |  MSG_4  |<-- third message pulled is cleared
    //             |---------| |                             |---------|
    //             |  MSG_4  | |<--luos_tasks_stack_id       |    0    |
    //             |---------| /                             |---------|
    //             |    0    |                               |    0    |
    //             |---------|                               |---------|
    //             |  etc... |                               |  etc... |
    //             +---------+                               +---------+
    //
    // find the oldest message allocated to this service
    for (uint16_t i = 0; i < luos_tasks_stack_id; i++)
    {
        if (luos_tasks[i].service_pt == target_service)
        {
            *returned_msg = luos_tasks[i].msg_pt;
            // Clear the slot by sliding others to the left on it
            used_msg = *returned_msg;
            MsgAlloc_ClearLuosTask(i);
            return SUCCEED;
        }
    }
    // At this point we don't find any message for this service
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |<--luos_tasks_stack_id
    //             |---------|
    //             |  MSG_3  |\_
    //             |---------|  |
    //             |  etc... |  |  <-- search these IDs
    //             |---------|  |  (function return FAILED if ID > luos_tasks_stack_id)
    //             |  Last   | _|
    //             +---------+/
    //
    return FAILED;
}
/******************************************************************************
 * @brief Pull a message allocated to a specific luos task
 * @param luos_task_id : Id of the allocator luos task
 * @param returned_msg : The message pointer.
 * @return error_return_t
 ******************************************************************************/
error_return_t MsgAlloc_PullMsgFromLuosTask(uint16_t luos_task_id, msg_t **returned_msg)
{
    //
    //        msg_buffer                    example : msg_buffer after pulling message D2
    //        +------------------------+        +------------------------+
    //        |------------------------|        |------------------------|
    //        +--^-------^-------^-----+        +--^-------^-------^-----+
    //           |       |       |                 |       |       |
    //   Msg:    1       2  ... LAST               1       2  ... LAST
    //                                                  used_msg
    //                                                 returned_msg
    //
    //             luos_tasks                       luos_tasks
    //             +---------+                      +---------+
    //             |  MSG_1  |                      |  MSG_1  |
    //             |---------|                      |---------|
    //             |  MSG_2  |                      |  MSG_3  |
    //             |---------|                      |---------|
    //             |  MSG_3  |                      |  MSG_4  |
    //             |---------|                      |---------|
    //             |  etc... |                      |  etc... |
    //             |---------|                      |---------|
    //             |   LAST  |                      |    0    |
    //             +---------+                      +---------+
    //
    // find the oldest message allocated to this service
    if (luos_task_id < luos_tasks_stack_id)
    {
        used_msg      = luos_tasks[luos_task_id].msg_pt;
        *returned_msg = (msg_t *)used_msg;
        // Clear the slot by sliding others to the left on it
        MsgAlloc_ClearLuosTask(luos_task_id);
        return SUCCEED;
    }
    // At this point we don't find any message for this service
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |<--luos_tasks_stack_id
    //             |---------|
    //             |  MSG_3  |\_
    //             |---------|  |
    //             |  etc... |  |  <-- search these IDs
    //             |---------|  |  (function return FAILED if ID > luos_tasks_stack_id)
    //             |  Last   | _|
    //             +---------+/
    //
    return FAILED;
}
/******************************************************************************
 * @brief get back the service who received the oldest message
 * @param allocated_service : Return the service concerned by the oldest message
 * @param luos_task_id : Id of the allocator slot
 * @return error_return_t : Fail is there is no more message available.
 ******************************************************************************/
error_return_t MsgAlloc_LookAtLuosTask(uint16_t luos_task_id, service_t **allocated_service)
{
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |<-- if searching this ID : fills service pointer associated to D 2 Luos Task
    //             |---------|
    //             |  MSG_3  |<--luos_tasks_stack_id
    //             |---------|
    //             |    0    |
    //             |---------|
    //             |  etc... |
    //             |---------|
    //             |    0    |
    //             +---------+
    //
    MSGALLOC_MUTEX_LOCK
    if (luos_task_id < luos_tasks_stack_id)
    {
        *allocated_service = luos_tasks[luos_task_id].service_pt;
        MSGALLOC_MUTEX_UNLOCK
        return SUCCEED;
    }
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |<--luos_tasks_stack_id
    //             |---------|
    //             |  MSG_3  |\_
    //             |---------|  |
    //             |  etc... |  |  <-- search these IDs
    //             |---------|  |  (function return FAILED if ID > luos_tasks_stack_id)
    //             |  Last   | _|
    //             +---------+/
    //
    MSGALLOC_MUTEX_UNLOCK
    return FAILED;
}
/******************************************************************************
 * @brief get back a specific slot message command
 * @param luos_task_id : Id of the allocator slot
 * @param cmd : The pointer filled with the cmd value.
 * @return error_return_t : Fail is there is no more message available.
 ******************************************************************************/
error_return_t MsgAlloc_GetLuosTaskCmd(uint16_t luos_task_id, uint8_t *cmd)
{
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  ||
    //             |---------|<--luos_tasks_stack_id : fills CMD header pointer
    //             |  MSG_2  |
    //             |---------|
    //             |  etc... |
    //             |---------|
    //             |   LAST  |
    //             +---------+
    //
    MSGALLOC_MUTEX_LOCK
    if (luos_task_id < luos_tasks_stack_id)
    {
        *cmd = luos_tasks[luos_task_id].msg_pt->header.cmd;
        MSGALLOC_MUTEX_UNLOCK
        return SUCCEED;
    }
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |<--luos_tasks_stack_id
    //             |---------|
    //             |  MSG_3  |\_
    //             |---------|  |
    //             |  etc... |  |  <-- search these IDs
    //             |---------|  |  (function return FAILED if ID > luos_tasks_stack_id)
    //             |  Last   | _|
    //             +---------+/
    //
    MSGALLOC_MUTEX_UNLOCK
    return FAILED;
}
/******************************************************************************
 * @brief get back a specific slot message command
 * @param luos_task_id : Id of the allocator slot
 * @param cmd : The pointer filled with the cmd value.
 * @return error_return_t : Fail is there is no more message available.
 ******************************************************************************/
error_return_t MsgAlloc_GetLuosTaskSourceId(uint16_t luos_task_id, uint16_t *source_id)
{
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|<--luos_tasks_stack_id : fills SOURCE header pointer
    //             |  MSG_2  |
    //             |---------|
    //             |  etc... |
    //             |---------|
    //             |   LAST  |
    //             +---------+
    //
    MSGALLOC_MUTEX_LOCK
    if (luos_task_id < luos_tasks_stack_id)
    {
        *source_id = luos_tasks[luos_task_id].msg_pt->header.source;
        MSGALLOC_MUTEX_UNLOCK
        return SUCCEED;
    }
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |<--luos_tasks_stack_id
    //             |---------|
    //             |  MSG_3  |\_
    //             |---------|  |
    //             |  etc... |  |  <-- search these IDs
    //             |---------|  |  (function return FAILED if ID > luos_tasks_stack_id)
    //             |  Last   | _|
    //             +---------+/
    //
    MSGALLOC_MUTEX_UNLOCK
    return FAILED;
}
/******************************************************************************
 * @brief get back a specific slot message command
 * @param luos_task_id : Id of the allocator slot
 * @param size : The pointer filled with the size value.
 * @return error_return_t : Fail is there is no more message available.
 ******************************************************************************/
error_return_t MsgAlloc_GetLuosTaskSize(uint16_t luos_task_id, uint16_t *size)
{
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|<--luos_tasks_stack_id : fills SIZE header pointer
    //             |  MSG_2  |
    //             |---------|
    //             |  etc... |
    //             |---------|
    //             |   LAST  |
    //             +---------+
    //
    MSGALLOC_MUTEX_LOCK
    if (luos_task_id < luos_tasks_stack_id)
    {
        *size = luos_tasks[luos_task_id].msg_pt->header.size;
        MSGALLOC_MUTEX_UNLOCK
        return SUCCEED;
    }
    //
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |<--luos_tasks_stack_id
    //             |---------|
    //             |  MSG_3  |\_
    //             |---------|  |
    //             |  etc... |  |  <-- search these IDs
    //             |---------|  |  (function return FAILED if ID > luos_tasks_stack_id)
    //             |  Last   | _|
    //             +---------+/
    //
    MSGALLOC_MUTEX_UNLOCK
    return FAILED;
}
/******************************************************************************
 * @brief return the number of allocated messages
 * @param None
 * @return the number of messages
 ******************************************************************************/
uint16_t MsgAlloc_LuosTasksNbr(void)
{
    return (uint16_t)luos_tasks_stack_id;
}
/******************************************************************************
 * @brief Clear a specific message in Luos Tasks
 * @param None
 * @return the number of messages
 ******************************************************************************/
void MsgAlloc_ClearMsgFromLuosTasks(msg_t *msg)
{
    //
    //  Example with message to clean = MSG_2
    //
    //    msg_buffer
    //  +-------------------------------------------------------------+
    //  |--|  1  | 2 |----|     3    |------------ |    X    |--------|
    //  +--^-----^--------^------------------------^------------------+
    //     |     |        |                        |
    //     |     |        |                        |
    //     |     |        |                        |
    //     |     |        | Luos_tasks init state  |      Luos_tasks ending state
    //     |     |        |   +---------+          |        +---------+
    //     +-----|--------|-->|  MSG_1  |          |        |  MSG_1  |
    //           |        |   |---------|          |        |---------|
    //           +--------|-->|  MSG_2  |          |        |  MSG_3  |
    //                    |   |---------|          |        |---------|
    //                    +-->|  MSG_3  |          |        |  etc... |
    //                        |---------|          |        |---------|
    //                        |  etc... |          |        |  Last   |
    //                        |---------|          |        |---------|
    //                        |  Last   |<---------+        |    0    |
    //                        +---------+                   +---------+
    //
    uint16_t id = 0;
    while (id < luos_tasks_stack_id)
    {
        if (luos_tasks[id].msg_pt == msg)
        {
            MsgAlloc_ClearLuosTask(id);
        }
        else
        {
            id++;
        }
    }
    //      If message to clean is not in Luos_tasks : nothing is done
    //
    //        msg_buffer
    //        +-------------------------------------------------------------+
    //        |-------------------------------------------------------------|
    //        +----------^^................^---------------^----------------+
    //                   ||                |               |
    //                   ||   Luos_tasks   |         msg  to clean
    //                   ||  +---------+   |
    //                   +|->|  MSG_1  |   |
    //                    |  |---------|   |
    //                    +->|  MSG_2  |   |
    //                       |---------|   |
    //                       |  etc... |   |
    //                       |---------|   |
    //                       |  Last   |<--+
    //                       +---------+
    //
}
/*******************************************************************************
 * Functions --> Tx tasks create, get and consume
 ******************************************************************************/

/******************************************************************************
 * @brief copy a message to transmit into msg_buffer and create a Tx task
 * @param data to transmit
 * @param size of the data to transmit
 * @return None
 ******************************************************************************/
error_return_t MsgAlloc_SetTxTask(service_t *service_pt, uint8_t *data, uint16_t crc, uint16_t size, luos_localhost_t localhost, uint8_t ack)
{
    LUOS_ASSERT((tx_tasks_stack_id >= 0) && (tx_tasks_stack_id < MAX_MSG_NB) && ((uintptr_t)data > 0));
    void *tx_msg = 0;

    // Then compute if we have space into the TX_message buffer stack
    if (tx_tasks_stack_id >= MAX_MSG_NB - 1)
    {
        return FAILED;
    }
    MSGALLOC_MUTEX_LOCK

    LuosHAL_SetIrqState(false);
    // Alloc the space for the message
    tx_msg = (void *)MsgAlloc_Alloc(size);
    LuosHAL_SetIrqState(true);

    // Copy the tx msg into the buffer
    memcpy((void *)tx_msg, (void *)data, size);

#ifndef VERBOSE_LOCALHOST
    if (localhost != LOCALHOST)
    {
#endif
        // if VERBOSE_LOCALHOST is defined :  Create a tx task to transmit on network for all localhost mode (including LOCALHOST)
        // if VERBOSE_LOCALHOST is NOT defined : create a tx task to transmit on network, except for LOCALHOST
        //
        // Now we are ready to transmit, we can create the tx task
        LuosHAL_SetIrqState(false);
        tx_tasks[tx_tasks_stack_id].size       = size;
        tx_tasks[tx_tasks_stack_id].data_pt    = (uint8_t *)tx_msg;
        tx_tasks[tx_tasks_stack_id].service_pt = service_pt;
        tx_tasks[tx_tasks_stack_id].localhost  = (localhost != EXTERNALHOST);
        tx_tasks_stack_id++;
        LUOS_ASSERT(tx_tasks_stack_id < MAX_MSG_NB);
        LuosHAL_SetIrqState(true);
        // Check if last tx task is the oldest msg of the buffer
        if (tx_tasks_stack_id == 0)
        {
            MsgAlloc_OldestMsgCandidate((msg_t *)tx_tasks[0].data_pt);
        }
#ifndef VERBOSE_LOCALHOST
    }
#endif

    // Write the CRC and the ACK if needed
    if (ack != 0)
    {
        // Write the CRC and Ack
        ((char *)tx_msg)[size - 3] = (uint8_t)(crc);
        ((char *)tx_msg)[size - 2] = (uint8_t)(crc >> 8);
        ((char *)tx_msg)[size - 1] = ack;
    }
    else
    {
        // Write the CRC only
        ((char *)tx_msg)[size - 2] = (uint8_t)(crc);
        ((char *)tx_msg)[size - 1] = (uint8_t)(crc >> 8);
    }
    MSGALLOC_MUTEX_UNLOCK

    // manage localhost (exclude EXTERNALHOST)
    if (localhost != EXTERNALHOST)
    {
        // This is a localhost (LOCALHOST or MULTIHOST) message copy it as a message task
        LUOS_ASSERT(!(msg_tasks_stack_id > 0) || (((uintptr_t)msg_tasks[0] >= (uintptr_t)&msg_buffer[0]) && ((uintptr_t)msg_tasks[0] < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE])));
        MSGALLOC_MUTEX_LOCK
        LuosHAL_SetIrqState(false);
        LUOS_ASSERT(msg_tasks[msg_tasks_stack_id] == 0);
        msg_tasks[msg_tasks_stack_id] = tx_msg;
        LUOS_ASSERT((msg_tasks[msg_tasks_stack_id] != 0));
        msg_tasks_stack_id++;
        LuosHAL_SetIrqState(true);
        MSGALLOC_MUTEX_UNLOCK
    }
    MsgAlloc_FindNewOldestMsg();
    return SUCCEED;
}
/******************************************************************************
 * @brief remove a transmit message task
 * @param None
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void MsgAlloc_PullMsgFromTxTask(void)
{
    if (tx_tasks_stack_id != 0)
    {
        LUOS_ASSERT((tx_tasks_stack_id > 0) && (tx_tasks_stack_id <= MAX_MSG_NB));
        //
        //
        //                      tx_tasks                         tx_tasks                         tx_tasks
        //                     +---------+                      +---------+                      +---------+<--tx_tasks_stack_id = 0
        //                     |   Tx1   |                      |   Tx2   |                      |    0    |
        //                     |---------|                      |---------|                      |---------|
        //                     |   Tx2   |                      |   Tx3   |                      |    0    |
        //                     |---------|                      |---------|                      |---------|
        //                     |   Tx3   |                      |   Tx4   |                      |    0    |
        //                     |---------|                      |---------|                      |---------|
        //                     |  etc... |                      |  etc... |       etc...         |  etc... |
        //                     |---------|                      |---------|                      |---------|
        //                     |  etc... |  tx_tasks_stack_id-->|  etc... |                      |    0    |
        //                     |---------|                      |---------|                      |---------|
        // tx_tasks_stack_id-->|  LAST   |                      |    0    |                      |    0    |
        //                     +---------+                      +---------+                      +---------+
        //
        // Decay tasks
        for (int i = 0; i < tx_tasks_stack_id; i++)
        {
            LuosHAL_SetIrqState(false);
            tx_tasks[i].data_pt = tx_tasks[i + 1].data_pt;
            tx_tasks[i].size    = tx_tasks[i + 1].size;
            LuosHAL_SetIrqState(true);
        }
        LuosHAL_SetIrqState(false);
        if (tx_tasks_stack_id != 0)
        {
            tx_tasks_stack_id--;
            tx_tasks[tx_tasks_stack_id].data_pt = 0;
            tx_tasks[tx_tasks_stack_id].size    = 0;
        }
        LuosHAL_SetIrqState(true);
        MsgAlloc_FindNewOldestMsg();
    }
}
/******************************************************************************
 * @brief remove all transmit task of a specific service
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_PullServiceFromTxTask(uint16_t service_id)
{
    //
    //   Remove a Tx message from a specific service by analyzing "target" in header (for example service is in tx task Tx2)
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
    LUOS_ASSERT((tx_tasks_stack_id > 0) && (tx_tasks_stack_id <= MAX_MSG_NB));
    uint8_t task_id = 0;
    // check all task
    while (task_id < tx_tasks_stack_id)
    {
        if (((msg_t *)tx_tasks[task_id].data_pt)->header.target == service_id)
        {
            // Decay tasks
            for (uint8_t i = task_id; i < tx_tasks_stack_id; i++)
            {
                LuosHAL_SetIrqState(false);
                tx_tasks[i].data_pt = tx_tasks[i + 1].data_pt;
                tx_tasks[i].size    = tx_tasks[i + 1].size;
                LuosHAL_SetIrqState(true);
            }
            LuosHAL_SetIrqState(false);
            if (tx_tasks_stack_id != 0)
            {
                tx_tasks_stack_id--;
                tx_tasks[tx_tasks_stack_id].data_pt = 0;
                tx_tasks[tx_tasks_stack_id].size    = 0;
            }
            LuosHAL_SetIrqState(true);
        }
        else
        {
            task_id++;
        }
    }
    MsgAlloc_FindNewOldestMsg();
}
/******************************************************************************
 * @brief return a message to transmit
 * @param service_pt service sending this data
 * @param data to send
 * @param size of the data to send
 * @param localhost is this message a localhost one
 * @return error_return_t : Fail is there is no more message available.
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL error_return_t MsgAlloc_GetTxTask(service_t **service_pt, uint8_t **data, uint16_t *size, uint8_t *localhost)
{
    LUOS_ASSERT(tx_tasks_stack_id < MAX_MSG_NB);

    //
    // example if luos_tasks_stack_id = 0
    //             luos_tasks
    //             +---------+
    //             |  MSG_1  |
    //             |---------|<--luos_tasks_stack_id  : tx_tasks[0] is filled with pointers (service, data, size & localhost)
    //             |  MSG_2  |
    //             |---------|
    //             |  etc... |
    //             |---------|
    //             |   LAST  |
    //             +---------+
    //
    LuosHAL_SetIrqState(false);
    if (tx_tasks_stack_id > 0)
    {
        *data       = tx_tasks[0].data_pt;
        *size       = tx_tasks[0].size;
        *service_pt = tx_tasks[0].service_pt;
        *localhost  = tx_tasks[0].localhost;
        LuosHAL_SetIrqState(true);
        return SUCCEED;
    }
    LuosHAL_SetIrqState(true);
    //
    //             luos_tasks
    //             +---------+<--luos_tasks_stack_id  (no message, function return FAILED)
    //             |  MSG_1  |
    //             |---------|
    //             |  MSG_2  |
    //             |---------|
    //             |  etc... |
    //             |---------|
    //             |   LAST  |
    //             +---------+
    //
    return FAILED;
}
/******************************************************************************
 * @brief check if there is uncomplete tx_tasks
 * @param None
 * @return error_return_t : Fail is there is untransmitted message.
 ******************************************************************************/
_CRITICAL error_return_t MsgAlloc_TxAllComplete(void)
{
    if (tx_tasks_stack_id > 0)
    {
        return FAILED;
    }
    return SUCCEED;
}
