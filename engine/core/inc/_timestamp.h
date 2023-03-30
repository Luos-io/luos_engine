/******************************************************************************
 * @file timestamp feature
 * @brief time stamp data
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef __TIMESTAMP_H_
#define __TIMESTAMP_H_

#include "luos_engine.h"

/*******************************************************************************
 * Function
 ******************************************************************************/

void Timestamp_EncodeMsg(msg_t *msg, time_luos_t timestamp);
void Timestamp_ConvertToLatency(msg_t *msg);
void Timestamp_ConvertToDate(msg_t *msg, uint64_t reception_date);

#endif /* __TIMESTAMP_H_ */
