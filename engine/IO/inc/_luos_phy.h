/******************************************************************************
 * @file phy.h
 * @brief This file have to be imported by phy layers to interract with Luos
 * @author Nicolas Rabault
 * @version 0.0.0
 ******************************************************************************/
#ifndef _PRIVATE_LUOS_PHY_H_
#define _PRIVATE_LUOS_PHY_H_

#include "luos_phy.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

void Phy_Init(void);
void Phy_Reset(void);
void Phy_ResetAll(void);
bool Phy_Busy(void);
void Phy_Loop(void);
luos_phy_t *Phy_Get(uint8_t id, JOB_CB job_cb, RUN_TOPO run_topo, RESET_PHY reset_phy);
error_return_t Phy_FindNextNode(void); // Use it to find the next node as a master.

#endif /* _PRIVATE_LUOS_PHY_H_ */
