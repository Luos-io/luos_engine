/******************************************************************************
 * @file gate_config
 * @brief Service gate configuration options
 * @author Luos
 ******************************************************************************/
#ifndef GATE_CONFIG_H
#define GATE_CONFIG_H

#include "luos_engine.h"

/*******************************************************************************
 * Gate DEFINITION
 *******************************************************************************
 *    Define                  | Description
 *    :-----------------------|-----------------------------------------------
 *    GATE_BUFF_SIZE          | Formatted Data Buffer. Max size of 1 msg
 *    NODETECTION             | Gate not perform a network detection a power up
 *    GATE_REFRESH_TIME_S     | Default refresh Gate recalculate optimal rate at first command
 *    INIT_TIME               | Delay before first detection, to verify that all boards are connected
 ******************************************************************************/

#ifndef GATE_BUFF_SIZE
    #define GATE_BUFF_SIZE 1024
#endif

#ifndef INIT_TIME
    #define INIT_TIME 150
#endif

#ifndef GATE_REFRESH_TIME_S
    #define GATE_REFRESH_TIME_S 0.05f
    #define GATE_REFRESH_AUTOSCALE
#endif

#endif /* GATE_CONFIG_H */
