/******************************************************************************
 * @file stats
 * @brief manage statistics of Luos
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <string.h>
#include "stats.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
general_stats_t general_stats;

/*******************************************************************************
 * Function
 ******************************************************************************/

void Stats_Init(void)
{
    memset(&general_stats, 0, sizeof(general_stats_t));
}

general_stats_t *Stats_Get(void)
{
    return &general_stats;
}

memory_stats_t *Stats_GetMemory(void)
{
    return &general_stats.node_stat.memory;
}

luos_stats_t *Stats_GetLuos(void)
{
    return &general_stats.node_stat;
}
