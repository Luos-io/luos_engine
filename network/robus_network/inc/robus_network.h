/******************************************************************************
 * @file robus
 * @brief User functionalities of the robus communication protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUS_H_
#define _ROBUS_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "luos_phy.h"

    /*******************************************************************************
     * Function
     ******************************************************************************/
    void Robus_Init(void);
    void Robus_Loop(void);

#ifdef __cplusplus
}
#endif
#endif /* _ROBUS_H_ */
