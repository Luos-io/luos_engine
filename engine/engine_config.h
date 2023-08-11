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
#define LUOS_LAST_RESERVED_CMD 42     // Last Luos reserved command
#define MAX_ALIAS_SIZE         16     // Number of max char for service alias
#define DETECTION_TIMEOUT_MS   10000  // Timeout used to detect a failed detection
#define DEFAULTID              0x00   // The default ID of a Luos service
#define PROTOCOL_REVISION      0      // The Luos protocol revision
#define BROADCAST_VAL          0x0FFF // The broadcast target value
#define MAX_DATA_MSG_SIZE      128    // The maximum size of a data message

#ifndef MAX_LOCAL_SERVICE_NUMBER
    #define MAX_LOCAL_SERVICE_NUMBER 5 // The maximum number of local services
#endif

#ifndef MAX_SERVICE_NUMBER
    #define MAX_SERVICE_NUMBER 20 // The maximum number of services in the complete architecture
#endif

#ifndef MAX_NODE_NUMBER
    #define MAX_NODE_NUMBER 20 // The maximum number of nodes in the complete architecture
#endif

#ifndef LOCAL_PHY_NB
    #define LOCAL_PHY_NB 1 // The number of phy layer in the node, by default we have only Luos engine + 1 phy layer
#endif

#define PHY_NB (LOCAL_PHY_NB + 1) // The total number of phy layer in the node including Luos engine

#ifdef MAX_RTB_ENTRY
    #error 'MAX_RTB_ENTRY' configuration is deprecated and have been replaced by MAX_NODE_NUMBER and MAX_SERVICE_NUMBER. MAX_RTB_ENTRY is now automatically calculated by the engine based on these values.
#endif
#define MAX_RTB_ENTRY (MAX_NODE_NUMBER + MAX_SERVICE_NUMBER)

#ifdef MAX_CONTAINER_NUMBER
    #error 'MAX_CONTAINER_NUMBER' is deprecated since luos_engine@2.0.0, replace it by 'MAX_LOCAL_SERVICE_NUMBER', see: www.github.com/Luos-io/luos_engine/releases/tag/2.0.0 for more information.
#endif

#ifndef MAX_LOCAL_PROFILE_NUMBER
    #define MAX_LOCAL_PROFILE_NUMBER MAX_LOCAL_SERVICE_NUMBER // The maximum number of profile in the node
#endif

#ifndef MSG_BUFFER_SIZE
    #define MSG_BUFFER_SIZE 3 * sizeof(msg_t) // The size of the message buffer
#endif

#ifndef MAX_MSG_NB
    #define MAX_MSG_NB 2 * MAX_LOCAL_SERVICE_NUMBER // The maximum number of message referenced by Luos
#endif

#ifndef MAX_LOCAL_TOPIC_NUMBER
    #define MAX_LOCAL_TOPIC_NUMBER 20 // The maximum number of topic in the node
#endif

// Tab of byte. + 2 for overlap ID because aligned to byte
#define ID_MASK_SIZE    ((MAX_LOCAL_SERVICE_NUMBER / 8) + 2)
#define TOPIC_MASK_SIZE ((MAX_LOCAL_TOPIC_NUMBER / 8) + 2)

#endif /* _ENGINE_CONFIG_H_ */
