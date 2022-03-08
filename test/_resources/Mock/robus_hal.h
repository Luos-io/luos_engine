/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @Family x86
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSHAL_H_
#define _LUOSHAL_H_

#include <stdint.h>
#include "robus_hal_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LUOS_UUID ((uint32_t *)0x00000001)

#define ADDRESS_ALIASES_FLASH   ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_BOOT_FLAG_FLASH (ADDRESS_LAST_PAGE_FLASH + PAGE_SIZE) - 4

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void RobusHAL_Init(void);
void RobusHAL_SetIrqState(uint8_t Enable);
uint32_t RobusHAL_GetSystick(void);
void RobusHAL_ComInit(uint32_t Baudrate);
void RobusHAL_SetTxState(uint8_t Enable);
void RobusHAL_SetRxState(uint8_t Enable);
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size);
uint8_t RobusHAL_GetTxLockState(void);
void RobusHAL_SetRxDetecPin(uint8_t Enable);
void RobusHAL_ResetTimeout(uint16_t nbrbit);
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr);
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr);
void RobusHAL_PushPTP(uint8_t PTPNbr);
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr);
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc);
void RobusHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data);
void RobusHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data);

// bootloader functions
void RobusHAL_SetMode(uint8_t mode);
void RobusHAL_Reboot(void);
void RobusHAL_SaveNodeID(uint16_t);

// timestamp functions
uint64_t RobusHAL_GetTimestamp(void);
void RobusHAL_StartTimestamp(void);
void RobusHAL_StopTimestamp(void);

#endif /* _LUOSHAL_H_ */
