/******************************************************************************
 * @file reception
 * @brief reception state machine
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _RECEPTION_H_
#define _RECEPTION_H_

#include "context.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// Callbacks reception
void get_header(volatile unsigned char *data);
void get_data(volatile unsigned char *data);
void get_collision(volatile unsigned char *data);

// Callbacks send
void catch_ack(volatile unsigned char *data);

void flush(void);
void timeout(void);
void msg_complete(msg_t *msg);
unsigned char module_concerned(header_t *header);

#endif /* _RECEPTION_H_ */
