/******************************************************************************
 * @file OD_time
 * @brief object dictionnary time
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_TIME_H_
#define OD_OD_TIME_H_

#include <string.h>
#include "time_luos.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

//******** Messages management ***********
static inline void TimeOD_TimeToMsg(const time_luos_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = TIME;
    memcpy(msg->data, self, sizeof(time_luos_t));
    msg->header.size = sizeof(time_luos_t);
}

static inline void TimeOD_TimeFromMsg(time_luos_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_TIME_H_ */
