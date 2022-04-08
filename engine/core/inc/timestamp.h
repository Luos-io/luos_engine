/******************************************************************************
 * @file timestamp feature
 * @brief time stamp data
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include "stdbool.h"
#include "luos_list.h"
#include "service_structs.h"
#include "string.h"
#include "od_time.h"

/*******************************************************************************
 * Function
 ******************************************************************************/

time_luos_t Timestamp_now(void);
bool Timestamp_IsTimestampMsg(msg_t *msg);
time_luos_t Timestamp_GetTimestamp(msg_t *msg);

#endif /* _TIMESTAMP_H_ */
