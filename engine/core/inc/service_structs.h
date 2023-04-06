/******************************************************************************
 * @file services structures
 * @brief describe all the service related structures
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __SERVICE_STRUCT_H
#define __SERVICE_STRUCT_H

#include <stdint.h>
#include "robus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * store informations about luos stats
 * please refer to the documentation
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            memory_stats_t memory;
            uint8_t max_loop_time_ms;
        };
        uint8_t unmap[sizeof(memory_stats_t) + 1]; /*!< streamable form. */
    };
} luos_stats_t;
/******************************************************************************
 * This structure is used to create services version
 * please refer to the documentation
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t major;
            uint8_t minor;
            uint8_t build;
        };
        uint8_t unmap[3]; /*!< streamable form. */
    };
} revision_t;
/******************************************************************************
 * This structure is used to manage services statistic
 * please refer to the documentation
 ******************************************************************************/
typedef struct __attribute__((__packed__)) service_stats_t
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t max_retry;
        };
        uint8_t unmap[1]; /*!< streamable form. */
    };
} service_stats_t;

/******************************************************************************
 * @struct general_stats_t
 * @brief format all datas to be sent trough msg
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            luos_stats_t node_stat;
            service_stats_t service_stat;
        };
        uint8_t unmap[sizeof(luos_stats_t) + sizeof(service_stats_t)]; /*!< streamable form. */
    };
} general_stats_t;

/******************************************************************************
 * This structure is used to manage services timed auto update
 * please refer to the documentation
 ******************************************************************************/
typedef struct __attribute__((__packed__)) timed_update_t
{
    uint32_t last_update;
    uint16_t time_ms;
    uint16_t target;
} timed_update_t;

/******************************************************************************
 * This structure is used to manage read or write access
 * please refer to the documentation
 ******************************************************************************/
typedef enum
{
    READ_WRITE_ACCESS,
    READ_ONLY_ACCESS,
    WRITE_ONLY_ACCESS,
    NO_ACCESS
} access_t;

/******************************************************************************
 * This structure is used to manage packages
 * please refer to the documentation
 ******************************************************************************/
typedef struct
{
    void (*Init)(void);
    void (*Loop)(void);
} package_t;

/******************************************************************************
 * This structure is used to manage services
 * please refer to the documentation
 ******************************************************************************/
typedef struct __attribute__((__packed__)) service_t
{
    ll_service_t *ll_service;
    // Callback
    void (*service_cb)(struct service_t *service, msg_t *msg);
    // Variables
    uint8_t default_alias[MAX_ALIAS_SIZE]; /*!< service default alias. */
    uint8_t alias[MAX_ALIAS_SIZE];         /*!< service alias. */
    timed_update_t auto_refresh;           /*!< service auto refresh context. */
    revision_t revision;                   /*!< service firmware version. */
    service_stats_t statistics;            /*!< service level statistics. */
    access_t access;                       /*!< service read write access. */
    void *profile_context;                 /*!< Pointer to the profile context. */
} service_t;

typedef void (*SERVICE_CB)(service_t *service, msg_t *msg);

/******************************************************************************
 * This enum list all CMD reserved to Luos
 * please refer to the documentation
 ******************************************************************************/
typedef enum
{
    // Luos specific registers
    LOCAL_RTB = ROBUS_PROTOCOL_NB, // Ask(size == 0), generate(size == 2) a local routing_table.
    RTB,                           // Receive a routing_table.
    WRITE_ALIAS,                   // Get and save a new given alias.
    UPDATE_PUB,                    // Ask to update a sensor value each time duration to the sender
    ASK_DETECTION,                 // Ask Luos to launch a detection

    // Revision management
    REVISION,        // service sends its firmware revision
    LUOS_REVISION,   // service sends its luos revision
    LUOS_STATISTICS, // service sends its luos revision

    // bootloader command and response
    BOOTLOADER_CMD,
    BOOTLOADER_RESP,

    // Verbose command
    VERBOSE,

    // compatibility area
    LUOS_LAST_RESERVED_CMD = 42
} reserved_luos_cmd_t;

#endif /*__SERVICE_STRUCT_H */
