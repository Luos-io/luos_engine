/******************************************************************************
 * @file OD_time
 * @brief object dictionnary time
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_TIME_H_
#define OD_OD_TIME_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef float time_luos_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// time values are stored in seconds (s)
//******** Conversions ***********

// sec
static inline float TimeOD_TimeTo_s(time_luos_t self)
{
    return self;
}

static inline time_luos_t TimeOD_TimeFrom_s(float sec)
{
    return sec;
}

// ms
static inline float TimeOD_TimeTo_ms(time_luos_t self)
{
    return self * 1000.0f;
}

static inline time_luos_t TimeOD_TimeFrom_ms(float ms)
{
    return ms / 1000.0f;
}

// µs
static inline float TimeOD_TimeTo_us(time_luos_t self)
{
    return self * 1000000.0f;
}

static inline time_luos_t TimeOD_TimeFrom_us(float us)
{
    return us / 1000000.0f;
}

// min
static inline float TimeOD_TimeTo_min(time_luos_t self)
{
    return self / 60.0f;
}

static inline time_luos_t TimeOD_TimeFrom_min(float min)
{
    return min * 60.0f;
}

// hour
static inline float TimeOD_TimeTo_h(time_luos_t self)
{
    return self / 3600.0f;
}

static inline time_luos_t TimeOD_TimeFrom_h(float hour)
{
    return hour * 3600.0f;
}

// day
static inline float TimeOD_TimeTo_day(time_luos_t self)
{
    return self / 86400.0f;
}

static inline time_luos_t TimeOD_TimeFrom_day(float day)
{
    return day * 86400.0f;
}

//******** Messages management ***********
static inline void TimeOD_TimeToMsg(const time_luos_t *const self, msg_t *const msg)
{
    msg->header.cmd = TIME;
    memcpy(msg->data, self, sizeof(time_luos_t));
    msg->header.size = sizeof(time_luos_t);
}

static inline void TimeOD_TimeFromMsg(time_luos_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_TIME_H_ */
