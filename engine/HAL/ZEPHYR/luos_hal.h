/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstraction Layer for Zephyr (Cortex-M).
 * @MCU Family Zephyr / nRF54L15
 ******************************************************************************/
#ifndef _LUOSHAL_H_
#define _LUOSHAL_H_

#include <stdint.h>
#include <stdbool.h>
#include "luos_hal_config.h"

#define _CRITICAL

#define BOOT_MODE_MASK   0x000000FF
#define BOOT_MODE_OFFSET 0
#define NODE_ID_MASK     0x00FFFF00
#define NODE_ID_OFFSET   8

typedef struct ll_timestamp
{
    uint32_t lower_timestamp;
    uint64_t higher_timestamp;
    uint32_t start_offset;
} ll_timestamp_t;

void LuosHAL_Init(void);
void LuosHAL_SetIrqState(bool Enable);
uint32_t LuosHAL_GetSystick(void);

uint64_t LuosHAL_GetTimestamp(void);
void LuosHAL_StartTimestamp(void);
void LuosHAL_StopTimestamp(void);

#endif /* _LUOSHAL_H_ */
