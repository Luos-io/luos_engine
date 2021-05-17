/******************************************************************************
 * @file Boot
 * @brief fonctionnalities for luos bootloader
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "boot.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define REV     \
    {           \
        1, 0, 0 \
    }

/*******************************************************************************
 * Variables
 ******************************************************************************/
typedef enum
{
    BOOT_APP = LUOS_LAST_TYPE
} App_type_t;

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Boot_Init(void)
{
    revision_t revision = {.unmap = REV};
    Luos_CreateContainer(0, BOOT_APP, "boot_app", revision);
}
