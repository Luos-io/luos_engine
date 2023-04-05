/******************************************************************************
 * @file struct_io
 * @brief io level structures
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _STRUCT_IO_H_
#define _STRUCT_IO_H_

#include <stdint.h>
#include "engine_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct luos_localhost_t
 * @brief Transmit message direction
 ******************************************************************************/
typedef enum
{
    EXTERNALHOST, // This message is for an external service
    LOCALHOST,    // This message is for an internal service only
    MULTIHOST     // This message is for an internal and an external service
} luos_localhost_t;

typedef enum
{
    // protocol level command
    WRITE_NODE_ID,   /*!< Get and save a new given node ID. */
    START_DETECTION, /*!< Start a detection*/
    END_DETECTION,   /*!< Detect the end of a detection*/
    ASSERT,          /*!< Node Assert message (only broadcast with a source as a node */

    /*!< Compatibility area*/
    // ROBUS_PROTOCOL_NB = 13,
} robus_cmd_t;

typedef enum
{
    // Protocol version
    BASE_PROTOCOL = PROTOCOL_REVISION,
    TIMESTAMP_PROTOCOL,
} robus_protocol_t;

#endif /* _STRUCT_IO_H_ */
