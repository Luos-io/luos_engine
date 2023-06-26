/******************************************************************************
 * @file luos_bootloader
 * @brief This file contains a service acting as a bootloader for Luos
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bootloader_core.h"
#include "luos_hal.h"
#include "luos_engine.h"
#include "routing_table.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef BOOTLOADER
    #define MAX_FRAME_SIZE (MAX_DATA_MSG_SIZE - 1)
    #define BUFFER_SIZE    0x800 // 2kB buffer to store received data

/*******************************************************************************
 * Variables
 ******************************************************************************/
// Variables used to manage current data to write and page location
uint32_t flash_addr = APP_START_ADDRESS;
uint8_t data_buff[(uint16_t)BUFFER_SIZE];
uint32_t nb_bytes = 0;

    #ifndef BOOTLOADER_UPDATER
// Create a variable of the size of mode flash value allowing to init the shared flash section
const uint8_t __attribute__((used)) __attribute__((section(".boot_flags"))) sharedSection = BOOT_MODE;
    #endif

/*******************************************************************************
 * Function
 ******************************************************************************/
static uint8_t LuosBootloader_GetMode(void);
static void LuosBootloader_DeInit(void);
static void LuosBootloader_JumpToApp(void);
static void LuosBootloader_SetNodeID(void);
static uint8_t LuosBootloader_IsEnoughSpace(uint32_t);
static void LuosBootloader_EraseMemory(void);
static void LuosBootloader_ProcessData(uint8_t *data, uint32_t data_size);
static void LuosBootloader_SaveLastData(void);
static void LuosBootloader_SendResponse(service_t *service, uint16_t source_id, uint16_t response_cmd);
static void LuosBootloader_SendCrc(service_t *service, uint16_t source_id, uint16_t response_cmd, uint8_t data);
static void LuosBootloader_MsgHandler(service_t *service, const msg_t *input);

/******************************************************************************
 * @brief Create a service to signal a bootloader node
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_Init(void)
{
    revision_t version = {.major = 2, .minor = 0, .build = 0};
    #ifdef BOOTLOADER_UPDATER
    Luos_CreateService(LuosBootloader_MsgHandler, VOID_TYPE, "boot_updater", version);
    #else
    Luos_CreateService(LuosBootloader_MsgHandler, VOID_TYPE, "boot_service", version);
    #endif

    // set ID node saved in flash
    LuosBootloader_SetNodeID();

    switch (LuosBootloader_GetMode())
    {
        case JUMP_TO_APP_MODE:
            // boot the application programmed in dedicated flash partition
            LuosBootloader_DeInit();
            LuosBootloader_JumpToApp();
            break;

        case APP_RELOAD_MODE:
            LuosHAL_SetMode((uint8_t)JUMP_TO_APP_MODE);
            break;

        case BOOT_MODE:
        default:
            break;
    }
}

/******************************************************************************
 * @brief Read the boot mode in flash memory
 * @param None
 * @return boot_mode
 ******************************************************************************/
uint8_t LuosBootloader_GetMode(void)
{
    return LuosHAL_GetMode();
}

/******************************************************************************
 * @brief DeInit peripherals in bootloader
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_DeInit(void)
{
    LuosHAL_DeInit();
}

/******************************************************************************
 * @brief Launch application from the bootloader
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_JumpToApp(void)
{
    LuosHAL_JumpToAddress(APP_START_ADDRESS);
}

/******************************************************************************
 * @brief Set node id with data saved in flash
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_SetNodeID(void)
{
    uint16_t node_id = LuosHAL_GetNodeID();
    node_t *node     = Node_Get();

    node->node_id = node_id;
}

/******************************************************************************
 * @brief Check if there is enough space to flash binary
 * @param binary_size : The size of the binary to flash
 * @return None
 ******************************************************************************/
inline uint8_t LuosBootloader_IsEnoughSpace(uint32_t binary_size)
{
    #ifdef BOOTLOADER_UPDATER
    uint32_t free_space = BOOT_START_ADDRESS - APP_START_ADDRESS - 1;
    #else
    uint32_t free_space = APP_END_ADDRESS - APP_START_ADDRESS;
    #endif
    if (free_space > binary_size)
    {
        return SUCCEED;
    }
    else
    {
        return FAILED;
    }
}

/******************************************************************************
 * @brief Erase memory wher application is stored
 * @param None
 * @return None
 ******************************************************************************/
inline void LuosBootloader_EraseMemory(void)
{
    LuosHAL_EraseMemory(APP_START_ADDRESS, nb_bytes);
}

/******************************************************************************
 * @brief Process binary data received from the gate
 * @param None
 * @return None
 ******************************************************************************/
inline void LuosBootloader_ProcessData(uint8_t *data, uint32_t data_size)
{
    static uint16_t data_index     = 0;
    static uint16_t residual_space = (uint16_t)BUFFER_SIZE;
    if (residual_space >= data_size)
    {
        // There is enough space in the current page to save the complete data
        // Fill the current page with data
        memcpy(&data_buff[data_index], data, data_size);

        // Update data_index and residual_space for the next chunk of data
        data_index += data_size;
        residual_space -= data_size;
    }
    else
    {
        // Complete the current page buffer
        memcpy(&data_buff[data_index], data, residual_space);

        // Save the completed page in flash
        LuosHAL_ProgramFlash(flash_addr, (uint16_t)BUFFER_SIZE, data_buff);

        // Prepare the next page buffer
        flash_addr += BUFFER_SIZE;
        data_index = 0;
        memset(data_buff, 0xFF, (uint16_t)BUFFER_SIZE);

        // Copy the remaining data in the new page buffer
        memcpy(&data_buff[data_index], &data[residual_space], data_size - residual_space);

        // Update data_index and residual_space
        data_index     = data_size - residual_space;
        residual_space = (uint16_t)BUFFER_SIZE - data_index;
    }
}

/******************************************************************************
 * @brief Save the current page when BIN_END command is received
 * @param None
 * @return None
 ******************************************************************************/
inline void LuosBootloader_SaveLastData(void)
{
    LuosHAL_ProgramFlash(flash_addr, (uint16_t)BUFFER_SIZE, data_buff);
}

/******************************************************************************
 * @brief Compute crc 8 for each data
 * @param data : Pointer to the datas
 * @param crc : Pointer to crc to be computed
 * @param polynome : The seed to init crc computing
 * @return crc : Computed CRC value
 ******************************************************************************/
void crc8(const uint8_t *data, uint8_t *crc, uint16_t polynome)
{
    uint16_t dbyte = 0;
    uint16_t mix   = 0;

    // zero padding data
    dbyte = (uint16_t)*data << 8;
    // add initial value
    dbyte ^= (uint16_t)*crc << 8;
    // right shift polynomial key
    polynome = polynome << 8;
    // shift and xor if necessary
    for (uint8_t j = 0; j < 8; ++j)
    {
        mix   = dbyte & 0x8000;
        dbyte = (dbyte << 1);
        if (mix)
            dbyte ^= polynome;
    }

    *crc = (uint8_t)(dbyte >> 8);
}

/******************************************************************************
 * @brief Compute crc for the whole binary
 * @param None
 * @return crc : Computed CRC value
 ******************************************************************************/
uint8_t compute_crc(void)
{
    uint8_t crc   = 0x00; // initial crc value
    uint8_t data  = 0x00;
    uint16_t poly = 0x0007;

    uint32_t data_counter   = 0;
    uint8_t data_index      = 0;
    uintptr_t *data_address = (uintptr_t *)APP_START_ADDRESS;
    uintptr_t data_flash    = 0;

    while (data_counter < nb_bytes)
    {
        // read 1 word in flash memory
        data_flash = *data_address;
        // read 4 bytes in flash memory
        for (data_index = 0; data_index < 4; data_index++)
        {
            if (data_counter < nb_bytes)
            {
                data = (uint8_t)(data_flash >> (8 * data_index));
                // compute crc
                crc8(&data, &crc, poly);
                data_counter += 1;
            }
        }
        // update data address
        data_address += 1;
    }

    return crc;
}

/******************************************************************************
 * @brief Send CRC message response
 * @param response : The type of crc message
 * @param data : The crc value
 * @return None
 ******************************************************************************/
void LuosBootloader_SendCrc(service_t *service, uint16_t source_id, uint16_t response_cmd, uint8_t data)
{
    msg_t ready_msg;
    ready_msg.header.cmd         = response_cmd;
    ready_msg.header.target_mode = SERVICEIDACK;
    ready_msg.header.target      = source_id;
    ready_msg.header.size        = sizeof(uint8_t);
    ready_msg.data[0]            = data;
    node_t *node                 = Node_Get();
    uint32_t tick                = LuosHAL_GetSystick();
    while (LuosHAL_GetSystick() - tick < node->node_id)
        ;
    Luos_SendMsg(service, &ready_msg);
}

/******************************************************************************
 * @brief Send response to the gate
 * @param response : The type of crc message
 * @return None
 ******************************************************************************/
void LuosBootloader_SendResponse(service_t *service, uint16_t source_id, uint16_t response_cmd)
{
    msg_t ready_msg;
    ready_msg.header.cmd         = response_cmd;
    ready_msg.header.target_mode = SERVICEIDACK;
    ready_msg.header.target      = source_id;
    ready_msg.header.size        = 0;
    node_t *node                 = Node_Get();
    uint32_t tick                = LuosHAL_GetSystick();
    while (LuosHAL_GetSystick() - tick < node->node_id)
        ;
    Luos_SendMsg(service, &ready_msg);
}

/******************************************************************************
 * @brief bootloader app
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_Loop(void)
{
}

/******************************************************************************
 * @brief Message handler called from luos library
 * @param service : Pointer to the service which received the message
 * @param input : Pointer to message received from luos network
 * @return None
 ******************************************************************************/
void LuosBootloader_MsgHandler(service_t *service, const msg_t *input)
{
    static uint8_t crc    = 0;
    static bool load_flag = false;
    uint8_t bootloader_data[MAX_FRAME_SIZE];
    uint32_t tickstart;

    switch (input->header.cmd)
    {
            // we're in the bootloader,
            // process cmd and data
        case BOOTLOADER_READY:
            // Subscribe to the flash topic
            Luos_Subscribe(service, (uint16_t)input->data[0]);
            // Reset the bootloader in boot mode
            LuosHAL_SetMode((uint8_t)BOOT_MODE);
            // Save binary length
            memcpy(&nb_bytes, &input->data[1], sizeof(uint32_t));

            // Check the free space in flash
            if (LuosBootloader_IsEnoughSpace(nb_bytes) == SUCCEED)
            {
                // Send READY response
                LuosBootloader_SendResponse(service, input->header.source, BOOTLOADER_READY);
            }
            else
            {
                // Send ERROR response
                LuosBootloader_SendResponse(service, input->header.source, BOOTLOADER_ERROR_SIZE);
            }
            break;

        case BOOTLOADER_ERASE:
            // Erase flash memory
            LuosBootloader_EraseMemory();
            // Reset load flag indicating that there is no apps anymore
            load_flag = false;
            // Send ERASE response
            LuosBootloader_SendResponse(service, input->header.source, BOOTLOADER_ERASE);
            break;

        case BOOTLOADER_BIN_CHUNK:
            memcpy(bootloader_data, input->data, input->header.size);

            // Handle binary data
            LuosBootloader_ProcessData(bootloader_data, input->header.size);

            // Send ack to the Host
            LuosBootloader_SendResponse(service, input->header.source, BOOTLOADER_BIN_CHUNK);
            break;

        case BOOTLOADER_BIN_END:
            // The binary download is complete
            // Save the current page in flash memory even if it's not complete
            LuosBootloader_SaveLastData();

            // Send ack to the Host
            LuosBootloader_SendResponse(service, input->header.source, BOOTLOADER_BIN_END);
            break;

        case BOOTLOADER_CRC:
            crc = compute_crc();
            // Send an ack to the Host
            LuosBootloader_SendCrc(service, input->header.source, BOOTLOADER_CRC, crc);
            break;

        case BOOTLOADER_APP_SAVED:
            // set load flag indicating that there is an app in flash
            load_flag = true;
            Luos_Unsubscribe(service, input->header.target);
            break;

        case BOOTLOADER_STOP:
            // wait for the command to be send to all nodes
            tickstart = LuosHAL_GetSystick();
            while ((LuosHAL_GetSystick() - tickstart) < 1000)
                ;
            // save bootloader mode in flash depending if we actually have an app in flash and need to reload it
            if (load_flag || (LuosBootloader_GetMode() == APP_RELOAD_MODE))
            {
                // boot the application programmed in dedicated flash partition
                LuosBootloader_DeInit();
                LuosBootloader_JumpToApp();
            }
            else
            {
                // reboot the node
                LuosHAL_Reboot();
            }
            break;
        default:
            break;
    }
}
#endif
