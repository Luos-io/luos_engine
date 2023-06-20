/******************************************************************************
 * @file luos_io.c
 * @brief Interface file between Luos and a physical layer
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOSIO_H_
#define _LUOSIO_H_

#include "struct_luos.h"
#include "struct_phy.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

// generic functions
void LuosIO_Init(void);
void LuosIO_Loop(void);
uint16_t LuosIO_TopologyDetection(service_t *service, connection_t *connection_table);
error_return_t LuosIO_Send(service_t *service, msg_t *msg);

// Job management
error_return_t LuosIO_GetNextJob(phy_job_t **job);
void LuosIO_RmJob(phy_job_t *job);
uint16_t LuosIO_GetJobNb(void);
error_return_t LuosIO_TxAllComplete(void);

// Specific phy treatment
error_return_t Phy_TxAllComplete(void);
port_t *Phy_GetTopologysource(void);

#endif /* _LUOSIO_H_ */
