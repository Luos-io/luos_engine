/******************************************************************************
 * @file OD_pid
 * @brief object dictionnary Proportional Integral Derivative (PID)
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_PID_H_
#define OD_OD_PID_H_

#include "luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * Pid
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            float p;
            float i;
            float d;
        };
        unsigned char unmap[3 * sizeof(float)];
        float table[3];
    };
} asserv_pid_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// There is no conversion possible for PID
//******** Conversions ***********

//******** Messages management ***********
static inline void PidOD_PidToMsg(const asserv_pid_t *const self, msg_t *const msg)
{
    msg->header.cmd = PID;
    memcpy(msg->data, self, sizeof(asserv_pid_t));
    msg->header.size = sizeof(asserv_pid_t);
}

static inline void PidOD_PidFromMsg(asserv_pid_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(msg->header.size == sizeof(asserv_pid_t));
    memcpy(self, msg->data, sizeof(asserv_pid_t));
}

#endif /* OD_OD_PID_H_ */
