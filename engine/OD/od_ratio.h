/******************************************************************************
 * @file OD_ratio
 * @brief object dictionnary ratio
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_RATIO_H_
#define OD_OD_RATIO_H_

#include <string.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    float _private;
} ratio_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// ratio are stored in percentage (%)
//******** Conversions ***********

// percentage
static inline float RatioOD_RatioTo_Percent(ratio_t self)
{
    return self._private;
}

static inline ratio_t RatioOD_RatioFrom_Percent(float percentage)
{
    ratio_t self;
    self._private = percentage;
    return self;
}

//******** Messages management ***********
static inline void RatioOD_RatioToMsg(const ratio_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = RATIO;
    memcpy(msg->data, self, sizeof(ratio_t));
    msg->header.size = sizeof(ratio_t);
}

static inline void RatioOD_RatioFromMsg(ratio_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_RATIO_H_ */
