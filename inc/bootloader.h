/******************************************************************************
 * @file Bootloader
 * @brief Bootloader functionnalities for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SHARED_MEMORY_ADDRESS   0x0800C000
#define SHARED_FLASH_PAGE       25
#define APP_ADDRESS             (uint32_t)0x0800C800

#define BOOT_MODE_MASK          0x000000FF

#define BOOTLOADER_MODE         0x00
#define APPLICATION_MODE        0x01

#define BOOTLOADER_RCV_COMMAND      0x01
#define BOOTLOADER_SND_COMMAND      0x10
typedef enum
{
    BOOTLOADER_IDLE,
    BOOTLOADER_START = BOOTLOADER_RCV_COMMAND,
    BOOTLOADER_STOP,
    BOOTLOADER_READY,
    BOOTLOADER_BIN_HEADER,
    BOOTLOADER_BIN_CHUNK,
    BOOTLOADER_BIN_END,
    BOOTLOADER_CRC_TEST,
    BOOTLOADER_READY_RESP = BOOTLOADER_SND_COMMAND,
    BOOTLOADER_BIN_HEADER_RESP,
    BOOTLOADER_BIN_END_RESP,
    BOOTLOADER_CRC_RESP,
} bootloader_cmd_t;

typedef enum
{
    BOOTLOADER_START_STATE,
    BOOTLOADER_READY_STATE,
    BOOTLOADER_STOP_STATE,
    BOOTLOADER_BIN_HEADER_STATE,
    BOOTLOADER_BIN_CHUNK_STATE,
    BOOTLOADER_BIN_END_STATE,
    BOOTLOADER_CRC_TEST_STATE,
} bootloader_state_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Main function used by the bootloader app
 ******************************************************************************/
void LuosBootloader_Run(void);

/******************************************************************************
 * @brief function used by Luos to send message to the bootloader
 ******************************************************************************/
void LuosBootloader_MsgHandler(uint8_t*);

#endif /* BOOTLOADER_H */