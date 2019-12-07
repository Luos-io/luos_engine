/*
 * od_force.h
 *
 *  Created on: 7 déc. 2019
 *      Author: nicolasrabault
 */

#ifndef OD_OD_FORCE_H_
#define OD_OD_FORCE_H_

typedef float moment_t;

// moment are stored in Newton meter (Nm)
//******** Conversions ***********

// Ncm
static inline float moment_to_ncm(moment_t self) {
    return self * 100.0f;
}

static inline moment_t moment_from_ncm(float ncm) {
    return ncm / 100.0f;
}

// Nm
static inline float moment_to_nm(moment_t self) {
    return self;
}

static inline moment_t moment_from_nm(float nm) {
    return nm;
}

// kgf.cm
static inline float moment_to_kgf_cm(moment_t self) {
    return self * 10.2f;
}

static inline moment_t moment_from_kgf_cm(float kgf_cm) {
    return kgf_cm / 10.2f;
}

// kgf.m
static inline float moment_to_kgf_m(moment_t self) {
    return self * 0.102f;
}

static inline moment_t moment_from_kgf_m(float kgf_m) {
    return kgf_m / 0.102f;
}

//******** Messages management ***********
static inline void moment_to_msg(const moment_t* const self, msg_t* const msg) {
    msg->header.cmd = MOMENT;
    memcpy(msg->data, self, sizeof(moment_t));
    msg->header.size = sizeof(moment_t);
}

static inline void moment_from_msg(moment_t* const self, const msg_t* const msg) {
    memcpy(self, msg->data, msg->header.size);
}

typedef float force_t;

// force are stored in Newton (N)
//******** Conversions ***********

// N
static inline float force_to_n(force_t self) {
    return self;
}

static inline force_t force_from_n(float n) {
    return n;
}

// kgf
static inline float force_to_kgf(force_t self) {
    return self * 0.102f;
}

static inline force_t force_from_kgf(float kgf) {
    return kgf / 0.102f;
}

//******** Messages management ***********
static inline void force_to_msg(const force_t* const self, msg_t* const msg) {
    msg->header.cmd = FORCE;
    memcpy(msg->data, self, sizeof(force_t));
    msg->header.size = sizeof(force_t);
}

static inline void force_from_msg(force_t* const self, const msg_t* const msg) {
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_FORCE_H_ */
