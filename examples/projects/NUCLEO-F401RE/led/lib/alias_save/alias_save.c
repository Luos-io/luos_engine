/******************************************************************************
 * @file save_alias
 * @brief a way to save alias on flash
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "main.h"
#include "alias_save.h"
#include "luos_engine.h"
#include "robus.h"
#include "string.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Alias_FlashPageErase(void);

/******************************************************************************
 * @brief read an alias from flash
 * @param slot a memory slot number
 * @param alias the table pointer to write the alias.
 * @return the size of the alias
 ******************************************************************************/
int Alias_read(int slot, char *alias)
{
    uint32_t addr = ALIAS_PAGE_ADDRESS + (slot * MAX_ALIAS_SIZE);
    memcpy(alias, (void *)(addr), MAX_ALIAS_SIZE);
    return strlen(alias);
}

/******************************************************************************
 * @brief write an alias from flash
 * @param slot a memory slot number
 * @param alias the table pointer to write the alias.
 * @param size the size of the alias
 * @return none
 ******************************************************************************/
void Alias_write(int slot, char *alias, size_t size)
{
    if (size > MAX_ALIAS_SIZE)
    {
        size = MAX_ALIAS_SIZE;
    }
    // Before writing we have to erase the entire page.
    // To do that we have to backup current values by copying it into RAM.
    uint8_t page_backup[(PAGE_SIZE / MAX_ALIAS_SIZE) + 1][MAX_ALIAS_SIZE];
    memcpy(page_backup, (void *)ALIAS_PAGE_ADDRESS, PAGE_SIZE);

    // Now we can erase the page
    Alias_FlashPageErase();

    // Clean the input data
    char clean_alias[MAX_ALIAS_SIZE] = {0};
    memcpy(clean_alias, alias, size);

    // Then add input data into backuped value on RAM
    memcpy(page_backup[slot], clean_alias, MAX_ALIAS_SIZE);

    // Copy it into flash
    HAL_FLASH_Unlock();

    // ST hal flash program function write data by uint32_t raw data
    uint8_t *page_data = (uint8_t *)page_backup;
    for (uint32_t i = 0; i < PAGE_SIZE; i += sizeof(uint32_t))
    {
        while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, i + ALIAS_PAGE_ADDRESS, *(uint32_t *)(&page_data[i])) != HAL_OK)
            ;
    }
    HAL_FLASH_Lock();
}

/******************************************************************************
 * @brief Erase flash page where aliases are stored
 * @param None
 * @return None
 ******************************************************************************/
static void Alias_FlashPageErase(void)
{
    uint32_t sector_error = 0;
    FLASH_EraseInitTypeDef s_eraseinit;

    s_eraseinit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    s_eraseinit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    s_eraseinit.NbSectors    = 1;
    s_eraseinit.Sector       = ALIAS_SECTOR;

    // Erase Page
    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&s_eraseinit, &sector_error);
    HAL_FLASH_Lock();
}
