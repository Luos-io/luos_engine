/******************************************************************************
 * @file pipe_link
 * @brief Manage the communication with a pipe.
 * @author Luos
 ******************************************************************************/
#ifndef PIPE_LINK_H
#define PIPE_LINK_H

#include "luos_engine.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void PipeLink_Send(service_t *service, void *data, uint32_t size);
uint16_t PipeLink_Find(service_t *service);
void PipeLink_Reset(service_t *service);
uint16_t PipeLink_GetId(void);
void PipeLink_SetDirectPipeSend(void *PipeSend);

#endif /* PIPE_LINK_H */
