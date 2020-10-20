/******************************************************************************
 * @file context
 * @brief definition of context structure environement luos variable
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <robus.h>
#include "config.h"
#include "portManager.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef void (*DATA_CB)(volatile unsigned char *data);

typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            unsigned char identifier : 4;
            unsigned char rx_error : 1;
            unsigned char rx_timeout : 1;
        };
        unsigned char unmap; /*!< Uncmaped form. */
    };
} status_t;

typedef struct
{

    // Variables
    DATA_CB data_cb;                  /*!< Data management callback. */
    status_t status;                  /*!< Status. */
    node_t node;                      /*!< Node informations. */
    volatile unsigned char *tx_data;  /*!< sent data pointer. */
    volatile unsigned char tx_lock;   /*!< transmission locking management. */
    volatile unsigned char collision; /*!< collision flag. */
    volatile unsigned char ack;       /*!< acknoledge flag. */
    unsigned int baudrate;            /*!< System current baudrate. */

    detection_mode_t detection_mode;
    detection_t detection;

    //Virtual container management
    vm_t vm_table[MAX_VM_NUMBER]; /*!< Virtual Container table. */
    unsigned char vm_number;      /*!< Virtual Container number. */
    vm_t *vm_last_send;           /*!< Last Virtual Container id who send something. */

} context_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile extern context_t ctx;
/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /* _CONTEXT_H_ */
