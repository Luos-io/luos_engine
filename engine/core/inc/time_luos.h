/******************************************************************************
 * @file OD_time
 * @brief object dictionnary time
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TIME_LUOS_H_
#define TIME_LUOS_H_

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

#endif /* TIME_LUOS_H_ */
