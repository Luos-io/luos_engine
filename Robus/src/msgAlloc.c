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
 * Alloc_tasks   A   B  msg_pre_tasks        msg_tasks         Luos_tasks
 *  +-------+    |   |   +---------+        +---------+        +---------+
 *  |Alloc A|<---+   +-->|Pre msg B|---C--->|  Msg C  |---D-+->| Task D1 |
 *  |Alloc B|<-------+   |---------|<id     |---------|<id  +->| Task D2 |
 *  |-------|<id         |---------|        |---------|        |---------|<id
 *  |-------|            |---------|        |---------|        |---------|
 *  +-------+            +---------+        +---------+        +---------+
 *
 *  - Event A : This event is called by IT and represent the end of reception of
 *              the header. In this event we get the size of the complete message
 *              so we can check if no validated tasks (msg_tasks or Luos_tasks)
 *              use the memory space needed to receive the complete message.
 *              To do it outside of IT we create an "Alloc A" task. This task
 *              will be executed in alloc_loop.
 *  - Event B : This event is called by IT and represent the end of a good message.
 *              In this event if we have to save the message into a msg_pre_tasks
 *              called "Pre msg B" on this example. Also we have to prepare the
 *              reception of the next header by creating a, "Alloc B" task on
 *              Alloc_tasks.
 *  - Event C : This event represent alloc_loop and it is executed outside of IT.
 *              To begin this event execute Alloc_tasks. Then check if
 *              "Pre msg B" compromise any data on msg_tasks or Luos_tasks.
 *              If it compromise something we have to remove compromised data before
 *              moving the msg as "Msg C".
 *  - Event D : This event represent robus_loop and it is executed outside of IT.
 *              This event pull msg_tasks tasks and interpreat all messages to
 *              create one or more Luos_tasks.
 *
 * After all of it Luos_tasks are ready to be managed by luos_loop execution.
 ******************************************************************************/

#include <string.h>
#include <stdbool.h>
#include "config.h"
#include "msgAlloc.h"
#include "luosHAL.h"

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
    msg_t *msg_pt; /*!< Start pointer of the msg on msg_buffer. */
    vm_t *vm_pt;   /*!< Pointer to the concerned vm. */
} luos_task_t;
/******************************************************************************
 * @struct allocator_task_t
 * @brief store information allowing to do somes opération outside of IRQ
 * 
 * This structure is used to link modules and messages into the allocator.
 * 
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    void *from;
    void *to;
} allocator_task_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

memory_stats_t *mem_stat = NULL;

// msg buffering
volatile uint8_t msg_buffer[MSG_BUFFER_SIZE]; /*!< Memory space used to save and alloc messages. */
volatile msg_t *current_msg;                  /*!< current work in progress msg pointer. */
volatile uint8_t *data_ptr;                   /*!< Pointer to the next data able to be writen into msgbuffer. */

// Allocator task stack
header_t *copy_task_pointer = NULL;                    /*!< This pointer is used to perform a header copy from the end of the msg_buffer to the begin of the msg_buffer. If this pointer if different than NULL there is a copy to make. */
volatile allocator_task_t alloc_tasks[MAX_MSG_NB * 2]; /*!< List of things to do outide of IRQ. */
volatile uint16_t alloc_tasks_stack_id;                /*!< last writen alloc tasks id. */

// msg preparation task stack
volatile msg_t *msg_pre_tasks[MAX_MSG_NB]; /*!< received message table. */
volatile uint16_t msg_pre_tasks_stack_id;  /*!< last writen msg_pre_tasks id. */

// msg interpretation task stack
volatile msg_t *msg_tasks[MAX_MSG_NB]; /*!< ready message table. */
volatile uint16_t msg_tasks_stack_id;  /*!< last writen msg_tasks id. */

// Luos task stack
volatile luos_task_t luos_tasks[MAX_MSG_NB]; /*!< Message allocation table. */
volatile uint16_t luos_tasks_stack_id;       /*!< last writen luos_tasks id. */

/*******************************************************************************
 * Functions
 ******************************************************************************/

// msg buffering
static inline error_return_t MsgAlloc_DoWeHaveSpace(void *to);

// Allocator task stack
static inline void MsgAlloc_CreateMsgSpaceTask(void *from, void *to);
static inline void MsgAlloc_ClearMsgSpaceTask(void);
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
    luos_tasks_stack_id = 0;
    msg_pre_tasks_stack_id = 0;
    msg_tasks_stack_id = 0;
    mem_stat = memory_stats;
}
/******************************************************************************
 * @brief execute some things out of IRQ
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_loop(void)
{
    // Compute memory stats for allocator task memory usage
    uint8_t stat = 0;
    stat = (uint8_t)(((uint32_t)alloc_tasks_stack_id * 100) / (MAX_MSG_NB * 2));
    if (stat > mem_stat->alloc_stack_ratio)
    {
        mem_stat->alloc_stack_ratio = stat;
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
    // Manage memory clear tasks
    while (alloc_tasks_stack_id)
    {
        // manage clear task
        MsgAlloc_ClearMsgSpace(alloc_tasks[0].from, alloc_tasks[0].to);
        // remove clear task
        MsgAlloc_ClearMsgSpaceTask();
    }
    // Manage msg_pre_task
    while (msg_pre_tasks_stack_id)
    {
        // clear the message_space
        MsgAlloc_ClearMsgSpace((void *)msg_pre_tasks[0], (void *)(msg_pre_tasks[0] + sizeof(header_t) + msg_pre_tasks[0]->header.size));

        // move msg_pre_task to msg_task
        if (msg_tasks_stack_id == MAX_MSG_NB)
        {
            // There is no more space on the luos_tasks, remove the oldest msg.
            MsgAlloc_ClearMsgTask();
            if (mem_stat->msg_drop_number < 0xFF)
            {
                mem_stat->msg_drop_number++;
            }
        }
        msg_tasks[msg_tasks_stack_id] = msg_pre_tasks[0];
        msg_tasks_stack_id++;
        // remove the msg_pre_task
        LuosHAL_SetIrqState(FALSE);
        for (int i = 0; i < msg_pre_tasks_stack_id - 1; i++)
        {
            msg_pre_tasks[i] = msg_pre_tasks[i + 1];
        }
        msg_pre_tasks_stack_id--;
        LuosHAL_SetIrqState(TRUE);
    }
    // Compute memory stats for msg task memory usage
    stat = (uint8_t)(((uint32_t)msg_tasks_stack_id * 100) / (MAX_MSG_NB));
    if (stat > mem_stat->msg_stack_ratio)
    {
        mem_stat->msg_stack_ratio = stat;
    }
}

/*******************************************************************************
 * Functions --> msg buffering
 ******************************************************************************/

/******************************************************************************
 * @brief prepare a buffer space to be usable by cleaning remaining messages and prepare pointers
 * @param from : start of the memory space to clean
 * @param to : start of the memory space to clean
 * @return error_return_t
 ******************************************************************************/
static inline error_return_t MsgAlloc_DoWeHaveSpace(void *to)
{
    if ((uint32_t)to > ((uint32_t)&msg_buffer[MSG_BUFFER_SIZE - 1]))
    {
        // We reach msg_buffer end return an error
        return FAIL;
    }
    return SUCESS;
}
/******************************************************************************
 * @brief Invalid the current message header by removing it (data will be ignored).
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_InvalidMsg(void)
{
    //******** Remove the header by reseting data_ptr *********
    data_ptr = (uint8_t *)current_msg;
}
/******************************************************************************
 * @brief Valid the current message header by preparing the allocator to get the message data
 * @param concerned_module : The module concerned by this message
 * @return None
 ******************************************************************************/
void MsgAlloc_ValidHeader(void)
{
//******** Prepare the allocator to get data  *********
// Save the concerned module pointer into the concerned module pointer stack
#ifdef DEBUG
    if (module_concerned_stack_pointer >= MAX_VM_NUMBER)
    {
        while (1)
            ;
    }
#endif
    // Be sure that the end of msg_buffer is after data_ptr + header_t.size
    if (MsgAlloc_DoWeHaveSpace((void *)(&current_msg->data[current_msg->header.size + 2])) == FAIL)
    {
        // We are at the end of msg_buffer, we need to move the current space to the begin of msg_buffer
        // Create a task to clean the begin of msg_buffer with the space of the complete message
        MsgAlloc_CreateMsgSpaceTask((void *)&msg_buffer[0], (void *)(&msg_buffer[0] + sizeof(header_t) + current_msg->header.size + 2));
        // Create a task to copy the header at the begining of msg_buffer
        copy_task_pointer = (header_t *)&current_msg->header;
        // Move current_msg to msg_buffer
        current_msg = (volatile msg_t *)&msg_buffer[0];
        // move data_ptr after the new location of the header
        data_ptr = &msg_buffer[0] + sizeof(header_t);
    }
    else
    {
        // We have space in msg_buffer, create a task to clean the memory zone
        MsgAlloc_CreateMsgSpaceTask((void *)current_msg, (void *)(&current_msg->data[current_msg->header.size + 2]));
    }
}
/******************************************************************************
 * @brief Finish the current message
 * @param module_concerned_by_current_msg list of concerned modules
 * @param module_concerned_stack_pointer Number of module concerned
 * @return None
 ******************************************************************************/
void MsgAlloc_EndMsg(void)
{
    //******** End the message **********
    // Store the received message in pre_task
    if (msg_pre_tasks_stack_id == MAX_MSG_NB)
    {
        // There is no more space on the luos_pre_tasks, remove the oldest msg.
        for (uint16_t rm = 0; rm < msg_pre_tasks_stack_id; rm++)
        {
            msg_pre_tasks[rm] = msg_pre_tasks[rm + 1];
        }
        msg_pre_tasks_stack_id--;
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
        }
    }
    msg_pre_tasks[msg_pre_tasks_stack_id] = current_msg;
    msg_pre_tasks_stack_id++;
    //******** Prepare the next msg *********
    //data_ptr is actually 2 bytes after the message data because of the CRC. Remove the CRC.
    data_ptr -= 2;
    // clean space between data_ptr (data_ptr + sizeof(header_t)+2)
    if (MsgAlloc_DoWeHaveSpace((void *)(data_ptr + sizeof(header_t) + 2)) == FAIL)
    {
        data_ptr = &msg_buffer[0];
    }
    // update the current_msg
    current_msg = (volatile msg_t *)data_ptr;
    // create a task to clear this space
    MsgAlloc_CreateMsgSpaceTask((void *)current_msg, (void *)(current_msg + sizeof(header_t) + 2));
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
 * @brief return the current message
 * @param None
 * @return msg_t* pointer to the current message
 ******************************************************************************/
msg_t *MsgAlloc_GetCurrentMsg(void)
{
    return (msg_t *)current_msg;
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
    if ((uint32_t)to > ((uint32_t)&msg_buffer[MSG_BUFFER_SIZE - 1]))
    {
        // We reach msg_buffer end return an error
        return FAIL;
    }
    //******** Prepare a memory space to be writable **********
    // check if there is no msg between from and to on luos_tasks
    while (((uint32_t)luos_tasks[0].msg_pt >= (uint32_t)from) & ((uint32_t)luos_tasks[0].msg_pt <= (uint32_t)to))
    {
        // This message is in the space we want to use, clear the task
        MsgAlloc_ClearLuosTask(0);
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
        }
    }
    // check if there is no msg between from and to on msg_tasks
    while (((uint32_t)msg_tasks[0] >= (uint32_t)from) & ((uint32_t)msg_tasks[0] <= (uint32_t)to))
    {
        // This message is in the space we want to use, clear the task
        MsgAlloc_ClearMsgTask();
        if (mem_stat->msg_drop_number < 0xFF)
        {
            mem_stat->msg_drop_number++;
        }
    }
    // if we go here there is no reason to continue because newest messages can't overlap the memory zone.
    return SUCESS;
}
/******************************************************************************
 * @brief prepare a task to clear a memory space
 * @param from : start of the memory space to clean
 * @param to : start of the memory space to clean
 * @return None
 ******************************************************************************/
static inline void MsgAlloc_CreateMsgSpaceTask(void *from, void *to)
{
    if (alloc_tasks_stack_id >= (MAX_MSG_NB - 1))
    {
        // we are out of buffer, remove a task
        MsgAlloc_ClearMsgSpaceTask();
    }
    alloc_tasks[alloc_tasks_stack_id].from = from;
    alloc_tasks[alloc_tasks_stack_id].to = to;
    alloc_tasks_stack_id++;
}
static inline void MsgAlloc_ClearMsgSpaceTask(void)
{
    // remove clear task
    LuosHAL_SetIrqState(FALSE);
    for (int i = 0; i < alloc_tasks_stack_id - 1; i++)
    {
        alloc_tasks[i] = alloc_tasks[i + 1];
    }
    alloc_tasks_stack_id--;
    LuosHAL_SetIrqState(TRUE);
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
    for (uint16_t rm = 0; rm < msg_tasks_stack_id; rm++)
    {
        msg_tasks[rm] = msg_tasks[rm + 1];
    }
    msg_tasks_stack_id--;
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
        LuosHAL_SetIrqState(FALSE);
        MsgAlloc_ClearMsgTask();
        LuosHAL_SetIrqState(TRUE);
        return SUCESS;
    }
    // At this point we don't find any message for this module
    return FAIL;
}

/*******************************************************************************
 * Functions --> Luos task stack
 ******************************************************************************/

/******************************************************************************
 * @brief Clear a slot. This action is due to an error
 * @param None
 * @return None
 ******************************************************************************/
static inline void MsgAlloc_ClearLuosTask(uint16_t luos_task_id)
{
    if (luos_task_id > luos_tasks_stack_id)
    {
        return;
    }
    for (uint16_t rm = luos_task_id; rm < luos_tasks_stack_id; rm++)
    {
        luos_tasks[rm] = luos_tasks[rm + 1];
    }
    luos_tasks_stack_id--;
}
/******************************************************************************
 * @brief Alloc luos task
 * @param module_concerned_by_current_msg concerned modules
 * @param module_concerned_by_current_msg concerned msg
 * @return None
 ******************************************************************************/
void MsgAlloc_LuosTaskAlloc(vm_t *module_concerned_by_current_msg, msg_t *concerned_msg)
{
    // find a free slot
    if (luos_tasks_stack_id == MAX_MSG_NB)
    {
        // There is no more space on the luos_tasks, remove the oldest msg.
        MsgAlloc_ClearLuosTask(0);
    }
    // fill the informations of the message in this slot
    luos_tasks[luos_tasks_stack_id].msg_pt = concerned_msg;
    luos_tasks[luos_tasks_stack_id].vm_pt = module_concerned_by_current_msg;
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
error_return_t MsgAlloc_PullMsg(vm_t *target_module, msg_t **returned_msg)
{
    //find the oldest message allocated to this module
    for (uint16_t i = 0; i < luos_tasks_stack_id; i++)
    {
        if (luos_tasks[i].vm_pt == target_module)
        {
            *returned_msg = luos_tasks[i].msg_pt;
            LuosHAL_SetIrqState(FALSE);
            // Clear the slot by sliding others to the left on it
            for (uint16_t rm = i; rm < luos_tasks_stack_id; rm++)
            {
                luos_tasks[rm] = luos_tasks[rm + 1];
            }
            luos_tasks_stack_id--;
            LuosHAL_SetIrqState(TRUE);
            return SUCESS;
        }
    }
    // At this point we don't find any message for this module
    return FAIL;
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
        LuosHAL_SetIrqState(FALSE);
        // Clear the slot by sliding others to the left on it
        for (uint16_t rm = luos_task_id; rm < luos_tasks_stack_id; rm++)
        {
            luos_tasks[rm] = luos_tasks[rm + 1];
        }
        luos_tasks_stack_id--;
        LuosHAL_SetIrqState(TRUE);
        return SUCESS;
    }
    // At this point we don't find any message for this module
    return FAIL;
}
/******************************************************************************
 * @brief get back the module who received the oldest message 
 * @param allocated_module : Return the module concerned by the oldest message
 * @param luos_task_id : Id of the allocator slot
 * @return error_return_t : Fail is there is no more message available.
 ******************************************************************************/
error_return_t MsgAlloc_LookAtLuosTask(uint16_t luos_task_id, vm_t **allocated_module)
{
    if (luos_task_id < luos_tasks_stack_id)
    {
        *allocated_module = luos_tasks[luos_task_id].vm_pt;
        return SUCESS;
    }
    return FAIL;
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
        return SUCESS;
    }
    return FAIL;
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
        return SUCESS;
    }
    return FAIL;
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
        return SUCESS;
    }
    return FAIL;
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