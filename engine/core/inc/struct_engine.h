/******************************************************************************
 * @file engine structures
 * @brief describe all engine private related structures
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __ENGINE_STRUCT_H
#define __ENGINE_STRUCT_H

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * This structure is used to manage packages
 * please refer to the documentation
 ******************************************************************************/
typedef struct
{
    void (*Init)(void);
    void (*Loop)(void);
} package_t;

#endif /*__ENGINE_STRUCT_H */
