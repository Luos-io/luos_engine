/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUS_H_
#define _ROBUS_H_

#include <stdint.h>
#include "robus_struct.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Robus_Init(void);
void Robus_Loop(void);
vm_t *Robus_ModuleCreate(uint8_t type);
void Robus_ModulesClear(void);
uint8_t Robus_SendMsg(vm_t *vm, msg_t *msg);
uint16_t *Robus_GetNodeBranches(uint8_t *size);

#endif /* _ROBUS_H_ */
