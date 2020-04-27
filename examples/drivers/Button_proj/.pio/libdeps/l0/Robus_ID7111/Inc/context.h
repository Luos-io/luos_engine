/*
 * context.h
 *
 * Created: 14/02/2017 11:53:28
 *  Author: Nicolas Rabault
 *  Abstract: shared structures containg the module states
 */
#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <robus.h>
#include "config.h"
#include "cmd.h"
#include "detection.h"

typedef void (*DATA_CB) (volatile unsigned char *data);

typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            unsigned char identifier : 4;
            unsigned char rx_error : 1;
            unsigned char rx_timeout : 1;
        };
        unsigned char unmap;                /*!< Uncmaped form. */
    };
} status_t;

typedef struct __attribute__((__packed__)){

    // Variables
    DATA_CB data_cb;    /*!< Data management callback. */
    RX_CB luos_cb;      /*!< Luos messages management callback. */
    status_t status;    /*!< Status. */
    unsigned short id;       /*!< Module ID. */
    unsigned char type;     /*!< Module type. */
    volatile unsigned char* tx_data;  /*!< sent data pointer. */
    volatile unsigned char tx_lock; /*!< transmission locking management. */
    volatile unsigned char collision; /*!< collision flag. */
    volatile unsigned char ack; /*!< acknoledge flag. */
    unsigned int baudrate;            /*!< System current baudrate. */

    detection_mode_t detection_mode;
    detection_t detection;

    //Virtual module management
    vm_t vm_table[MAX_VM_NUMBER];       /*!< Virtual Module table. */
    unsigned char vm_number; /*!< Virtual Module number. */
    vm_t* vm_last_send; /*!< Last Virtual Module id who send something. */

    //msg allocation management
    msg_t msg[MSG_BUFFER_SIZE];          /*!< Message table (one for each virtual module). */
    unsigned char current_buffer;        /*!< current msg buffer used. */
    unsigned char alloc_msg[MSG_BUFFER_SIZE];            /*!< Message allocation table. */
}context_t;

volatile extern context_t ctx;

#endif /* _CONTEXT_H_ */
