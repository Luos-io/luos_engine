/******************************************************************************
 * @file reception
 * @brief reception state machine
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RECEPTION_H_
#define _RECEPTION_H_

#include <robus.h>
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
            uint8_t identifier : 4;
            uint8_t rx_error : 1;
            uint8_t rx_timeout : 1;
            uint8_t rx_framing_error : 1;
        };
        unsigned char unmap; /*!< Uncmaped form. */
    };
} status_t;

typedef struct
{
    DATA_CB callback;
    status_t status;
}
RxCom_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// Callbacks reception
void Recep_GetHeader(volatile unsigned char *data);
void Recep_GetData(volatile unsigned char *data);
void Recep_GetCollision(volatile unsigned char *data);

// Callbacks send
void Recep_CatchAck(volatile unsigned char *data);

void Recep_Init(void);
void Recep_EndMsg(void);
void Recep_Reset(void);
void Recep_Timeout(void);
void Recep_InterpretMsgProtocol(msg_t *msg);
uint8_t Recep_NodeConcerned(header_t *header);
vm_t *Recep_GetConcernedVm(header_t *header);

#endif /* _RECEPTION_H_ */
