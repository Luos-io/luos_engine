/*
 * od_electric.h
 *
 *  Created on: 7 déc. 2019
 *      Author: nicolasrabault
 */

#ifndef OD_OD_ELECTRIC_H_
#define OD_OD_ELECTRIC_H_

typedef float voltage_t;

// voltage are stored in volt (v)
//******** Conversions ***********

// mv
static inline float voltage_to_mv(voltage_t self)
{
    return self * 1000.0f;
}

static inline voltage_t voltage_from_mv(float mv)
{
    return mv / 1000.0f;
}

// v
static inline float voltage_to_v(voltage_t self)
{
    return self;
}

static inline voltage_t voltage_from_v(float v)
{
    return v;
}

//******** Messages management ***********
static inline void voltage_to_msg(const voltage_t *const self, msg_t *const msg)
{
    msg->header.cmd = VOLTAGE;
    memcpy(msg->data, self, sizeof(voltage_t));
    msg->header.size = sizeof(voltage_t);
}

static inline void voltage_from_msg(voltage_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

typedef float current_t;

// current are stored in Ampere (A)
//******** Conversions ***********

// ma
static inline float current_to_ma(current_t self)
{
    return self * 1000.0f;
}

static inline current_t current_from_ma(float ma)
{
    return ma / 1000.0f;
}

// A
static inline float current_to_a(current_t self)
{
    return self;
}

static inline current_t current_from_a(float a)
{
    return a;
}

//******** Messages management ***********
static inline void current_to_msg(const current_t *const self, msg_t *const msg)
{
    msg->header.cmd = CURRENT;
    memcpy(msg->data, self, sizeof(current_t));
    msg->header.size = sizeof(current_t);
}

static inline void current_from_msg(current_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

typedef float power_t;

// power are stored in Watt (W)
//******** Conversions ***********

// mw
static inline float power_to_mw(power_t self)
{
    return self * 1000.0f;
}

static inline power_t power_from_mw(float mw)
{
    return mw / 1000.0f;
}

// A
static inline float power_to_w(power_t self)
{
    return self;
}

static inline power_t power_from_w(float w)
{
    return w;
}

//******** Messages management ***********
static inline void power_to_msg(const power_t *const self, msg_t *const msg)
{
    msg->header.cmd = POWER;
    memcpy(msg->data, self, sizeof(power_t));
    msg->header.size = sizeof(power_t);
}

static inline void power_from_msg(current_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_ELECTRIC_H_ */
