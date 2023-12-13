/******************************************************************************
 * @file utils structures
 * @brief describe practical general structures used by Luos
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __STRUCT_UTILS_H
#define __STRUCT_UTILS_H

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct error_return_t
 * @brief Return function error global convention
 ******************************************************************************/
typedef enum
{
    SUCCEED,      /*!< function work properly. */
    PROHIBITED,   /*!< function usage is currently prohibited. */
    FAILED = 0xFF /*!< function fail. */
} error_return_t;

#endif /*__STRUCT_UTILS_H */
