/******************************************************************************
 * @file context
 * @brief definition of context structure environement luos variable
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <robus.h>
#include "robus_config.h"
#include "port_manager.h"
#include "reception.h"
#include "transmission.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct
{
    network_state_t state;
    bool timeout_run;
    uint32_t timeout;
} network_lock_t;

typedef struct
{

    // Variables
    node_t node;    /*!< Node informations. */
    RxCom_t rx;     /*!< Receiver informations. */
    TxCom_t tx;     /*!< Transmitter informations. */
    PortMng_t port; /*!< Port informations. */

    // Low level service management
    ll_service_t ll_service_table[MAX_SERVICE_NUMBER]; /*!< Low level Service table. */
    uint16_t ll_service_number;                        /*!< Low level Service number. */
    uint8_t IDMask[ID_MASK_SIZE];
    uint16_t IDShiftMask;

    // network management
    network_lock_t node_connected;

    uint8_t filter_state;
    uint16_t filter_id;
    uint8_t verbose;
    uint8_t TopicMask[TOPIC_MASK_SIZE]; /*!< multicast target bank. */

} context_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile extern context_t ctx;
/*******************************************************************************
 * Function
 ******************************************************************************/

#endif /* _CONTEXT_H_ */
