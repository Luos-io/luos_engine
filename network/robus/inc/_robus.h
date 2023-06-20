/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __ROBUS_H_
#define __ROBUS_H_

#include <stdint.h>
#include "luos_phy.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
luos_phy_t *Robus_GetPhy(void);

#endif /* __ROBUS_H_ */
