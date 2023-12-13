/******************************************************************************
 * @file serial_network.h
 * @brief serial communication driver for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOS_SERIAL_H_
#define _LUOS_SERIAL_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "luos_phy.h"

    /*******************************************************************************
     * Definitions
     ******************************************************************************/

    /*******************************************************************************
     * Function
     ******************************************************************************/
    void Serial_Init(void);
    void Serial_Loop(void);

#ifdef __cplusplus
}
#endif
#endif /* _LUOS_SERIAL_H_ */
