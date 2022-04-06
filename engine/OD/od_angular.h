/******************************************************************************
 * @file OD_angular
 * @brief object dictionnary angular
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_ANGULAR_H_
#define OD_OD_ANGULAR_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef float angular_position_t;

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
    return self;
}

static inline angular_position_t AngularOD_PositionFrom_deg(float deg)
{
    return deg;
}

// rev
static inline float AngularOD_PositionTo_rev(angular_position_t self)
{
    return self / 360.0f;
}

static inline angular_position_t AngularOD_PositionFrom_rev(float rev)
{
    return rev * 360.0f;
}

// rad
static inline float AngularOD_PositionTo_rad(angular_position_t self)
{
    return (self * 2.0f * 3.141592653589793f) / 360.0f;
}

static inline angular_position_t AngularOD_PositionFrom_rad(float rad)
{
    return (rad * 360.0f) / (2.0f * 3.141592653589793f);
}

//******** Messages management ***********
static inline void AngularOD_PositionToMsg(const angular_position_t *const self, msg_t *const msg)
{
    msg->header.cmd = ANGULAR_POSITION;
    memcpy(msg->data, self, sizeof(angular_position_t));
    msg->header.size = sizeof(angular_position_t);
}

static inline void AngularOD_PositionFromMsg(angular_position_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

typedef float angular_speed_t;

// angular_speed are stored in degree/s (deg/s)
//******** Conversions ***********

// deg_s
static inline float AngularOD_SpeedTo_deg_s(angular_speed_t self)
{
    return self;
}

static inline angular_speed_t AngularOD_SpeedFrom_deg_s(float deg)
{
    return deg;
}

// rev_s
static inline float AngularOD_SpeedTo_rev_s(angular_speed_t self)
{
    return self / 360.0f;
}

static inline angular_speed_t AngularOD_SpeedFrom_rev_s(float rev_s)
{
    return rev_s * 360.0f;
}

// rev_min
static inline float AngularOD_SpeedTo_rev_min(angular_speed_t self)
{
    return self * 60.0f / 360.0f;
}

static inline angular_speed_t AngularOD_SpeedFrom_rev_min(float rev_min)
{
    return rev_min * 360.0f / 60.0f;
}

// rad_s
static inline float AngularOD_SpeedTo_rad_s(angular_speed_t self)
{
    return (self * 2.0f * 3.141592653589793f) / 360.0f;
}

static inline angular_speed_t AngularOD_SpeedFrom_rad_s(float rad_s)
{
    return (rad_s * 360.0f) / (2.0f * 3.141592653589793f);
}

//******** Messages management ***********
static inline void AngularOD_SpeedToMsg(const angular_speed_t *const self, msg_t *const msg)
{
    msg->header.cmd = ANGULAR_SPEED;
    memcpy(msg->data, self, sizeof(angular_speed_t));
    msg->header.size = sizeof(angular_speed_t);
}

static inline void AngularOD_SpeedFromMsg(angular_speed_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_ANGULAR_H_ */
