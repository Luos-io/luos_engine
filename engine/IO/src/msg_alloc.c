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
#include "msg_alloc.h"
#include "luos_phy.h"
#include "luos_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    uint8_t *data;      // Pointer to the first byte of the message.
    uint8_t phy_filter; // Physical filter of the message.
} alloc_slot_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
memory_stats_t *mem_stat = NULL;

// msg buffering
volatile uint8_t msg_buffer[MSG_BUFFER_SIZE]; /*!< Memory space used to save and alloc messages. */
volatile uint8_t *data_ptr;                   /*!< Pointer to the next data able to be written into msgbuffer. */

alloc_slot_t alloc_slots[MAX_MSG_NB];   /*!< Slots used to save the index of the first byte of a message. */
volatile uint16_t oldest_alloc_slot;    // Index of the oldest allocation.
volatile uint16_t available_alloc_slot; // Index of the next available allocation slot.

/*******************************************************************************
 * Functions
 ******************************************************************************/

// msg buffering
_CRITICAL static inline error_return_t MsgAlloc_DoWeHaveSpaceUntilBufferEnd(const void *to);

// Allocator task stack TX check space
static inline error_return_t MsgAlloc_CheckMsgSpace(const void *from, const void *to);

// Available buffer space evaluation
static inline uint32_t MsgAlloc_BufferAvailableSpaceComputation(void);

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
    Phy_SetIrqState(false);
    data_ptr             = (uint8_t *)&msg_buffer[0];
    oldest_alloc_slot    = 0;
    available_alloc_slot = 0;
    memset((void *)alloc_slots, 0, sizeof(alloc_slots));
    Phy_SetIrqState(true);
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
void MsgAlloc_Loop(void)
{
    static volatile uint8_t *last_data_ptr = NULL;
    // Compute buffer occupation rate
    uint8_t stat = 0;
    if (data_ptr != last_data_ptr)
    {
        last_data_ptr = data_ptr;
        stat          = (uint8_t)(((MSG_BUFFER_SIZE - MsgAlloc_BufferAvailableSpaceComputation()) * 100) / (MSG_BUFFER_SIZE));
        if (stat > mem_stat->buffer_occupation_ratio)
        {
            mem_stat->buffer_occupation_ratio = stat;
        }
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
    LUOS_ASSERT(((uintptr_t)data_ptr >= (uintptr_t)&msg_buffer[0]) && ((uintptr_t)data_ptr < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE]));

    Phy_SetIrqState(false);
    uint8_t *oldest_msg        = alloc_slots[oldest_alloc_slot].data;
    uint8_t *data_ptr_snapshot = (uint8_t *)data_ptr;
    Phy_SetIrqState(true);

    if (oldest_msg != NULL)
    {
        LUOS_ASSERT(((uintptr_t)oldest_msg >= (uintptr_t)&msg_buffer[0]) && ((uintptr_t)oldest_msg < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE]));
        // There is some tasks
        if ((uintptr_t)oldest_msg > (uintptr_t)data_ptr_snapshot)
        {
            // The oldest task is between `data_ptr` and the end of the buffer
            //        msg_buffer
            //        +-------------------------------------------------------------+
            //        |-------------------------------------------------------------|
            //        +------^---------------------^--------------------------------+
            //               |                     |
            //               |<-----Free space---->|
            //               |                     |
            //               data_ptr              oldest_msg
            //
            stack_free_space = (uintptr_t)oldest_msg - (uintptr_t)data_ptr_snapshot;
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
            //                      oldest_msg                        data_ptr
            //
            stack_free_space = ((uintptr_t)oldest_msg - (uintptr_t)&msg_buffer[0]) + ((uintptr_t)&msg_buffer[MSG_BUFFER_SIZE] - (uintptr_t)data_ptr_snapshot);
        }
    }
    else
    {
        // There is nothing allocated
        stack_free_space = MSG_BUFFER_SIZE;
    }
    return stack_free_space;
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
_CRITICAL static inline error_return_t MsgAlloc_DoWeHaveSpaceUntilBufferEnd(const void *to)
{
    LUOS_ASSERT((uintptr_t)to >= (uintptr_t)&msg_buffer[0]);
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
 * @param phy_target_t phy_filter
 * @return uint8_t * : pointer to the allocated message
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL uint8_t *MsgAlloc_Alloc(uint16_t data_size, uint8_t phy_filter)
{
    // This function is always called with IRQ disabled.
    LUOS_ASSERT((data_size > 0)
                && (phy_filter != 0)
                && (data_size <= MSG_BUFFER_SIZE));
    uint8_t *returned_ptr;
    // Check data ptr alignement
    if ((uintptr_t)data_ptr % 2 == 1)
    {
        data_ptr++;
    }

    // Check if we have space for the message
    if (MsgAlloc_DoWeHaveSpaceUntilBufferEnd((void *)(data_ptr + data_size)) == FAILED)
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

        // Check if we dont jump any messages
        if (MsgAlloc_CheckMsgSpace((void *)data_ptr, (void *)&msg_buffer[MSG_BUFFER_SIZE - 1]) != SUCCEED)
        {
            // We don't have the space to store the message, return NULL to indicate that there is no more space
            return NULL;
        }

        // We don't have the space to store the message, move data_ptr to the beginning of the buffer
        returned_ptr = (uint8_t *)&msg_buffer[0];
    }
    else
    {
        returned_ptr = (uint8_t *)data_ptr;
    }
    // Check if we have space for the message
    if (MsgAlloc_CheckMsgSpace((void *)returned_ptr, (void *)((uintptr_t)returned_ptr + data_size)) != SUCCEED)
    {
        // We don't have the space to store the message, return NULL to indicate that there is no more space
        return NULL;
    }
    // We consider this space as occupied
    // Move data to the next available space
    data_ptr = (uint8_t *)((uintptr_t)returned_ptr + data_size);
    // Move the data_ptr to the begin of the buffer if we don't have space for at least a header
    if ((uintptr_t)data_ptr >= (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE - sizeof(header_t)])
    {
        data_ptr = &msg_buffer[0];
    }
    return returned_ptr;
}

/******************************************************************************
 * @brief Reference a message
 * @param uint8_t *rx_data
 * @param phy_target_t phy_filter
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void MsgAlloc_Reference(uint8_t *rx_data, uint8_t phy_filter)
{
    LUOS_ASSERT((rx_data < &msg_buffer[MSG_BUFFER_SIZE]) && (rx_data >= &msg_buffer[0]) && (phy_filter != 0));
    // Reference a space into the alloc_slots
    Phy_SetIrqState(false);
    uint16_t my_slot = available_alloc_slot++;
    if (available_alloc_slot >= MAX_MSG_NB)
    {
        available_alloc_slot = 0;
    }
    LUOS_ASSERT(available_alloc_slot != oldest_alloc_slot);
    Phy_SetIrqState(true);
    alloc_slots[my_slot].data       = rx_data;
    alloc_slots[my_slot].phy_filter = phy_filter;
}

/******************************************************************************
 * @brief Free a message
 * @param uint8_t phy_id : id of the phy that free the message
 * @param uint8_t *data : pointer to the message to free
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL void MsgAlloc_Free(uint8_t phy_id, const uint8_t *data)
{
    LUOS_ASSERT(data != NULL);
    // Find the slot where the data is stored
    int i = oldest_alloc_slot;
    while (i != available_alloc_slot)
    {
        if (alloc_slots[i].data == data)
        {
            // This is the one we are looking for
            // Remove the phy_id from the phy_filter
            // Assert if this phy have already been freed
            LUOS_ASSERT(alloc_slots[i].phy_filter & (0x01 << phy_id));
            Phy_SetIrqState(false);
            alloc_slots[i].phy_filter &= ~(0x01 << phy_id);
            // Check if the phy_filter is empty
            if (alloc_slots[i].phy_filter == 0)
            {
                // This message is not used anymore, free it
                alloc_slots[i].data = NULL;
                if (i == oldest_alloc_slot)
                {
                    do
                    {
                        oldest_alloc_slot++;
                        if (oldest_alloc_slot >= MAX_MSG_NB)
                        {
                            oldest_alloc_slot = 0;
                        }
                    } while ((alloc_slots[oldest_alloc_slot].data == NULL) && (oldest_alloc_slot != available_alloc_slot));
                }
            }
            Phy_SetIrqState(true);
            return;
        }
        i++;
        if (i >= MAX_MSG_NB)
        {
            i = 0;
        }
    }
    // If you pass here, the message have been freed already.
    // You probably are in detection and the reset detection reseted the allocator before this free.
}

/******************************************************************************
 * @brief No message in buffer receive since initialization
 * @param None
 * @return msg_t* sucess or fail if good init
 ******************************************************************************/
error_return_t MsgAlloc_IsEmpty(void)
{
    if (oldest_alloc_slot == available_alloc_slot)
    {
        return SUCCEED;
    }
    else
    {
        return FAILED;
    }
}

/******************************************************************************
 * @brief Check if there is space in buffer
 * @param from : start of the memory space to check
 * @param to : stop of the memory space to check
 * @return error_return_t
 ******************************************************************************/
static inline error_return_t MsgAlloc_CheckMsgSpace(const void *from, const void *to)
{
    LUOS_ASSERT(((uintptr_t)from < (uintptr_t)to)
                && ((uintptr_t)from >= (uintptr_t)&msg_buffer[0])
                && ((uintptr_t)from < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE])
                && ((uintptr_t)to >= (uintptr_t)&msg_buffer[0])
                && ((uintptr_t)to < (uintptr_t)&msg_buffer[MSG_BUFFER_SIZE]));
    uint8_t *oldest_msg = alloc_slots[oldest_alloc_slot].data;
    if ((((uintptr_t)oldest_msg >= (uintptr_t)from) && ((uintptr_t)oldest_msg <= (uintptr_t)to)))
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
        //                             "oldest_msg"
        //
        return FAILED;
    }
    return SUCCEED;
}
