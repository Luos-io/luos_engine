/******************************************************************************
 * @file robus_struct
 * @brief definition protocole robus structure
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _ROBUS_STRUCT_H_
#define _ROBUS_STRUCT_H_

#include "config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct memory_stats_t
 * @brief store informations about RAM occupation
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    unsigned char alloc_stack_ratio;
    unsigned char msg_stack_ratio;
    unsigned char luos_stack_ratio;
    unsigned char msg_drop_number;
} memory_stats_t;

/*
 * This structure is used to get the message addressing mode list.
 */
typedef enum
{
    ID,        /*!< Unique or virtual ID, used to send something to only one container. */
    IDACK,     /*!< Unique or virtual ID with reception Acknoledgment (ACK). */
    TYPE,      /*!< Type mode, used to send something to all container of the same type. */
    BROADCAST, /*!< Broadcast mode, used to send something to everybody. */
    MULTICAST, /*!< Multicast mode, used to send something to multiple containers. */
    NODEID,    /*!< Node mode, used to send something to all containers of a node. */
    NODEIDACK  /*!< Node mode with reception Acknoledgment (ACK). */
} target_mode_t;

/* This structure is used specify data and destination of datas.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            unsigned short protocol : 4;    /*!< Protocol version. */
            unsigned short target : 12;     /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
            unsigned short target_mode : 4; /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
            unsigned short source : 12;     /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
            unsigned char cmd;              /*!< msg definition. */
            unsigned short size;            /*!< Size of the data field. */
        };
        unsigned char unmap[7]; /*!< Unmaped form. */
    };
} header_t;

/* This structure is used to receive or send messages between containers in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            header_t header;                       /*!< Header filed. */
            unsigned char data[MAX_DATA_MSG_SIZE]; /*!< Data with size known. */
        };
        unsigned char stream[sizeof(header_t) + MAX_DATA_MSG_SIZE]; /*!< unmaped option. */
    };
} msg_t;

/* This structure is used to manage virtual containers
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) vm_t
{

    // Container infomations
    unsigned short id;   /*!< Container ID. */
    unsigned short type; /*!< Container type. */

    // Variables
    unsigned short max_multicast_target;                         /*!< Position pointer of the last multicast target. */
    unsigned short multicast_target_bank[MAX_MULTICAST_ADDRESS]; /*!< multicast target bank. */
    unsigned short dead_container_spotted;                       /*!< The ID of a container that don't reply to a lot of ACK msg */
} vm_t;

/******************************************************************************
 * @struct error_return_t
 * @brief Return function error global convention
 ******************************************************************************/
typedef enum
{
    SUCESS,     /*!< function work properly. */
    FAIL = 0xFF /*!< function fail. */
} error_return_t;

/******************************************************************************
 * @struct node_t
 * @brief node informations structure
 ******************************************************************************/
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            struct __attribute__((__packed__))
            {
                unsigned short node_id : 12;  /*!< Node id */
                unsigned short certified : 4; /*!< True if the node have a certificate */
            };
            unsigned short port_table[NBR_BRANCH]; /*!< Phisical port connections */
        };
        unsigned char unmap[NBR_BRANCH + 2]; /*!< Uncmaped form. */
    };
} node_t;

typedef void (*RX_CB)(vm_t *vm, msg_t *msg);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /* _ROBUS_STRUCT_H_ */
