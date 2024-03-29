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
luos_phy_t *Phy_GetPhyFromId(uint8_t phy_id);
error_return_t Phy_FindNextNode(void);                          // Use it to find the next node as a master.
phy_job_t *Phy_GetNextJob(luos_phy_t *phy_ptr, phy_job_t *job); // Use it to get the next job to send.
// Filtering initialization
void Phy_FiltersInit(void);
void Phy_AddLocalServices(uint16_t service_id, uint16_t service_number);
bool Phy_FilterType(uint16_t type_id);
void Phy_IndexSet(uint8_t *index, uint16_t id);
void Phy_NodeIndexRm(uint16_t id);
void Phy_ServiceIndexRm(uint16_t id);
void Phy_ResetAllNeeded(void);

#endif /* _PRIVATE_LUOS_PHY_H_ */
