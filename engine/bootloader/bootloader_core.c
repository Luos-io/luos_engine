/******************************************************************************
 * @file Bootloader
 * @brief Bootloader functionnalities for luos framework
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
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bootloader_cmd_t bootloader_cmd;

#ifdef BOOTLOADER
// variables use to save binary data in flash
static uint8_t bootloader_data[MAX_FRAME_SIZE];
static uint16_t bootloader_data_size = 0;

uint32_t flash_addr = APP_START_ADDRESS;
uint8_t data_buff[(uint16_t)BUFFER_SIZE];
uint16_t data_index     = 0;
uint16_t residual_space = (uint16_t)BUFFER_SIZE;
uint32_t nb_bytes       = 0;
uint8_t crc             = 0;
bool load_flag          = false;
uint16_t source_id      = 0; // used to save source_id, ie gate_id
uint32_t tickstart      = 0;

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
static inline uint8_t LuosBootloader_IsEnoughSpace(uint32_t);
static inline void LuosBootloader_EraseMemory(void);
static inline void LuosBootloader_ProcessData(void);
static inline void LuosBootloader_SaveLastData(void);
static void LuosBootloader_SendResponse(bootloader_cmd_t);
static void LuosBootloader_SendCrc(bootloader_cmd_t, uint8_t);
#endif

/******************************************************************************
 * @brief Save node id in flash
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_JumpToBootloader(void)
{
    // Set bootlaoder mode
    LuosHAL_SetMode((uint8_t)APP_RELOAD_MODE);

    // Save node id in flash
    node_t *node     = Node_Get();
    uint16_t node_id = node->node_id;

    LuosHAL_SaveNodeID(node_id);

    // Reset the MCU
    LuosHAL_Reboot();
}

#ifdef BOOTLOADER
/******************************************************************************
 * @brief Create a service to signal a bootloader node
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_Init(void)
{
    revision_t version = {.major = 2, .minor = 0, .build = 0};
    #ifdef BOOTLOADER_UPDATER
    Luos_CreateService(0, VOID_TYPE, "boot_updater", version);
    #else
    Luos_CreateService(0, VOID_TYPE, "boot_service", version);
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
uint8_t LuosBootloader_IsEnoughSpace(uint32_t binary_size)
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
void LuosBootloader_EraseMemory(void)
{
    LuosHAL_EraseMemory(APP_START_ADDRESS, nb_bytes);
}

/******************************************************************************
 * @brief Process binary data received from the gate
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_ProcessData(void)
{
    if (residual_space >= bootloader_data_size)
    {
        // there is enough space in the current page
        // fill the current page with data
        memcpy(&data_buff[data_index], bootloader_data, bootloader_data_size);

        // update data_index and residual_space
        data_index += bootloader_data_size;
        residual_space -= bootloader_data_size;
    }
    else
    {
        // complete the current page buffer
        memcpy(&data_buff[data_index], bootloader_data, residual_space);

        // save the completed page in flash memory
        LuosHAL_ProgramFlash(flash_addr, (uint16_t)BUFFER_SIZE, data_buff);

        // prepare next page buffer
        flash_addr += BUFFER_SIZE;
        data_index = 0;
        memset(data_buff, 0xFF, (uint16_t)BUFFER_SIZE);

        // copy the remaining data in the new page buffer
        memcpy(&data_buff[data_index], &bootloader_data[residual_space], bootloader_data_size - residual_space);

        // update data_index and residual_space
        data_index     = bootloader_data_size - residual_space;
        residual_space = (uint16_t)BUFFER_SIZE - data_index;
    }
}

/******************************************************************************
 * @brief Save the current page when BIN_END command is received
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_SaveLastData(void)
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
void LuosBootloader_SendCrc(bootloader_cmd_t response, uint8_t data)
{
    msg_t ready_msg;
    ready_msg.header.cmd         = BOOTLOADER_RESP;
    ready_msg.header.target_mode = SERVICEIDACK;
    ready_msg.header.target      = source_id;
    ready_msg.header.size        = 2 * sizeof(uint8_t);
    ready_msg.data[0]            = response;
    ready_msg.data[1]            = data;
    node_t *node                 = Node_Get();
    uint32_t tick                = LuosHAL_GetSystick();
    while (LuosHAL_GetSystick() - tick < node->node_id)
        ;
    Luos_SendMsg(0, &ready_msg);
}

/******************************************************************************
 * @brief Send response to the gate
 * @param response : The type of crc message
 * @return None
 ******************************************************************************/
void LuosBootloader_SendResponse(bootloader_cmd_t response)
{
    msg_t ready_msg;
    ready_msg.header.cmd         = BOOTLOADER_RESP;
    ready_msg.header.target_mode = SERVICEIDACK;
    ready_msg.header.target      = source_id;
    ready_msg.header.size        = sizeof(uint8_t);
    ready_msg.data[0]            = response;
    node_t *node                 = Node_Get();
    uint32_t tick                = LuosHAL_GetSystick();
    while (LuosHAL_GetSystick() - tick < node->node_id)
        ;
    Luos_SendMsg(0, &ready_msg);
}

/******************************************************************************
 * @brief bootloader app
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_Loop(void)
{
}
#endif

/******************************************************************************
 * @brief Message handler called from luos library
 * @param input : Pointer to message received from luos network
 * @return None
 ******************************************************************************/
void LuosBootloader_MsgHandler(const msg_t *input)
{
    bootloader_cmd = input->data[0];

    switch (bootloader_cmd)
    {
#ifdef WITH_BOOTLOADER
        case BOOTLOADER_START:
            // We're in the app,
            // set bootloader mode, save node ID and reboot
            LuosBootloader_JumpToBootloader();
            break;
#endif
#ifdef BOOTLOADER
            // we're in the bootloader,
            // process cmd and data
        case BOOTLOADER_READY:
            source_id            = input->header.source;
            bootloader_data_size = input->header.size - 2 * sizeof(char);
            Luos_Subscribe(0, (uint16_t)input->data[1]);
            memcpy(bootloader_data, &(input->data[2]), bootloader_data_size);

            LuosHAL_SetMode((uint8_t)BOOT_MODE);

            // save binary length
            memcpy(&nb_bytes, &bootloader_data[0], sizeof(uint32_t));
            // check free space in flash
            if (LuosBootloader_IsEnoughSpace(nb_bytes) == SUCCEED)
            {
                // send READY response
                LuosBootloader_SendResponse(BOOTLOADER_READY_RESP);
            }
            else
            {
                // send ERROR response
                LuosBootloader_SendResponse(BOOTLOADER_ERROR_SIZE);
            }
            break;

        case BOOTLOADER_ERASE:
            // erase flash memory
            LuosBootloader_EraseMemory();
            // reset load flag
            load_flag = false;
            // send ERASE response
            LuosBootloader_SendResponse(BOOTLOADER_ERASE_RESP);
            break;

        case BOOTLOADER_BIN_CHUNK:
            source_id            = input->header.source;
            bootloader_data_size = input->header.size - sizeof(char);
            memcpy(bootloader_data, &(input->data[1]), bootloader_data_size);

            // handle binary data
            LuosBootloader_ProcessData();
            // send ack to the Host
            LuosBootloader_SendResponse(BOOTLOADER_BIN_CHUNK_RESP);
            break;

        case BOOTLOADER_BIN_END:
            source_id            = input->header.source;
            bootloader_data_size = input->header.size - sizeof(char);
            memcpy(bootloader_data, &(input->data[1]), bootloader_data_size);

            // save the current page in flash memory
            LuosBootloader_SaveLastData();
            // send ack to the Host
            LuosBootloader_SendResponse(BOOTLOADER_BIN_END_RESP);
            break;

        case BOOTLOADER_CRC_TEST:
            crc = compute_crc();
            // send ack to the Host
            LuosBootloader_SendCrc(BOOTLOADER_CRC_RESP, crc);
            break;

        case BOOTLOADER_APP_SAVED:
            // set load flag
            load_flag = true;
            Luos_Unsubscribe(0, input->header.target);
            break;

        case BOOTLOADER_STOP:
            // wait for the command to be send to all nodes
            tickstart = LuosHAL_GetSystick();
            while ((LuosHAL_GetSystick() - tickstart) < 1000)
                ;
            // save bootloader mode in flash
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
#endif
        case BOOTLOADER_RESET:
            LuosHAL_SetMode((uint8_t)BOOT_MODE);
            LuosHAL_Reboot();
            break;
        default:
            break;
    }
}
