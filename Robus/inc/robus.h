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
void Robus_Init(memory_stats_t *memory_stats);
void Robus_Loop(void);
vm_t *Robus_ContainerCreate(uint16_t type);
void Robus_ContainersClear(void);
uint8_t Robus_SendMsg(vm_t *vm, msg_t *msg);
node_t *Robus_GetNode(void);

#endif /* _ROBUS_H_ */
