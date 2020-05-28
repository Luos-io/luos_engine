/**
 * This code is a memcpy like for flash
 * It allow to keep Luos aliases permanently
 * 
 * Warning : those function deal with Flash memory !
 *     - Flash memory can't be writen a lot of time so avoid frequent use of those functions
 *     - There is no random placement of values, this is a really basic write
 *     - Those function can take a lot of time and RAM due to big memory part copy.
 **/

#ifndef __LUOS_FLASH_H
#define __LUOS_FLASH_H

#include "main.h"

// Base address of the Flash sectors
// By default STM32G4 is divided into 2 banks having the same size (128 pages of 2KB)
// Here we just have to select the one we want to work with
#define ADDR_FLASH_BANK1 ((uint32_t)0x08000000)
#define ADDR_FLASH_BANK2 ((uint32_t)0x08040000)
#define ADDR_FLASH_BASE ADDR_FLASH_BANK2

#define PAGE_SIZE (uint32_t) FLASH_PAGE_SIZE /* Page size */

// To save aliases we use the last page of the flash
// compute the address of the last flash page
#define ALIASES_BASE_ADDRESS ((uint32_t)((FLASH_PAGE_NB - 1) * PAGE_SIZE) + ADDR_FLASH_BASE)

uint8_t luos_flash_read(uint16_t byte_id, size_t size, void *data);
uint8_t luos_flash_write(uint16_t byte_id, size_t size, const void *data);

#endif