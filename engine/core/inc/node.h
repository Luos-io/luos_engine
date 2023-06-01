/******************************************************************************
 * @file node management
 * @brief time stamp data
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __NODE_H_
#define __NODE_H_

#include <stdint.h>
#include "engine_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    NO_DETECTION,
    DETECTION_OK,
    LOCAL_DETECTION,
    EXTERNAL_DETECTION,
} node_state_t;

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
                uint16_t node_id : 12;  /*!< Node id */
                uint16_t certified : 4; /*!< True if the node have a certificate */
                uint8_t node_info;
            };
            uint16_t port_table[NBR_PORT]; /*!< Phisical port connections */
        };
        uint8_t unmap[NBR_PORT + 3]; /*!< Uncmaped form. */
    };
} node_t;

/*******************************************************************************
 * Function
 ******************************************************************************/
void Node_Init(void);
void Node_Loop(void);
node_t *Node_Get(void);
node_state_t Node_GetState(void);
void Node_SetState(node_state_t);

#endif /* __NODE_H_ */
