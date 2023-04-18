/******************************************************************************
 * @file luos structures
 * @brief describe all core structures
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __LUOS_STRUCT_H
#define __LUOS_STRUCT_H

#include "engine_config.h"
#include "struct_stat.h"
#include "struct_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum
{
    // Protocol version
    BASE_PROTOCOL = PROTOCOL_REVISION,
    TIMESTAMP_PROTOCOL,
} protocol_t;

/******************************************************************************
 * This enum list all CMD reserved to Luos
 * please refer to the documentation
 ******************************************************************************/
typedef enum
{
    // Luos specific registers
    WRITE_NODE_ID,   // Get and save a new given node ID.
    START_DETECTION, // Start a detection
    END_DETECTION,   // Detect the end of a detection
    LOCAL_RTB,       // Ask(size == 0), generate(size == 2) a local routing_table.
    RTB,             // Receive a routing_table.
    WRITE_ALIAS,     // Get and save a new given alias.
    UPDATE_PUB,      // Ask to update a sensor value each time duration to the sender
    ASK_DETECTION,   // Ask Luos to launch a detection
    DEADTARGET,      // Send a dead target message
    ASSERT,          // Node Assert message (only broadcast with a source as a node)

    // Revision management
    REVISION,        // service sends its firmware revision
    LUOS_REVISION,   // service sends its luos revision
    LUOS_STATISTICS, // service sends its luos revision

    // bootloader command and response
    BOOTLOADER_CMD,
    BOOTLOADER_RESP,

    // compatibility area
    // LUOS_LAST_RESERVED_CMD = 42
} reserved_luos_cmd_t;

/******************************************************************************
 * @enum target_mode_t
 * @brief This enum list all target mode.
 ******************************************************************************/
typedef enum
{
    SERVICEID,    /*!< Unique or virtual ID, used to send something to only one service. */
    SERVICEIDACK, /*!< Unique or virtual ID with reception Acknoledgment (ACK). */
    TYPE,         /*!< Type mode, used to send something to all service of the same type. */
    BROADCAST,    /*!< Broadcast mode, used to send something to everybody. */
    TOPIC,        /*!< Multicast mode, used to send something to multiple services. */
    NODEID,       /*!< Node mode, used to send something to all services of a node. */
    NODEIDACK,    /*!< Node mode with reception Acknoledgment (ACK). */

    ID    = SERVICEID,   /*!< This define is deprecated, please use SERVICEID instead. */
    IDACK = SERVICEIDACK /*!< This define is deprecated, please use SERVICEIDACK instead. */
} target_mode_t;

/******************************************************************************
 * @struct header_t
 * @brief This structure is used specify data and destination of datas.
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint16_t config : 4;      /*!< Protocol version. */
            uint16_t target : 12;     /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
            uint16_t target_mode : 4; /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
            uint16_t source : 12;     /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
            uint8_t cmd;              /*!< msg definition. */
            uint16_t size;            /*!< Size of the data field. */
        };
        uint8_t unmap[7]; /*!< Unmaped form. */
    };
} header_t;

/******************************************************************************
 * @struct msg_t
 * @brief Message structure
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            header_t header;                 /*!< Header filed. */
            uint8_t data[MAX_DATA_MSG_SIZE]; /*!< Data with size known. */
        };
        uint8_t stream[sizeof(header_t) + MAX_DATA_MSG_SIZE]; /*!< unmaped option. */
    };
} msg_t;

/******************************************************************************
 * This structure is used to manage services timed auto update
 * please refer to the documentation
 ******************************************************************************/
typedef struct timed_update_t
{
    uint32_t last_update;
    uint16_t time_ms;
    uint16_t target;
} timed_update_t;

/******************************************************************************
 * This structure is used to manage dead target message
 * Service_id or node_id can be set to 0 to ignore it.
 * Only 1 of those 2 can have a value indicating if the dead target is a service or a node.
 * please refer to the documentation
 ******************************************************************************/
typedef struct __attribute__((__packed__)) dead_target_t
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint16_t service_id;
            uint16_t node_id;
        };
        uint8_t unmap[4];
    };
} dead_target_t;

/******************************************************************************
 * This structure is used to manage read or write access
 * please refer to the documentation
 ******************************************************************************/
typedef enum
{
    READ_WRITE_ACCESS,
    READ_ONLY_ACCESS,
    WRITE_ONLY_ACCESS,
    NO_ACCESS
} access_t;

/******************************************************************************
 * This structure is used to create services version
 * please refer to the documentation
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t major;
            uint8_t minor;
            uint8_t build;
        };
        uint8_t unmap[3]; /*!< streamable form. */
    };
} revision_t;

/******************************************************************************
 * This structure is used to manage services
 * please refer to the documentation
 ******************************************************************************/
typedef struct service_t
{
    // Service infomations
    uint16_t id;                           /*!< Service ID. */
    uint16_t type;                         /*!< Service type. */
    uint8_t default_alias[MAX_ALIAS_SIZE]; /*!< service default alias. */
    uint8_t alias[MAX_ALIAS_SIZE];         /*!< service alias. */
    revision_t revision;                   /*!< service firmware version. */
    access_t access;                       /*!< service read write access. */

    // Callback
    void (*service_cb)(struct service_t *service, const msg_t *msg);

    // Statistics
    service_stats_t statistics; /*!< service level statistics. */

    // Private Variables
    uint16_t last_topic_position;    /*!< Position pointer of the last topic added. */
    uint16_t topic_list[LAST_TOPIC]; /*!< multicast target bank. */
    timed_update_t auto_refresh;     /*!< service auto refresh context. */
    void *profile_context;           /*!< Pointer to the profile context. */

} service_t;

typedef void (*SERVICE_CB)(service_t *service, const msg_t *msg);

#endif /*__LUOS_STRUCT_H */
