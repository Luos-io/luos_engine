/******************************************************************************
 * @file gate_config
 * @brief Service gate configuration options
 * @author Luos
 ******************************************************************************/
#ifndef GATE_CONFIG_H
#define GATE_CONFIG_H

#include "luos.h"

/*******************************************************************************
 * Gate DEFINITION
 *******************************************************************************
 *    Define                  | Description
 *    :-----------------------|-----------------------------------------------
 *    GATE_BUFF_SIZE          | Formatted Data Buffer. Max size of 1 msg
 *    GATE_POLLING            | No autorefresh always ask data (more intensive to Luos bandwidth.)
 *    NODETECTION             | Gate not perform a network detection a power up
 *    GATE_REFRESH_TIME_S     | Default refresh Gate recalculate optimal rate at first command
 ******************************************************************************/

#ifndef GATE_BUFF_SIZE
#define GATE_BUFF_SIZE 1024
#endif

#define GATE_REFRESH_TIME_S 0.05f

#endif /* GATE_CONFIG_H */
