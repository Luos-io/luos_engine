/*
 * od_ratio.h
 *
 *  Created on: 7 déc. 2019
 *      Author: nicolasrabault
 */

#ifndef OD_OD_RATIO_H_
#define OD_OD_RATIO_H_

typedef float ratio_t;

// ratio are stored in percentage (%)
//******** Conversions ***********

// percentage
static inline float ratio_to_percentage(ratio_t self)
{
    return self;
}

static inline ratio_t ratio_from_percentage(float percentage)
{
    return percentage;
}

//******** Messages management ***********
static inline void ratio_to_msg(const ratio_t *const self, msg_t *const msg)
{
    msg->header.cmd = RATIO;
    memcpy(msg->data, self, sizeof(ratio_t));
    msg->header.size = sizeof(ratio_t);
}

static inline void ratio_from_msg(ratio_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_RATIO_H_ */
