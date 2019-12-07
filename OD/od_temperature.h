/*
 * od_temperature.h
 *
 *  Created on: 7 déc. 2019
 *      Author: nicolasrabault
 */

#ifndef OD_OD_TEMPERATURE_H_
#define OD_OD_TEMPERATURE_H_

typedef float temperature_t;

// temperature are stored in degree celcius (°C)
//******** Conversions ***********

// °c
static inline float temperature_to_deg_c(temperature_t self) {
    return self;
}

static inline temperature_t temperature_from_deg_c(float deg_c) {
    return deg_c;
}

// °f
static inline float temperature_to_deg_f(temperature_t self) {
    return self * 33.8f;
}

static inline temperature_t temperature_from_deg_f(float deg_f) {
    return deg_f / 33.8f;
}

//******** Messages management ***********
static inline void temperature_to_msg(const temperature_t* const self, msg_t* const msg) {
    msg->header.cmd = TEMPERATURE;
    memcpy(msg->data, self, sizeof(temperature_t));
    msg->header.size = sizeof(temperature_t);
}

static inline void temperature_from_msg(temperature_t* const self, const msg_t* const msg) {
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_TEMPERATURE_H_ */
