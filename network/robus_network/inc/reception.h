/******************************************************************************
 * @file reception
 * @brief reception state machine
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RECEPTION_H_
#define _RECEPTION_H_

#include "_robus_network.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef void (*DATA_CB)(luos_phy_t *phy, volatile uint8_t *data);

typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t identifier : 4;
            uint8_t rx_error : 1;
            uint8_t rx_timeout : 1;
            uint8_t rx_framing_error : 1;
        };
        uint8_t unmap; /*!< Uncmaped form. */
    };
} status_t;

typedef struct
{
    DATA_CB callback;
    status_t status;
} RxCom_t;

/*******************************************************************************
 * Function
 ******************************************************************************/

void Recep_data(volatile uint8_t *data);

// Callbacks reception
void Recep_GetHeader(luos_phy_t *phy_robus, volatile uint8_t *data);
void Recep_GetData(luos_phy_t *phy_robus, volatile uint8_t *data);
void Recep_GetCollision(luos_phy_t *phy_robus, volatile uint8_t *data);
void Recep_Drop(luos_phy_t *phy_robus, volatile uint8_t *data);

// Callbacks send
void Recep_CatchAck(luos_phy_t *phy_robus, volatile uint8_t *data);

void Recep_Init(void);
void Recep_PhyInit(luos_phy_t *phy_robus);
void Recep_EndMsg(void);
void Recep_Reset(void);
void Recep_Timeout(void);
bool Recep_NodeConcerned(header_t *header);

#endif /* _RECEPTION_H_ */
