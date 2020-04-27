/*
 * od_time.h
 *
 *  Created on: 16 dec 2019
 *      Author: simonbdy
 */

#ifndef OD_OD_TIME_H_
#define OD_OD_TIME_H_

typedef float time_luos_t;

// time values are stored in seconds (s)
//******** Conversions ***********

// sec
static inline float time_to_sec(time_luos_t self) {
    return self;
}

static inline time_luos_t time_from_sec(float sec) {
    return sec;
}

// ms
static inline float time_to_ms(time_luos_t self) {
    return self * 1000.0f;
}

static inline time_luos_t time_from_ms(float ms) {
    return ms / 1000.0f;
}

// µs
static inline float time_to_us(time_luos_t self) {
    return self * 1000000.0f;
}

static inline time_luos_t time_from_us(float us) {
    return us / 1000000.0f;
}

// min
static inline float time_to_min(time_luos_t self) {
    return self / 60.0f;
}

static inline time_luos_t time_from_min(float min) {
    return min * 60.0f;
}

// hour
static inline float time_to_hour(time_luos_t self) {
    return self / 3600.0f;
}

static inline time_luos_t time_from_hour(float hour) {
    return hour * 3600.0f;
}

// day
static inline float time_to_day(time_luos_t self) {
    return self / 86400.0f;
}

static inline time_luos_t time_from_day(float day) {
    return day * 86400.0f;
}

//******** Messages management ***********
static inline void time_to_msg(const time_luos_t* const self, msg_t* const msg) {
    msg->header.cmd = TIME;
    memcpy(msg->data, self, sizeof(time_luos_t));
    msg->header.size = sizeof(time_luos_t);
}

static inline void time_from_msg(time_luos_t* const self, const msg_t* const msg) {
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_TIME_H_ */
