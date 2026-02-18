/******************************************************************************
 * @file OD_force
 * @brief object dictionnary force
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_FORCE_H_
#define OD_OD_FORCE_H_

#include <string.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    float raw;
} torque_t;

typedef struct
{
    float raw;
} force_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

// torque are stored in Newton meter (Nm)
//******** Conversions ***********

// N.mm
static inline float ForceOD_TorqueTo_N_mm(torque_t self)
{
    return self.raw * 1000.0f;
}

static inline torque_t ForceOD_TorqueFrom_N_mm(float n_mm)
{
    torque_t self;
    self.raw = n_mm / 1000.0f;
    return self;
}

// N.cm
static inline float ForceOD_TorqueTo_N_cm(torque_t self)
{
    return self.raw * 100.0f;
}

static inline torque_t ForceOD_TorqueFrom_N_cm(float n_cm)
{
    torque_t self;
    self.raw = n_cm / 100.0f;
    return self;
}

// N.m
static inline float ForceOD_TorqueTo_N_m(torque_t self)
{
    return self.raw;
}

static inline torque_t ForceOD_TorqueFrom_N_m(float n_m)
{
    torque_t self;
    self.raw = n_m;
    return self;
}

// kgf.mm
static inline float ForceOD_TorqueTo_kgf_mm(torque_t self)
{
    return self.raw * 101.97f;
}

static inline torque_t ForceOD_TorqueFrom_kgf_mm(float kgf_mm)
{
    torque_t self;
    self.raw = kgf_mm / 101.97f;
    return self;
}

// kgf.cm
static inline float ForceOD_TorqueTo_kgf_cm(torque_t self)
{
    return self.raw * 10.2f;
}

static inline torque_t ForceOD_TorqueFrom_kgf_cm(float kgf_cm)
{
    torque_t self;
    self.raw = kgf_cm / 10.2f;
    return self;
}

// kgf.m
static inline float ForceOD_TorqueTo_kgf_m(torque_t self)
{
    return self.raw * 0.102f;
}

static inline torque_t ForceOD_TorqueFrom_kgf_m(float kgf_m)
{
    torque_t self;
    self.raw = kgf_m / 0.102f;
    return self;
}

// ozf.in
static inline float ForceOD_TorqueTo_ozf_in(torque_t self)
{
    return self.raw * 141.612f;
}

static inline torque_t ForceOD_TorqueFrom_ozf_in(float ozf_in)
{
    torque_t self;
    self.raw = ozf_in / 141.612f;
    return self;
}

// lbf.in
static inline float ForceOD_TorqueTo_lbf_in(torque_t self)
{
    return self.raw * 8.851f;
}

static inline torque_t ForceOD_TorqueFrom_lbf_in(float lbf_in)
{
    torque_t self;
    self.raw = lbf_in / 8.851f;
    return self;
}

//******** Messages management ***********
static inline void ForceOD_TorqueToMsg(const torque_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = TORQUE;
    memcpy(msg->data, self, sizeof(torque_t));
    msg->header.size = sizeof(torque_t);
}

static inline void ForceOD_TorqueFromMsg(torque_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

// force are stored in Newton (N)
//******************************** Conversions *******************************

// N
static inline float ForceOD_ForceTo_N(force_t self)
{
    return self.raw;
}

static inline force_t ForceOD_ForceFrom_N(float n)
{
    force_t self;
    self.raw = n;
    return self;
}

// kgf
static inline float ForceOD_ForceTo_kgf(force_t self)
{
    return self.raw * 0.102f;
}

static inline force_t ForceOD_ForceFrom_kgf(float kgf)
{
    force_t self;
    self.raw = kgf / 0.102f;
    return self;
}

// ozf
static inline float ForceOD_ForceTo_ozf(force_t self)
{
    return self.raw * 3.5969430896f;
}

static inline force_t ForceOD_ForceFrom_ozf(float ozf)
{
    force_t self;
    self.raw = ozf / 3.5969430896f;
    return self;
}

// lbf
static inline float ForceOD_ForceTo_lbf(force_t self)
{
    return self.raw * 0.2248089431f;
}

static inline force_t ForceOD_ForceFrom_lbf(float lbf)
{
    force_t self;
    self.raw = lbf / 0.2248089431f;
    return self;
}

//******** Messages management ***********
static inline void ForceOD_ForceToMsg(const force_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = FORCE;
    memcpy(msg->data, self, sizeof(force_t));
    msg->header.size = sizeof(force_t);
}

static inline void ForceOD_ForceFromMsg(force_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_FORCE_H_ */
