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
 *  - Event C : This event represent robus_loop and it is executed outside of IT.
 *              This event pull msg_tasks tasks and interpret all messages to
 *              create one or more Luos_tasks.
 *  - Task D  : This is all msg trait by Luos Library interpret in Luos_loop. Msg can be
 *              for Luos Library or for container. this is executed outside of IT.
 *  - Task E  : Msg_buffer can also save some TX tasks and list them into tx_task tasks
 *
 * After all of it Luos_tasks are ready to be managed by luos_loop execution.
 ******************************************************************************/

#include <string.h>
#include <stdbool.h>
#include "config.h"
#include "msg_alloc.h"
#include "luos_hal.h"
#include "luos_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct luos_task_t
 * @brief Message allocator loger structure.
 *
 * This structure is used to link modules and messages into the allocator.
 *
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    msg_t *msg_pt;                   /*!< Start pointer of the msg on msg_buffer. */
    ll_container_t *ll_container_pt; /*!< Pointer to the concerned ll_container. */
} luos_task_t;

typedef struct
{
    char *data_pt; /*!< Start pointer of the data on msg_buffer. */
    uint16_t size; /*!< size of the data. */
} tx_task_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/
memory_stats_t *mem_stat = NULL;

// msg buffering
volatile uint8_t msg_buffer[MSG_BUFFER_SIZE]; /*!< Memory space used to save and alloc messages. */
volatile msg_t *current_msg;                  /*!< current work in progress msg pointer. */
volatile uint8_t *data_ptr;                   /*!< Pointer to the next data able to be writen into msgbuffer. */
volatile uint8_t *data_end_estimation;        /*!< Estimated end of the current receiving message. */
volatile msg_t *used_msg = NULL;              /*!< Message curently used by luos loop. */

// Allocator task stack
volatile header_t *copy_task_pointer = NULL; /*!< This pointer is used to perform a header copy from the end of the msg_buffer to the begin of the msg_buffer. If this pointer if different than NULL there is a copy to make. */

// msg interpretation task stack
volatile msg_t *msg_tasks[MAX_MSG_NB]; /*!< ready message table. */
volatile uint16_t msg_tasks_stack_id;  /*!< last writen msg_tasks id. */

// Luos task stack
volatile luos_task_t luos_tasks[MAX_MSG_NB]; /*!< Message allocation table. */
volatile uint16_t luos_tasks_stack_id;       /*!< last writen luos_tasks id. */

// Tx task stack
volatile tx_task_t tx_tasks[MAX_MSG_NB]; /*!< Message to transmit allocation table. */
volatile uint16_t tx_tasks_stack_id;     /*!< last writen tx_tasks id. */

/*******************************************************************************
 * Functions
 ******************************************************************************/

// msg buffering
static inline error_return_t MsgAlloc_DoWeHaveSpace(void *to);

// Allocator task stack
static inline error_return_t MsgAlloc_ClearMsgSpace(void *from, void *to);

// msg interpretation task stack
static inline void MsgAlloc_ClearMsgTask(void);

// Luos task stack
static inline void MsgAlloc_ClearLuosTask(uint16_t luos_task_id);

/*******************************************************************************
 * Functions --> generic
 ******************************************************************************/

/******************************************************************************
 * @brief Init the allocator.
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_Init(memory_stats_t *memory_stats)
{
    //******** Init global vars pointers **********
    current_msg = (msg_t *)&msg_buffer[0];
    data_ptr = (uint8_t *)&msg_buffer[0];
    data_end_estimation = (uint8_t *)&current_msg->data[2];
    msg_tasks_stack_id = 0;
    memset((void *)msg_tasks, 0, sizeof(msg_tasks));
    luos_tasks_stack_id = 0;
    memset((void *)luos_tasks, 0, sizeof(luos_tasks));
    tx_tasks_stack_id = 0;
    memset((void *)tx_tasks, 0, sizeof(tx_tasks));
    copy_task_pointer = NULL;
    used_msg = NULL;
    if (memory_stats != NULL)
    {
        mem_stat = memory_stats;
    }
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
    // Compute memory stats for msg task memory usage
    stat = (uint8_t)(((uint32_t)msg_tasks_stack_id * 100) / (MAX_MSG_NB));
    if (stat > mem_stat->msg_stack_ratio)
    {
        mem_stat->msg_stack_ratio = stat;
    }
    // Check if we have to make a header copy from the end to the begin of msg_buffer.
    if (copy_task_pointer != NULL)
    {
        // copy_task_pointer point to a header to copy at the begin of msg_buffer
        // Copy the header at the begining of msg_buffer
        memcpy((void *)&msg_buffer[0], (void *)copy_task_pointer, sizeof(header_t));
        // reset copy_task_pointer status
        copy_task_pointer = NULL;
    }
}

/*******************************************************************************
 * Functions --> msg buffering
 ******************************************************************************/

/******************************************************************************
 * @brief check if there is enought space to store this data into buffer
 * @param from : start of the memory space to clean
 * @param to : start of the memory space to clean
 * @return error_return_t
 ******************************************************************************/
static inline error_return_t MsgAlloc_DoWeHaveSpace(void *to)
{
    if ((uint32_t)to > ((uint32_t)&msg_buffer[MSG_BUFFER_SIZE - 1]))
    {
        // We reach msg_buffer end return an error
        return FAILED;
    }
    return SUCCEED;
}
/******************************************************************************
 * @brief Invalid the current message header by removing it (data will be ignored).
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_InvalidMsg(void)
{
    //******** Remove the header by reseting data_ptr *********
    //clean the memory zone
    MsgAlloc_ClearMsgSpace((void *)current_msg, (void *)(data_ptr));
    data_ptr = (uint8_t *)current_msg;
    data_end_estimation = (uint8_t *)&current_msg->data[2];
    if (current_msg == (volatile msg_t *)&msg_buffer[0])
    {
        copy_task_pointer = NULL;
    }
}
/******************************************************************************
 * @brief Valid the current message header by preparing the allocator to get the message data
 * @param valid : is the header valid or not
 * @param data_size : size of the data to receive
 * @return None
 ******************************************************************************/
void MsgAlloc_ValidHeader(uint8_t valid, uint16_t data_size)
{
    //******** Prepare the allocator to get data  *********
    // Save the concerned module pointer into the concerned module pointer stack
    if (valid == true)
    {
        if (MsgAlloc_DoWeHaveSpace((void *)(&current_msg->data[data_size + 2])) == FAILED)
        {
            // We are at the end of msg_buffer, we need to move the current space to the begin of msg_buffer
            // Create a task to copy the header at the begining of msg_buffer
            copy_task_pointer = (header_t *)&current_msg->header;
            // Move current_msg to msg_buffer
            current_msg = (volatile msg_t *)&msg_buffer[0];
            // move data_ptr after the new location of the header
            data_ptr = &msg_buffer[sizeof(header_t)];
        }
        // Save the end position of our message
        data_end_estimation = (uint8_t *)&current_msg->data[data_size + 2];
        // check if there is a msg treatment pending
        if (((uint32_t)used_msg >= (uint32_t)current_msg) && ((uint32_t)used_msg <= (uint32_t)(&current_msg->data[data_size + 2])))
        {
            used_msg = NULL;
            // This message is in the space we want to use, clear the task
            if (mem_stat->msg_drop_number < 0xFF)
            {
                mem_stat->msg_drop_number++;
                mem_stat->buffer_occupation_ratio = 100;
            }
        }
    }
    else
    {
        data_ptr = (uint8_t *)current_msg;
    }
}
/******************************************************************************
 * @brief Finish the current message
 * @return None
 ******************************************************************************/
void MsgAlloc_EndMsg(void)
{
    //******** End the message **********
    //clean the memory zone
    MsgAlloc_ClearMsgSpace((void *)current_msg, (void *)data_ptr);

    // Store the received message
    if (msg_tasks_stack_id == MAX_MSG_NB)
    {
        // There is no more space on the msg_tasks, remove the oldest msg.
        MsgAlloc_ClearMsgTask();
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
            mem_stat->rx_msg_stack_ratio = 100;
        }
    }
    LUOS_ASSERT(msg_tasks[msg_tasks_stack_id] == 0);
    LUOS_ASSERT(!(msg_tasks_stack_id > 0) || (((uint32_t)msg_tasks[0] >= (uint32_t)&msg_buffer[0]) && ((uint32_t)msg_tasks[0] < (uint32_t)&msg_buffer[MSG_BUFFER_SIZE])));
    msg_tasks[msg_tasks_stack_id] = current_msg;
    msg_tasks_stack_id++;
    //******** Prepare the next msg *********
    //data_ptr is actually 2 bytes after the message data because of the CRC. Remove the CRC.
    data_ptr -= 2;
    // Check data ptr alignement
    if (*data_ptr % 2 != 1)
    {
        data_ptr++;
    }
    // Check if we have space for the next message
    if (MsgAlloc_DoWeHaveSpace((void *)(data_ptr + sizeof(header_t) + 2)) == FAILED)
    {
        data_ptr = &msg_buffer[0];
    }
    // update the current_msg
    current_msg = (volatile msg_t *)data_ptr;
    // Save the estimated end of the next message
    data_end_estimation = (uint8_t *)&current_msg->data[2];
    // create a task to clear this space
    MsgAlloc_ClearMsgSpace((void *)current_msg, (void *)(&current_msg->stream[sizeof(header_t) + 2]));
}
/******************************************************************************
 * @brief write a byte into the current message.
 * @param uint8_t data to write in the allocator
 * @return None
 ******************************************************************************/
void MsgAlloc_SetData(uint8_t data)
{
    //******** Write data  *********
    *data_ptr = data;
    data_ptr++;
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

/*******************************************************************************
 * Functions --> Allocator task stack
 ******************************************************************************/

/******************************************************************************
 * @brief prepare a buffer space to be usable by cleaning remaining messages and prepare pointers
 * @param from : start of the memory space to clean
 * @param to : start of the memory space to clean
 * @return error_return_t
 ******************************************************************************/
static inline error_return_t MsgAlloc_ClearMsgSpace(void *from, void *to)
{
    //******** Check if there is sufficient space on the buffer **********
    if (MsgAlloc_DoWeHaveSpace(to) == FAILED)
    {
        // We reach msg_buffer end return an error
        return FAILED;
    }
    //******** Prepare a memory space to be writable **********

    // check if there is a msg traitement pending
    if (((uint32_t)used_msg >= (uint32_t)from) && ((uint32_t)used_msg <= (uint32_t)to))
    {
        used_msg = NULL;
        // This message is in the space we want to use, clear the task
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
        }
    }
    while (((uint32_t)luos_tasks[0].msg_pt >= (uint32_t)from) && ((uint32_t)luos_tasks[0].msg_pt <= (uint32_t)to) && (luos_tasks_stack_id > 0))
    {
        // This message is in the space we want to use, clear the task
        MsgAlloc_ClearLuosTask(0);
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
        }
    }
    // check if there is no msg between from and to on msg_tasks
    while (((uint32_t)msg_tasks[0] >= (uint32_t)from) && ((uint32_t)msg_tasks[0] <= (uint32_t)to) && (msg_tasks_stack_id > 0))
    {
        // This message is in the space we want to use, clear the task
        MsgAlloc_ClearMsgTask();
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
        }
    }
    // check if there is no msg between from and to on tx_tasks
    while (((uint32_t)tx_tasks[0].data_pt >= (uint32_t)from) && ((uint32_t)tx_tasks[0].data_pt <= (uint32_t)to) && (tx_tasks_stack_id > 0))
    {
        // This message is in the space we want to use, clear the task
        MsgAlloc_PullMsgFromTxTask();
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
        }
    }
    // if we go here there is no reason to continue because newest messages can't overlap the memory zone.
    return SUCCEED;
}
/*******************************************************************************
 * Functions --> msg interpretation task stack
 ******************************************************************************/

/******************************************************************************
 * @brief Clear a slot. This action is due to an error
 * @param None
 * @return None
 ******************************************************************************/
static inline void MsgAlloc_ClearMsgTask(void)
{
    LUOS_ASSERT((msg_tasks_stack_id <= MAX_MSG_NB) && (msg_tasks_stack_id > 0));

    for (uint16_t rm = 0; rm < msg_tasks_stack_id; rm++)
    {
        LuosHAL_SetIrqState(true);
        LuosHAL_SetIrqState(false);
        msg_tasks[rm] = msg_tasks[rm + 1];
    }
    msg_tasks_stack_id--;
    msg_tasks[msg_tasks_stack_id] = 0;
    LuosHAL_SetIrqState(true);
}
/******************************************************************************
 * @brief Pull a message that is not interpreted by robus yet
 * @param returned_msg : The message pointer.
 * @return error_return_t
 ******************************************************************************/
error_return_t MsgAlloc_PullMsgToInterpret(msg_t **returned_msg)
{
    if (msg_tasks_stack_id > 0)
    {
        *returned_msg = (msg_t *)msg_tasks[0];
        LUOS_ASSERT(((uint32_t)*returned_msg >= (uint32_t)&msg_buffer[0]) && ((uint32_t)*returned_msg < (uint32_t)&msg_buffer[MSG_BUFFER_SIZE]));
        MsgAlloc_ClearMsgTask();
        return SUCCEED;
    }
    // At this point we don't find any message for this module
    return FAILED;
}

/*******************************************************************************
 * Functions --> Luos task stack
 ******************************************************************************/

/******************************************************************************
 * @brief Notify the end of the usage of the message.
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
static inline void MsgAlloc_ClearLuosTask(uint16_t luos_task_id)
{
    LUOS_ASSERT((luos_task_id <= luos_tasks_stack_id) || (luos_tasks_stack_id <= MAX_MSG_NB));
    for (uint16_t rm = luos_task_id; rm < luos_tasks_stack_id; rm++)
    {
        luos_tasks[rm] = luos_tasks[rm + 1];
    }
    LuosHAL_SetIrqState(false);
    if (luos_tasks_stack_id != 0)
    {
        luos_tasks_stack_id--;
    }
    LuosHAL_SetIrqState(true);
}
/******************************************************************************
 * @brief Alloc luos task
 * @param module_concerned_by_current_msg concerned modules
 * @param module_concerned_by_current_msg concerned msg
 * @return None
 ******************************************************************************/
void MsgAlloc_LuosTaskAlloc(ll_container_t *container_concerned_by_current_msg, msg_t *concerned_msg)
{
    // find a free slot
    if (luos_tasks_stack_id == MAX_MSG_NB)
    {
        // There is no more space on the luos_tasks, remove the oldest msg.
        MsgAlloc_ClearLuosTask(0);
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
            mem_stat->luos_stack_ratio = 100;
        }
    }
    // fill the informations of the message in this slot
    luos_tasks[luos_tasks_stack_id].msg_pt = concerned_msg;
    luos_tasks[luos_tasks_stack_id].ll_container_pt = container_concerned_by_current_msg;
    luos_tasks_stack_id++;
    // luos task memory usage
    uint8_t stat = (uint8_t)(((uint32_t)luos_tasks_stack_id * 100) / (MAX_MSG_NB));
    if (stat > mem_stat->luos_stack_ratio)
    {
        mem_stat->luos_stack_ratio = stat;
    }
}

/*******************************************************************************
 * Functions --> Luos tasks find and consume
 ******************************************************************************/

/******************************************************************************
 * @brief Pull a message allocated to a specific module
 * @param target_module : The module concerned by this message
 * @param returned_msg : The message pointer.
 * @return error_return_t
 ******************************************************************************/
error_return_t MsgAlloc_PullMsg(ll_container_t *target_module, msg_t **returned_msg)
{
    //find the oldest message allocated to this module
    for (uint16_t i = 0; i < luos_tasks_stack_id; i++)
    {
        if (luos_tasks[i].ll_container_pt == target_module)
        {
            *returned_msg = luos_tasks[i].msg_pt;

            // Clear the slot by sliding others to the left on it
            used_msg = *returned_msg;
            MsgAlloc_ClearLuosTask(i);
            return SUCCEED;
        }
    }
    // At this point we don't find any message for this module
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
    //find the oldest message allocated to this module
    if (luos_task_id < luos_tasks_stack_id)
    {
        *returned_msg = luos_tasks[luos_task_id].msg_pt;

        // Clear the slot by sliding others to the left on it
        used_msg = *returned_msg;
        MsgAlloc_ClearLuosTask(luos_task_id);
        return SUCCEED;
    }
    // At this point we don't find any message for this module
    return FAILED;
}
/******************************************************************************
 * @brief get back the module who received the oldest message
 * @param allocated_module : Return the module concerned by the oldest message
 * @param luos_task_id : Id of the allocator slot
 * @return error_return_t : Fail is there is no more message available.
 ******************************************************************************/
error_return_t MsgAlloc_LookAtLuosTask(uint16_t luos_task_id, ll_container_t **allocated_module)
{
    if (luos_task_id < luos_tasks_stack_id)
    {
        *allocated_module = luos_tasks[luos_task_id].ll_container_pt;
        return SUCCEED;
    }
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
    if (luos_task_id < luos_tasks_stack_id)
    {
        *cmd = luos_tasks[luos_task_id].msg_pt->header.cmd;
        return SUCCEED;
    }
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
    if (luos_task_id < luos_tasks_stack_id)
    {
        *source_id = luos_tasks[luos_task_id].msg_pt->header.source;
        return SUCCEED;
    }
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
    if (luos_task_id < luos_tasks_stack_id)
    {
        *size = luos_tasks[luos_task_id].msg_pt->header.size;
        return SUCCEED;
    }
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
 * @brief return the number of allocated messages
 * @param None
 * @return the number of messages
 ******************************************************************************/
void MsgAlloc_ClearMsgFromLuosTasks(msg_t *msg)
{
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
}

/*******************************************************************************
 * Functions --> Tx tasks create, get and consume
 ******************************************************************************/

/******************************************************************************
 * @brief copy a message to transmit into msg_buffer and create a Tx task
 * @param data to transmit
 * @param size of the data to transmit
 ******************************************************************************/
void MsgAlloc_SetTxTask(char *data, uint16_t size, uint8_t locahost)
{
    LUOS_ASSERT((tx_tasks_stack_id >= 0) && (tx_tasks_stack_id < MAX_MSG_NB) && ((uint32_t)data > 0) && ((uint32_t)current_msg < (uint32_t)&msg_buffer[MSG_BUFFER_SIZE]) && ((uint32_t)current_msg >= (uint32_t)&msg_buffer[0]));
    void *rx_msg_bkp = 0;
    void *tx_msg = 0;
    uint16_t progression_size = 0;
    uint16_t estimated_size = 0;
    // Stop it
    LuosHAL_SetIrqState(false);
    // compute RX progression
    progression_size = (uint32_t)data_ptr - (uint32_t)current_msg;
    estimated_size = (uint32_t)data_end_estimation - (uint32_t)current_msg;
    rx_msg_bkp = (void *)current_msg;
    // Check if the message to send size fit into msg buffer
    if (MsgAlloc_DoWeHaveSpace((void *)((uint32_t)current_msg + size)) == FAILED)
    {
        // message to send don't fit move everything at the begining of the buffer
        tx_msg = (msg_t *)msg_buffer;
        current_msg = (msg_t *)((uint32_t)msg_buffer + size);
        data_ptr = (uint8_t *)((uint32_t)current_msg + progression_size);
        data_end_estimation = (uint8_t *)((uint32_t)current_msg + estimated_size);
        MsgAlloc_ClearMsgSpace((void *)tx_msg, (void *)data_end_estimation);
    }
    else
    {
        // Message to send fit
        tx_msg = (void *)current_msg;
        // Check if the receiving message size fit into msg buffer
        if (MsgAlloc_DoWeHaveSpace((void *)((uint32_t)current_msg + size + estimated_size)) == FAILED)
        {
            // receiving message don't fit, move it to the start of the buffer
            MsgAlloc_ClearMsgSpace((void *)tx_msg, (void *)((uint32_t)tx_msg + size));
            current_msg = (msg_t *)msg_buffer;
            data_end_estimation = (uint8_t *)((uint32_t)current_msg + estimated_size);
            MsgAlloc_ClearMsgSpace((void *)current_msg, (void *)data_end_estimation);
        }
        else
        {
            // receiving message fit, move receiving message of tx_message size
            current_msg = (msg_t *)((uint32_t)current_msg + size);
            data_end_estimation = (uint8_t *)((uint32_t)current_msg + estimated_size);
            MsgAlloc_ClearMsgSpace((void *)tx_msg, (void *)data_end_estimation);
        }
        data_ptr = (uint8_t *)((uint32_t)current_msg + progression_size);
        LUOS_ASSERT((uint32_t)(data_ptr) < (uint32_t)(&msg_buffer[MSG_BUFFER_SIZE]));
    }
    void *current_msg_cpy = (void *)current_msg;
    // Copy previously received header parts
    if (progression_size >= sizeof(header_t))
    {
        // We already receive more than a header
        // Copy the header before reenabling IRQ
        memcpy((void *)current_msg_cpy, rx_msg_bkp, sizeof(header_t));
        // re-enable IRQ
        LuosHAL_SetIrqState(true);
        // Now we can copy additional datas
        memcpy((void *)((uint32_t)current_msg_cpy + sizeof(header_t)), (void *)((uint32_t)rx_msg_bkp + sizeof(header_t)), (progression_size - sizeof(header_t)));
    }
    else
    {
        // We receive less than a header
        // Copy previously received datas
        memcpy((void *)current_msg_cpy, rx_msg_bkp, progression_size);
        // re-enable IRQ
        LuosHAL_SetIrqState(true);
    }

    // Copy 3 bytes from the message to transmit just to be sure to be ready to start transmitting
    // During those 3 bytes we have the time necessary to copy the other bytes
    memcpy((void *)tx_msg, (void *)data, 3);
    // Now we are ready to transmit, we can create the tx task

    LuosHAL_SetIrqState(false);
    tx_tasks[tx_tasks_stack_id].size = size;
    tx_tasks[tx_tasks_stack_id].data_pt = (char *)tx_msg;
    tx_tasks_stack_id++;
    if (tx_tasks_stack_id == MAX_MSG_NB)
    {
        MsgAlloc_PullMsgFromTxTask();
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
            mem_stat->tx_msg_stack_ratio = 100;
        }
    }
    LuosHAL_SetIrqState(true);
    // Finish the copy of the message to transmit
    memcpy((void *)&((char *)tx_msg)[3], (void *)&data[3], size - 3);
    // The copy of the message is finish, manage localhost
    if (locahost)
    {
        // This is a localhost message copy it as a message task
        LUOS_ASSERT(msg_tasks[msg_tasks_stack_id] == 0);
        LUOS_ASSERT(!(msg_tasks_stack_id > 0) || (((uint32_t)msg_tasks[0] >= (uint32_t)&msg_buffer[0]) && ((uint32_t)msg_tasks[0] < (uint32_t)&msg_buffer[MSG_BUFFER_SIZE])));
        msg_tasks[msg_tasks_stack_id] = tx_msg;
        msg_tasks_stack_id++;
    }
}
/******************************************************************************
 * @brief remove a transmit message task
 * @param None
 ******************************************************************************/
void MsgAlloc_PullMsgFromTxTask(void)
{
    LUOS_ASSERT((tx_tasks_stack_id > 0) && (tx_tasks_stack_id <= MAX_MSG_NB));
    // Decay tasks
    for (int i = 0; i < tx_tasks_stack_id; i++)
    {
        LuosHAL_SetIrqState(false);
        tx_tasks[i].data_pt = tx_tasks[i + 1].data_pt;
        tx_tasks[i].size = tx_tasks[i + 1].size;
        LuosHAL_SetIrqState(true);
    }
    LuosHAL_SetIrqState(false);
    if (tx_tasks_stack_id != 0)
    {
        tx_tasks_stack_id--;
        tx_tasks[tx_tasks_stack_id].data_pt = 0;
        tx_tasks[tx_tasks_stack_id].size = 0;
    }
    LuosHAL_SetIrqState(true);
}
/******************************************************************************
 * @brief return a message to transmit
 * @param data to send
 * @param size of the data to send
 * @return error_return_t : Fail is there is no more message available.
 ******************************************************************************/
error_return_t MsgAlloc_GetTxTask(char **data, uint16_t *size)
{
    LUOS_ASSERT((tx_tasks_stack_id >= 0) && (tx_tasks_stack_id < MAX_MSG_NB));
    if (tx_tasks_stack_id > 0)
    {
        *data = tx_tasks[0].data_pt;
        *size = tx_tasks[0].size;
        return SUCCEED;
    }
    return FAILED;
}
