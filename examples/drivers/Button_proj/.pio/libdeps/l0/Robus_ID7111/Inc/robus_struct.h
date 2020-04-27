/**
 * \file robus_struct.h
 * \brief Robus communication main include file.
 * \author Nicolas Rabault
 * \version 0.1
 * \date 18 Fevrier 2017
 *
 * Include this file to use the robus communication protocol.
 *
 */

#ifndef _ROBUS_STRUCT_H_
#define _ROBUS_STRUCT_H_

#include "config.h"

/**
 * \enum target_mode_t
 * \brief Message addressing mode enum.
 *
 * This structure is used to get the message addressing mode list.
 */
typedef enum {
    ID,        /*!< Unique or virtual ID, used to send something to only one module. */
    IDACK,     /*!< Unique or virtual ID with reception Acknoledgment (ACK). */
    TYPE,      /*!< Type mode, used to send something to all module of the same type. */
    BROADCAST, /*!< Broadcast mode, used to send something to everybody. */
    MULTICAST  /*!< Multicast mode, used to send something to multiple modules. */
}target_mode_t;

/**
 * \struct header_t
 * \brief Header structure.
 *
 * This structure is used specify data and destination of datas.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            unsigned short protocol : 4;       /*!< Protocol version. */
            unsigned short target : 12;        /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
            unsigned short target_mode : 4;    /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
            unsigned short source : 12;        /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
            unsigned char cmd;                 /*!< msg definition. */
            unsigned short size;                /*!< Size of the data field. */
        };
        unsigned char unmap[7];                /*!< Unmaped form. */
    };
}header_t;

/**
 * \struct msg_t
 * \brief Message structure.
 *
 * This structure is used to receive or send messages between modules in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            header_t header;                            /*!< Header filed. */
            unsigned char data[MAX_DATA_MSG_SIZE];      /*!< Data with size known. */
        };
        unsigned char stream[sizeof(header_t) + MAX_DATA_MSG_SIZE]; /*!< unmaped option. */
    };
    union {
        unsigned short crc;
        volatile unsigned char ack;
    };
}msg_t;

/**
 * \struct vm_t
 * \brief Virtual Module Structure
 *
 * This structure is used to manage virtual modules
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) vm_t{

    // Module infomations
    unsigned short id;                  /*!< Module ID. */
    unsigned char type;                 /*!< Module type. */

    // Variables
    msg_t* msg_pt;                                               /*!< Message pointer. */
    unsigned char max_multicast_target;                          /*!< Position pointer of the last multicast target. */
    unsigned short multicast_target_bank[MAX_MULTICAST_ADDRESS]; /*!< multicast target bank. */
    unsigned short dead_module_spotted;                          /*!< The ID of a module that don't reply to a lot of ACK msg */
}vm_t;

typedef void (*RX_CB) (vm_t* vm, msg_t *msg);

#endif /* _ROBUS_STRUCT_H_ */
