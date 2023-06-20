/******************************************************************************
 * @file engine_config
 * @brief config for luos_engine library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ENGINE_CONFIG_H_
#define _ENGINE_CONFIG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LUOS_LAST_RESERVED_CMD 42 // Last Luos reserved command
#define ROBUS_PROTOCOL_NB      13 // Number of Robus protocol command
#define ALIAS_SIZE             15 // Number of max char for service alias
#define MAX_ALIAS_SIZE         16
#define DETECTION_TIMEOUT_MS   10000 // Timeout used to detect a failed detection
#define BOOT_TIMEOUT           1000
#define DEFAULTID              0x00
#define PROTOCOL_REVISION      0
#define BROADCAST_VAL          0x0FFF

#define MAX_DATA_MSG_SIZE 128

#ifndef MAX_SERVICE_NUMBER
    #define MAX_SERVICE_NUMBER 5
#endif

#ifndef MAX_NODE_NUMBER
    #define MAX_NODE_NUMBER 20
#endif

#ifndef MAX_RTB_ENTRY
    #define MAX_RTB_ENTRY 40
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

#ifndef LAST_TOPIC
    #define LAST_TOPIC 20
#endif

// Tab of byte. + 2 for overlap ID because aligned to byte
#define ID_MASK_SIZE    ((MAX_SERVICE_NUMBER / 8) + 2)
#define TOPIC_MASK_SIZE ((LAST_TOPIC / 8) + 2)

#endif /* _ENGINE_CONFIG_H_ */
