/******************************************************************************
 * @file robus_HAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family STM32FO
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RobusHAL_H_
#define _RobusHAL_H_

#include <stdint.h>
#include <stdbool.h>
#include "robus_config.h"
#include "robus_hal_config.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void RobusHAL_Init(void);
void RobusHAL_Loop(void);
void RobusHAL_ComInit(uint32_t Baudrate);
void RobusHAL_SetTxState(uint8_t Enable);
void RobusHAL_SetRxState(uint8_t Enable);
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size);
uint8_t RobusHAL_GetTxLockState(void);
void RobusHAL_ResetTimeout(uint16_t nbrbit);
void RobusHAL_SetRxDetecPin(uint8_t Enable);
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr);
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr);
void RobusHAL_PushPTP(uint8_t PTPNbr);
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr);
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc);
void RobusHAL_SetIrqState(bool Enable);
#endif /* _RobusHAL_H_ */
