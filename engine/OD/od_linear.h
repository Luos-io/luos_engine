/******************************************************************************
 * @file OD_linear
 * @brief object dictionnary linear
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_LINEAR_H_
#define OD_OD_LINEAR_H_

#include "io_struct.h"
#include "string.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    float _private;
} linear_position_t;

typedef struct
{
    float _private;
} linear_speed_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

// linear_position are stored in meter (m)
//******** Conversions ***********
// nm
static inline float LinearOD_PositionTo_nm(linear_position_t self)
{
    return self._private * 1000000000.0f;
}

static inline linear_position_t LinearOD_PositionFrom_nm(float nm)
{
    linear_position_t self;
    self._private = nm / 1000000000.0f;
    return self;
}

// um
static inline float LinearOD_PositionTo_um(linear_position_t self)
{
    return self._private * 1000000.0f;
}

static inline linear_position_t LinearOD_PositionFrom_um(float um)
{
    linear_position_t self;
    self._private = um / 1000000.0f;
    return self;
}

// mm
static inline float LinearOD_PositionTo_mm(linear_position_t self)
{
    return self._private * 1000.0f;
}

static inline linear_position_t LinearOD_PositionFrom_mm(float mm)
{
    linear_position_t self;
    self._private = mm / 1000.0f;
    return self;
}

// cm
static inline float LinearOD_PositionTo_cm(linear_position_t self)
{
    return self._private * 100.0f;
}

static inline linear_position_t LinearOD_PositionFrom_cm(float cm)
{
    linear_position_t self;
    self._private = cm / 100.0f;
    return self;
}

// m
static inline float LinearOD_PositionTo_m(linear_position_t self)
{
    return self._private;
}

static inline linear_position_t LinearOD_PositionFrom_m(float m)
{
    linear_position_t self;
    self._private = m;
    return self;
}

// km
static inline float LinearOD_PositionTo_km(linear_position_t self)
{
    return self._private / 1000.0f;
}

static inline linear_position_t LinearOD_PositionFrom_km(float km)
{
    linear_position_t self;
    self._private = km * 1000.0f;
    return self;
}

// inch
static inline float LinearOD_PositionTo_in(linear_position_t self)
{
    return self._private * 39.37007874015748f;
}

static inline linear_position_t LinearOD_PositionFrom_in(float in)
{
    linear_position_t self;
    self._private = in / 39.37007874015748f;
    return self;
}

// foot
static inline float LinearOD_PositionTo_ft(linear_position_t self)
{
    return self._private * 3.28084f;
}

static inline linear_position_t LinearOD_PositionFrom_ft(float ft)
{
    linear_position_t self;
    self._private = ft / 3.28084f;
    return self;
}

// mile
static inline float LinearOD_PositionTo_mi(linear_position_t self)
{
    return self._private / 1609.344f;
}

static inline linear_position_t LinearOD_PositionFrom_mi(float mi)
{
    linear_position_t self;
    self._private = mi * 1609.344f;
    return self;
}

//******** Messages management ***********
static inline void LinearOD_PositionToMsg(const linear_position_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = LINEAR_POSITION;
    memcpy(msg->data, self, sizeof(linear_position_t));
    msg->header.size = sizeof(linear_position_t);
}

static inline void LinearOD_PositionFromMsg(linear_position_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

// linear_speed are stored in meter per second (m_s)
//******** Conversions ***********

// mm_s
static inline float LinearOD_SpeedTo_mm_s(linear_speed_t self)
{
    return self._private * 1000.0f;
}

static inline linear_speed_t LinearOD_SpeedFrom_mm_s(float mm_s)
{
    linear_speed_t self;
    self._private = mm_s / 1000.0f;
    return self;
}

// m_s
static inline float LinearOD_SpeedTo_m_s(linear_speed_t self)
{
    return self._private;
}

static inline linear_speed_t LinearOD_SpeedFrom_m_s(float m_s)
{
    linear_speed_t self;
    self._private = m_s;
    return self;
}

// km_h
static inline float LinearOD_SpeedTo_km_h(linear_speed_t self)
{
    return (self._private * 3600.0f) / 1000.0f;
}

static inline linear_speed_t LinearOD_SpeedFrom_km_h(float km_h)
{
    linear_speed_t self;
    self._private = (km_h * 1000.0f) / 3600.0f;
    return self;
}

// in_s
static inline float LinearOD_SpeedTo_in_s(linear_speed_t self)
{
    return self._private * 39.37007874015748;
}

static inline linear_speed_t LinearOD_SpeedFrom_in_s(float in_s)
{
    linear_speed_t self;

    self._private = in_s / 39.37007874015748;
    return self;
}

// mi_h
static inline float LinearOD_SpeedTo_mi_h(linear_speed_t self)
{
    return self._private * 2.23694f;
}

static inline linear_speed_t LinearOD_SpeedFrom_mi_h(float mi_h)
{
    linear_speed_t self;
    self._private = mi_h / 2.23694f;
    return self;
}

//******** Messages management ***********
static inline void LinearOD_SpeedToMsg(const linear_speed_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = LINEAR_SPEED;
    memcpy(msg->data, self, sizeof(linear_speed_t));
    msg->header.size = sizeof(linear_speed_t);
}

static inline void LinearOD_SpeedFromMsg(linear_speed_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_LINEAR_H_ */
