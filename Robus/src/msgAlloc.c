/******************************************************************************
 * @file msgAlloc.c
 * @brief Message allocation manager
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "config.h"
#include "msgAlloc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

//msg allocation management
uint8_t msg_buffer[MSG_BUFFER_SIZE];     /*!< Memory space used to save and alloc messages. */
msg_allocator_t alloc_table[MAX_MSG_NB]; /*!< Message allocation table. */
uint16_t alloc_id;                       /*!< last writen alloc_table id. */
msg_t *current_msg;                      /*!< current work in progress msg pointer. */
uint8_t *data_ptr;                       /*!< Pointer to the next data able to be writen into msgbuffer. */

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief get the next free alloc_table slot.
 * @param None
 * @return None
 ******************************************************************************/
static void MsgAlloc_FindAllocSlot(void)
{
    /******** Find the next available slotslot **********
     * - loop from alloc_id to MAX_MSG_NB to find a free slot set alloc_id and return if there is one
     * - loop from 0 to alloc_id to find a free slot set alloc_id and return if there is one
     * - At this point there is no slot available, put an error somewhere
     * - remove the message on the next slot and make it clean for the module of this slot
     * - set alloc_id to the next slot pointer and return .
     */
}

/******************************************************************************
 * @brief prepare a buffer space to be usable by cleaning remaining messages and prepare pointers
 * @param from : start of the memory space to clean
 * @param to : start of the memory space to clean
 * @return None
 ******************************************************************************/
static void MsgAlloc_ClearMemSpace(uint8_t *from, uint8_t *to)
{
    /******** Prepare a memory space to be writable **********
     * - be sure that the end of msg_buffer is after the to pointer
     * - check if thers is no msg between from and to
     * - If there is one remove it
     */
}

/******************************************************************************
 * @brief Init the allocator.
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_Init(void)
{
    /******** Init **********
     * - init current_msg to msg_buffer
     * - init data_ptr to msg_buffer
     */
}

/******************************************************************************
 * @brief Invalid the current message header by removing it (data will be ignored).
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_InvalidHeader(void)
{
    /******** Remove the header  *********
     * - move data_ptr to current_msg
     */
}

/******************************************************************************
 * @brief Valid the current message header by preparing the allocator to get the message data
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_ValidHeader(void)
{
    /******** Prepare the allocator to get data  *********
     * - Be sure that the end of msg_buffer is after data_ptr + header_t.size
     * - If the end of buffer is before the end of the message :
     *     - clean space between &msg_buffer[0] and (&msg_buffer[0] + sizeof(header_t) + header_t.size) using MsgAlloc_ClearMemSpace(uint8_t *from, uint8_t *to)
     *     - Copy the header at the begining of msg_buffer
     *     - move current_msg to &msg_buffer[0]
     *     - move data_ptr to current_msg + sizeof(header_t)
     * - If not 
     *     - clean space between data_ptr and (data_ptr + header_t.size) using MsgAlloc_ClearMemSpace(uint8_t *from, uint8_t *to)
     */
}

/******************************************************************************
 * @brief Finish and valid the current message and prepare the allocator to get the next one
 * @param None
 * @return None
 ******************************************************************************/
void MsgAlloc_EndMsg(void)
{
    /******** End the message **********
     * - MsgAlloc_FindAllocSlot()
     * - fill alloc_table[alloc_id] struct
     */
    /******** Prepare the next msg *********
     * - clean space between data_ptr (data_ptr + sizeof(header_t)) using MsgAlloc_ClearMemSpace(uint8_t *from, uint8_t *to)
     * - update the current_msg = data_ptr
     */
}

/******************************************************************************
 * @brief write a byte into the current message.
 * @param uint8_t data to write in the allocator
 * @return None
 ******************************************************************************/
uint8_t MsgAlloc_SetData(uint8_t data)
{
    /******** Write data  *********
     * - copy data into *data_ptr
     * - move data_ptr forward data_ptr++
     */
}

/******************************************************************************
 * @brief return the current message
 * @param None
 * @return msg_t* pointer to the current message
 ******************************************************************************/
msg_t *MsgAlloc_GetCurrentMsg(void)
{
    return current_msg;
}

/******************************************************************************
 * @brief get back alloc_table and alloc_id
 * @param msg_allocator_t *_alloc_table : The table linking modules and messages
 * @param uint16_t *_alloc_id : The last writen msg
 * @return None
 ******************************************************************************/
void MsgAlloc_GetAllocVars(msg_allocator_t *_alloc_table, uint16_t *_alloc_id)
{
    _alloc_table = alloc_table;
    _alloc_id = alloc_id;
}

/******************************************************************************
 * @brief get back alloc_table and alloc_id
 * @param msg_allocator_t *_alloc_table : The table linking modules and messages
 * @param uint16_t *_alloc_id : The last writen msg
 * @return None
 ******************************************************************************/
void MsgAlloc_ClearSlot(uint16_t _alloc_id)
{
    // Disable interrupt
    alloc_table[_alloc_id].msg_pt = 0;
    alloc_table[_alloc_id].vm_pt = 0;
    // Enable interrupt
}