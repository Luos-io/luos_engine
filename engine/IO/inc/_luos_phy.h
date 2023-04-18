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
void Phy_Loop(void);
luos_phy_t *Phy_Get(uint8_t id, PHY_CB phy_cb);

#endif /* _PRIVATE_LUOS_PHY_H_ */
