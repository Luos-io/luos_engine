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
#define DEFAULTID         0x00
#define PROTOCOL_REVISION 0
#define BROADCAST_VAL     0x0FFF

#define TIMEOUT_VAL       2
#define MAX_ALIAS_SIZE    16
#define MAX_DATA_MSG_SIZE 128

#ifndef DEFAULTBAUDRATE
    #define DEFAULTBAUDRATE 1000000
#endif

#ifndef NBR_RETRY
    #define NBR_RETRY 10
#endif

#ifndef MAX_SERVICE_NUMBER
    #define MAX_SERVICE_NUMBER 5
#endif

#ifdef MAX_CONTAINER_NUMBER
    #error 'MAX_CONTAINER_NUMBER' is deprecated since luos_engine@2.0.0, replace it by 'MAX_SERVICE_NUMBER', see: www.github.com/Luos-io/luos_engine/releases/tag/2.0.0 for more information.
#endif

#ifndef MAX_PROFILE_NUMBER
    #define MAX_PROFILE_NUMBER MAX_SERVICE_NUMBER
#endif

#ifndef MSG_BUFFER_SIZE
    #define MSG_BUFFER_SIZE 3 * sizeof(msg_t)
#endif

#ifndef MAX_MSG_NB
    #define MAX_MSG_NB 2 * MAX_SERVICE_NUMBER
#endif

#ifndef NBR_PORT
    #define NBR_PORT 2
#endif

#ifndef LAST_TOPIC
    #define LAST_TOPIC 20
#endif

// Tab of byte. + 2 for overlap ID because aligned to byte
#define ID_MASK_SIZE    ((MAX_SERVICE_NUMBER / 8) + 2)
#define TOPIC_MASK_SIZE ((LAST_TOPIC / 8) + 2)
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /* _ROBUS_CONFIG_H_ */
