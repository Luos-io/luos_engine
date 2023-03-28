/******************************************************************************
 * @file OD_angular
 * @brief object dictionnary angular
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_ANGULAR_H_
#define OD_OD_ANGULAR_H_

#include <string.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    float _private;
} angular_position_t;

typedef struct
{
    float _private;
} angular_speed_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// angular_position are stored in degree (deg)
//******** Conversions ***********

// deg
static inline float AngularOD_PositionTo_deg(angular_position_t self)
{
    return self._private;
}

static inline angular_position_t AngularOD_PositionFrom_deg(float deg)
{
    angular_position_t self;
    self._private = deg;
    return self;
}

// rev
static inline float AngularOD_PositionTo_rev(angular_position_t self)
{
    return self._private / 360.0f;
}

static inline angular_position_t AngularOD_PositionFrom_rev(float rev)
{
    angular_position_t self;
    self._private = rev * 360.0f;
    return self;
}

// rad
static inline float AngularOD_PositionTo_rad(angular_position_t self)
{
    return (self._private * 2.0f * 3.141592653589793f) / 360.0f;
}

static inline angular_position_t AngularOD_PositionFrom_rad(float rad)
{
    angular_position_t self;
    self._private = (rad * 360.0f) / (2.0f * 3.141592653589793f);
    return self;
}

//******** Messages management ***********
static inline void AngularOD_PositionToMsg(const angular_position_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = ANGULAR_POSITION;
    memcpy(msg->data, self, sizeof(angular_position_t));
    msg->header.size = sizeof(angular_position_t);
}

static inline void AngularOD_PositionFromMsg(angular_position_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

// angular_speed are stored in degree/s (deg/s)
//******************************** Conversions *******************************

// deg_s
static inline float AngularOD_SpeedTo_deg_s(angular_speed_t self)
{
    return self._private;
}

static inline angular_speed_t AngularOD_SpeedFrom_deg_s(float deg)
{
    angular_speed_t self;
    self._private = deg;
    return self;
}

// rev_s
static inline float AngularOD_SpeedTo_rev_s(angular_speed_t self)
{
    return self._private / 360.0f;
}

static inline angular_speed_t AngularOD_SpeedFrom_rev_s(float rev_s)
{
    angular_speed_t self;
    self._private = rev_s * 360.0f;
    return self;
}

// rev_min
static inline float AngularOD_SpeedTo_rev_min(angular_speed_t self)
{
    return self._private * 60.0f / 360.0f;
}

static inline angular_speed_t AngularOD_SpeedFrom_rev_min(float rev_min)
{
    angular_speed_t self;
    self._private = rev_min * 360.0f / 60.0f;
    return self;
}

// rad_s
static inline float AngularOD_SpeedTo_rad_s(angular_speed_t self)
{
    return (self._private * 2.0f * 3.141592653589793f) / 360.0f;
}

static inline angular_speed_t AngularOD_SpeedFrom_rad_s(float rad_s)
{
    angular_speed_t self;
    self._private = (rad_s * 360.0f) / (2.0f * 3.141592653589793f);
    return self;
}

//******** Messages management ***********
static inline void AngularOD_SpeedToMsg(const angular_speed_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = ANGULAR_SPEED;
    memcpy(msg->data, self, sizeof(angular_speed_t));
    msg->header.size = sizeof(angular_speed_t);
}

static inline void AngularOD_SpeedFromMsg(angular_speed_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_ANGULAR_H_ */
