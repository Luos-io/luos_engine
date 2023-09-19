/******************************************************************************
 * @file robus_config config
 * @brief config of the Robus protocol
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUS_CONFIG_H_
#define _ROBUS_CONFIG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TIMEOUT_VAL 2

#ifdef DEFAULTBAUDRATE
    #error 'DEFAULTBAUDRATE' configuration is deprecated and have been replaced by ROBUS_NETWORK_BAUDRATE.
#endif
#ifndef ROBUS_NETWORK_BAUDRATE
    #define ROBUS_NETWORK_BAUDRATE 1000000
#endif

#ifndef NBR_RETRY
    #define NBR_RETRY 10
#endif

#ifndef NBR_PORT
    #define NBR_PORT 2
#endif

#define CRC_SIZE 2

#endif /* _ROBUS_CONFIG_H_ */
