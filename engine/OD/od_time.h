/******************************************************************************
 * @file OD_time
 * @brief object dictionnary time
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_TIME_H_
#define OD_OD_TIME_H_

#include <string.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    double _private;
} time_luos_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// time values are stored in seconds (s)
//******** Conversions ***********

// sec
static inline double TimeOD_TimeTo_s(time_luos_t self)
{
    return self._private;
}

static inline time_luos_t TimeOD_TimeFrom_s(double sec)
{
    time_luos_t self;
    self._private = sec;
    return self;
}

// ms
static inline double TimeOD_TimeTo_ms(time_luos_t self)
{
    return self._private * 1000.0f;
}

static inline time_luos_t TimeOD_TimeFrom_ms(double ms)
{
    time_luos_t self;
    self._private = ms / 1000.0f;
    return self;
}

// µs
static inline double TimeOD_TimeTo_us(time_luos_t self)
{
    return self._private * 1000000.0f;
}

static inline time_luos_t TimeOD_TimeFrom_us(double us)
{
    time_luos_t self;
    self._private = us / 1000000.0f;
    return self;
}

// ns
static inline double TimeOD_TimeTo_ns(time_luos_t self)
{
    return self._private * 1000000000.0f;
}

static inline time_luos_t TimeOD_TimeFrom_ns(double ns)
{
    time_luos_t self;
    self._private = ns / 1000000000.0f;
    return self;
}

// min
static inline double TimeOD_TimeTo_min(time_luos_t self)
{
    return self._private / 60.0f;
}

static inline time_luos_t TimeOD_TimeFrom_min(double min)
{
    time_luos_t self;
    self._private = min * 60.0f;
    return self;
}

// hour
static inline double TimeOD_TimeTo_h(time_luos_t self)
{
    return self._private / 3600.0f;
}

static inline time_luos_t TimeOD_TimeFrom_h(double hour)
{
    time_luos_t self;
    self._private = hour * 3600.0f;
    return self;
}

// day
static inline double TimeOD_TimeTo_day(time_luos_t self)
{
    return self._private / 86400.0f;
}

static inline time_luos_t TimeOD_TimeFrom_day(double day)
{
    time_luos_t self;
    self._private = day * 86400.0f;
    return self;
}

//******** Messages management ***********
static inline void TimeOD_TimeToMsg(const time_luos_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = TIME;
    memcpy(msg->data, self, sizeof(time_luos_t));
    msg->header.size = sizeof(time_luos_t);
}

static inline void TimeOD_TimeFromMsg(time_luos_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_TIME_H_ */
