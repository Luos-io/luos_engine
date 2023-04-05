/******************************************************************************
 * @file stat structures
 * @brief describe all the statistics related structures
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __STAT_STRUCT_H
#define __STAT_STRUCT_H

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct memory_stats_t
 * @brief store informations about RAM occupation
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    uint8_t rx_msg_stack_ratio;
    uint8_t engine_msg_stack_ratio;
    uint8_t tx_msg_stack_ratio;
    uint8_t buffer_occupation_ratio;
    uint8_t msg_drop_number;
} memory_stats_t;

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

#endif /*__STAT_STRUCT_H */
