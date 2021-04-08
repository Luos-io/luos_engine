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
#include "port_manager.h"
#include "reception.h"
#include "transmission.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{

    // Variables
    node_t node;        /*!< Node informations. */
    RxCom_t rx;         /*!< Receiver informations. */
    TxCom_t tx;         /*!< Transmitter informations. */
    volatile uint8_t ack_needed; /*!< Ack needed flag. */
    PortMng_t port;     /*!< port informations. */

    //Virtual container management
    ll_container_t ll_container_table[MAX_CONTAINER_NUMBER]; /*!< Virtual Container table. */
    uint16_t ll_container_number;                            /*!< Virtual Container number. */

} context_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile extern context_t ctx;
/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /* _CONTEXT_H_ */
