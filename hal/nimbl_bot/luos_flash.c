#include "luos_flash.h"
#include <string.h>

/**
  * @brief  Erase PAGE
  * @param  None
  * @retval Success or error status:
  *           - 0: on success
  *           - 1: on fail
  */
static uint8_t page_format(void)
{
    HAL_StatusTypeDef flashstatus = HAL_OK;
    uint32_t page_error = 0;

    // Setup erase configuration
    // We use only the last page of BANK 2
    FLASH_EraseInitTypeDef s_eraseinit;
    s_eraseinit.TypeErase = FLASH_TYPEERASE_PAGES;
    s_eraseinit.Banks = FLASH_BANK_2;
    //s_eraseinit.PageAddress = ALIASES_BASE_ADDRESS;
    s_eraseinit.Page = FLASH_PAGE_NB - 1;
    s_eraseinit.NbPages = 1;

    // Erase Page
    HAL_FLASH_Unlock();
    flashstatus = HAL_FLASHEx_Erase(&s_eraseinit, &page_error);
    HAL_FLASH_Lock();
    // If erase operation was failed, a Flash error code is returned
    if (flashstatus != HAL_OK)
    {
        return 1;
    }
    return 0;
}

/**
  * @brief  Read a "size" number of octet from byte_id.
  * @param  byte_id: 16 bit virtual address of the variable represent byte number into flash mem
  * @param  size: number of byte to read
  * @param  data: pointer to a data table
  * @retval Success or error status:
  *           - 0: on success
  *           - 1: on fail
  */
uint8_t luos_flash_read(uint16_t byte_id, size_t size, void *data)
{
    // check if there is no memory overflow
    if ((byte_id + size) > PAGE_SIZE)
        return 1;

    // copy flash datas into data pointer.
    memcpy(data, (__IO void *)(byte_id + ALIASES_BASE_ADDRESS), size);
    // for (uint16_t i = StartVirtualAddress; i < HowManyToRead + StartVirtualAddress; i++)
    // {
    //     *Data = (*(__IO uint32_t *)((i * 4) + _EEPROM_FLASH_PAGE_ADDRESS));
    //     Data++;
    // }
    return 0;
}

/**
  * @brief  write a "size" number of octet from byte_id.
  * @param  byte_id: 16 bit virtual address of the variable represent byte number into flash mem
  * @param  size: number of byte to write
  * @param  data: pointer to a data table
  * @retval Success or error status:
  *           - 0: on success
  *           - 1: on fail
  */
uint8_t luos_flash_write(uint16_t byte_id, size_t size, const void *data)
{
    // Before writing we have to erase the entire page
    // to do that we have to backup current falues by copying it into RAM
    uint8_t page_backup[PAGE_SIZE];
    memcpy(page_backup, (__IO void *)ALIASES_BASE_ADDRESS, PAGE_SIZE);

    // Now we can erase the page
    if (page_format())
    {
        //erase fail
        return 1;
    }

    // Then add input data into backuped value on RAM
    memcpy(&page_backup[byte_id], data, size);

    // and copy it into flash
    HAL_FLASH_Unlock();
    // ST hal flash program function write data by uint64_t raw data
    for (uint32_t i = 0; i < PAGE_SIZE; i += sizeof(uint64_t))
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, i + ALIASES_BASE_ADDRESS, *(uint64_t *)(&page_backup[i])) != HAL_OK)
        {
            //write fail
            HAL_FLASH_Lock();
            return 1;
        }
    HAL_FLASH_Lock();
    return 0;
}