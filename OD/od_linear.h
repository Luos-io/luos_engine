/*
 * od_linear.h
 *
 *  Created on: 2 déc. 2019
 *      Author: nicolasrabault
 */

#ifndef OD_OD_LINEAR_H_
#define OD_OD_LINEAR_H_

#include "robus_struct.h"
#include "string.h"

typedef float linear_position_t;

// linear_position are stored in meter (m)
//******** Conversions ***********
// nm
static inline float linear_position_to_nm(linear_position_t self)
{
    return self * 1000000000.0f;
}

static inline linear_position_t linear_position_from_nm(float nm)
{
    return nm / 1000000000.0f;
}

// um
static inline float linear_position_to_um(linear_position_t self)
{
    return self * 1000000.0f;
}

static inline linear_position_t linear_position_from_um(float um)
{
    return um / 1000000.0f;
}

// mm
static inline float linear_position_to_mm(linear_position_t self)
{
    return self * 1000.0f;
}

static inline linear_position_t linear_position_from_mm(float mm)
{
    return mm / 1000.0f;
}

// cm
static inline float linear_position_to_cm(linear_position_t self)
{
    return self * 100.0f;
}

static inline linear_position_t linear_position_from_cm(float cm)
{
    return cm / 100.0f;
}

// m
static inline float linear_position_to_m(linear_position_t self)
{
    return self;
}

static inline linear_position_t linear_position_from_m(float m)
{
    return m;
}

// km
static inline float linear_position_to_km(linear_position_t self)
{
    return self / 1000.0f;
}

static inline linear_position_t linear_position_from_km(float km)
{
    return km * 1000.0f;
}

// inch
static inline float linear_position_to_in(linear_position_t self)
{
    return self * 254.0f;
}

static inline linear_position_t linear_position_from_in(float in)
{
    return in / 254.0f;
}

// foot
static inline float linear_position_to_ft(linear_position_t self)
{
    return self * 3048.0f;
}

static inline linear_position_t linear_position_from_ft(float ft)
{
    return ft / 3048.0f;
}

// mile
static inline float linear_position_to_mi(linear_position_t self)
{
    return self / 1609.344f;
}
static inline linear_position_t linear_position_from_mi(float mi)
{
    return mi * 1609.344f;
}

//******** Messages management ***********
static inline void linear_position_to_msg(const linear_position_t *const self, msg_t *const msg)
{
    msg->header.cmd = LINEAR_POSITION;
    memcpy(msg->data, self, sizeof(linear_position_t));
    msg->header.size = sizeof(linear_position_t);
}

static inline void linear_position_from_msg(linear_position_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

typedef float linear_speed_t;

// linear_speed are stored in meter per second (m_s)
//******** Conversions ***********

// mm_s
static inline float linear_speed_to_mm_s(linear_speed_t self)
{
    return self * 1000.0f;
}

static inline linear_speed_t linear_speed_from_mm_s(float mm_s)
{
    return mm_s / 1000.0f;
}

// m_s
static inline float linear_speed_to_m_s(linear_speed_t self)
{
    return self;
}

static inline linear_speed_t linear_speed_from_m_s(float m_s)
{
    return m_s;
}

// km_h
static inline float linear_speed_to_km_h(linear_speed_t self)
{
    return (self * 3600.0f) / 1000.0f;
}

static inline linear_speed_t linear_speed_from_km_h(float km_h)
{
    return (km_h * 1000.0f) / 3600.0f;
}

// in_s
static inline float linear_speed_to_in_s(linear_speed_t self)
{
    return (self * 3600.0f) / 254.0f;
}

static inline linear_speed_t linear_speed_from_in_s(float in_s)
{
    return (in_s * 254.0f) / 3600.0f;
}

// mi_h
static inline float linear_speed_to_mi_h(linear_speed_t self)
{
    return (self * 3600.0f) / 254.0f;
}

static inline linear_speed_t linear_speed_from_mi_h(float mi_h)
{
    return (mi_h * 1609.344f) / 3600.0f;
}

//******** Messages management ***********
static inline void linear_speed_to_msg(const linear_speed_t *const self, msg_t *const msg)
{
    msg->header.cmd = LINEAR_SPEED;
    memcpy(msg->data, self, sizeof(linear_speed_t));
    msg->header.size = sizeof(linear_speed_t);
}

static inline void linear_speed_from_msg(linear_speed_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_LINEAR_H_ */
