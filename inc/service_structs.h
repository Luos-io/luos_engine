/******************************************************************************
 * @file services structure
 * @brief describs all the
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __SERVICE_STRUCT_H
#define __SERVICE_STRUCT_H

#include <stdint.h>
#include "luos.h"
#include "luos_od.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* store informations about luos stats
 * please refer to the documentation
 */
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
/* This structure is used to create services version
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t Major;
            uint8_t Minor;
            uint8_t Build;
        };
        uint8_t unmap[3]; /*!< streamable form. */
    };
} revision_t;
/* This structure is used to manage services statistic
 * please refer to the documentation
 */
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

/* This structure is used to manage services timed auto update
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) timed_update_t
{
    uint32_t last_update;
    uint16_t time_ms;
    uint16_t target;
} timed_update_t;

/* This structure is used to manage read or write access
 * please refer to the documentation
 */
typedef enum
{
    READ_WRITE_ACCESS,
    READ_ONLY_ACCESS,
    WRITE_ONLY_ACCESS,
    NO_ACCESS
} access_t;

/* This structure is used to manage packages
 * please refer to the documentation
 */
typedef struct
{
    void (*Init)(void);
    void (*Loop)(void);
} package_t;

/* This structure is used to manage services
 * please refer to the documentation
 */
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
    luos_stats_t *node_statistics;         /*!< Node level statistics. */
    service_stats_t statistics;            /*!< service level statistics. */
    access_t access;                       /*!< service read write access. */
    void *template_context;                /*!< Pointer to the template context. */
} service_t;

typedef void (*SERVICE_CB)(service_t *service, msg_t *msg);

/*
 * Luos unic ID => ARM serial number
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        uint32_t uuid[3];
        uint8_t unmap[3 * sizeof(uint32_t)]; /*!< Uncmaped form. */
    };
} luos_uuid_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /*__SERVICE_STRUCT_H */
