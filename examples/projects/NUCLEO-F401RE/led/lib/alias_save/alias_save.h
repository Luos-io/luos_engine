/******************************************************************************
 * @file alias_save
 * @brief a way to save alias for STM32 boards
 * @author Luos
 * @version 1.0.0
 ******************************************************************************/
#ifndef ALIAS_SAVE_H
#define ALIAS_SAVE_H

#include <stdint.h>

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
#ifndef PAGE_SIZE
    #define PAGE_SIZE (uint32_t)0x400
#endif
#ifndef ALIAS_SECTOR
    #define ALIAS_SECTOR FLASH_SECTOR_7
#endif
#ifndef ALIAS_PAGE_ADDRESS
    #define ALIAS_PAGE_ADDRESS ((uint32_t)(FLASH_END - PAGE_SIZE + 1))
#endif

/*******************************************************************************
 * Function
 ******************************************************************************/
int Alias_read(int slot, char *alias);
void Alias_write(int slot, const uint8_t *alias, size_t size);

#endif /* ALIAS_SAVE_H */
