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
#include "Reception.h"
#include "Transmission.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{

    // Variables
    node_t node;                     /*!< Node informations. */
    RxCom_t rx;                      /*!< Receiver informations. */
    TxCom_t tx;                      /*!< Transmitter informations. */
    uint8_t ack;                     /*!< Ack informations. */
    PortMng_t port;                  /*!< port informations. */

    //Virtual container management
    vm_t vm_table[MAX_VM_NUMBER];    /*!< Virtual Container table. */
    uint8_t vm_number;         /*!< Virtual Container number. */

} context_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile extern context_t ctx;
/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /* _CONTEXT_H_ */
