/******************************************************************************
 * @file pub_sub.h
 * @brief multicast referencing description
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _STATS_H_
#define _STATS_H_

#include "struct_stat.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void Stats_Init(void);

general_stats_t *Stats_Get(void);
memory_stats_t *Stats_GetMemory(void);
luos_stats_t *Stats_GetLuos(void);

#endif /* _PUB_SUB_H_ */
