/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUS_H_
#define _ROBUS_H_

#include <stdbool.h>
#include "luos_phy.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void Robus_Init(void);
void Robus_Loop(void);

error_return_t Robus_SetTxTask(service_t *service, msg_t *msg);

void Robus_SaveNodeID(uint16_t nodeid);
void Robus_ResetNodeID(void);
bool Robus_Busy(void);
error_return_t Robus_FindNeighbour(void);
luos_phy_t *Robus_GetPhy(void);

#endif /* _ROBUS_H_ */
