/******************************************************************************
 * @file OD_control
 * @brief object dictionnary managing flux control commands
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_CONTROL_H_
#define OD_OD_CONTROL_H_

#include "luos.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * Control modes
 */
typedef enum
{
    PLAY,
    PAUSE,
    STOP
} control_type_t;
/*
 * controle
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            // control modes
            uint8_t flux : 2;
            uint8_t rec : 1;
        };
        uint8_t unmap;
    };
} control_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// There is no conversion possible for PID
//******** Conversions ***********

//******** Messages management ***********
static inline void ControlOD_ControlToMsg(const control_t *const self, msg_t *const msg)
{
    msg->header.cmd = CONTROL;
    memcpy(msg->data, self, sizeof(control_t));
    msg->header.size = sizeof(control_t);
}

static inline void ControlOD_ControlFromMsg(control_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(msg->header.size == sizeof(control_t));
    memcpy(self, msg->data, sizeof(control_t));
    // check data validity
    if (self->flux == 3)
    {
        // impossible value, go back to default values
        self->unmap = 0;
    }
}

#endif /* OD_OD_CONTROL_H_ */
