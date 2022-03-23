/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family STM32G4
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_H_
#define _LUOSHAL_H_

#include <stdint.h>
#include <luos_hal_config.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LUOS_UUID ((uint32_t *)0x1FFF7590)

#define PAGE_SIZE        (uint32_t) FLASH_PAGE_SIZE
#define ADDR_FLASH_BANK1 ((uint32_t)0x08000000)
#define ADDR_FLASH_BANK2 ((uint32_t)0x08040000)
#define NB_OF_PAGE       FLASH_PAGE_NB

#define BOOT_MODE_MASK   0x000000FF
#define BOOT_MODE_OFFSET 0
#define NODE_ID_MASK     0x00FFFF00
#define NODE_ID_OFFSET   8

#define ADDRESS_ALIASES_FLASH   ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_BOOT_FLAG_FLASH (ADDRESS_LAST_PAGE_FLASH + PAGE_SIZE) - 4

/*******************************************************************************
 * Variables
 ******************************************************************************/

typedef struct ll_timestamp
{
    uint32_t lower_timestamp;
    uint64_t higher_timestamp;
    uint64_t start_offset;
} ll_timestamp_t;

/*******************************************************************************
 * Function
 ******************************************************************************/
void LuosHAL_Init(void);
void LuosHAL_SetIrqState(uint8_t Enable);
uint32_t LuosHAL_GetSystick(void);
void LuosHAL_ComInit(uint32_t Baudrate);
void LuosHAL_SetTxState(uint8_t Enable);
void LuosHAL_SetRxState(uint8_t Enable);
void LuosHAL_ComTransmit(uint8_t *data, uint16_t size);
uint8_t LuosHAL_GetTxLockState(void);
void LuosHAL_SetRxDetecPin(uint8_t Enable);
void LuosHAL_ResetTimeout(uint16_t nbrbit);
void LuosHAL_SetPTPDefaultState(uint8_t PTPNbr);
void LuosHAL_SetPTPReverseState(uint8_t PTPNbr);
void LuosHAL_PushPTP(uint8_t PTPNbr);
uint8_t LuosHAL_GetPTPState(uint8_t PTPNbr);
void LuosHAL_ComputeCRC(uint8_t *data, uint8_t *crc);
void LuosHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data);
void LuosHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data);

// bootloader functions
void LuosHAL_SetMode(uint8_t mode);
void LuosHAL_Reboot(void);
void LuosHAL_SaveNodeID(uint16_t);

#ifdef BOOTLOADER_CONFIG
void LuosHAL_DeInit(void);
void LuosHAL_JumpToApp(uint32_t);
uint8_t LuosHAL_GetMode(void);
uint16_t LuosHAL_GetNodeID(void);
void LuosHAL_EraseMemory(uint32_t, uint16_t);
void LuosHAL_ProgramFlash(uint32_t, uint16_t, uint8_t *);
#endif

// timestamp functions
uint64_t LuosHAL_GetTimestamp(void);
void LuosHAL_StartTimestamp(void);
void LuosHAL_StopTimestamp(void);

#endif /* _LUOSHAL_H_ */
