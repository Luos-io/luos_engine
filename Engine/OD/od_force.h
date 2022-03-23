/******************************************************************************
 * @file OD_force
 * @brief object dictionnary force
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_FORCE_H_
#define OD_OD_FORCE_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef float moment_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

// moment are stored in Newton meter (Nm)
//******** Conversions ***********

// N.mm
static inline float ForceOD_MomentTo_N_mm(moment_t self)
{
    return self * 1000.0f;
}

static inline moment_t ForceOD_MomentFrom_N_mm(float n_mm)
{
    return n_mm / 1000.0f;
}

// N.cm
static inline float ForceOD_MomentTo_N_cm(moment_t self)
{
    return self * 100.0f;
}

static inline moment_t ForceOD_MomentFrom_N_cm(float n_cm)
{
    return n_cm / 100.0f;
}

// N.m
static inline float ForceOD_MomentTo_N_m(moment_t self)
{
    return self;
}

static inline moment_t ForceOD_MomentFrom_N_m(float n_m)
{
    return n_m;
}

// kgf.mm
static inline float ForceOD_MomentTo_kgf_mm(moment_t self)
{
    return self * 101.97f;
}

static inline moment_t ForceOD_MomentFrom_kgf_mm(float kgf_mm)
{
    return kgf_mm / 101.97f;
}

// kgf.cm
static inline float ForceOD_MomentTo_kgf_cm(moment_t self)
{
    return self * 10.2f;
}

static inline moment_t ForceOD_MomentFrom_kgf_cm(float kgf_cm)
{
    return kgf_cm / 10.2f;
}

// kgf.m
static inline float ForceOD_MomentTo_kgf_m(moment_t self)
{
    return self * 0.102f;
}

static inline moment_t ForceOD_MomentFrom_kgf_m(float kgf_m)
{
    return kgf_m / 0.102f;
}

// ozf.in
static inline float ForceOD_MomentTo_ozf_in(moment_t self)
{
    return self * 141.612f;
}

static inline moment_t ForceOD_MomentFrom_ozf_in(float ozf_in)
{
    return ozf_in / 141.612f;
}

// lbf.in
static inline float ForceOD_MomentTo_lbf_in(moment_t self)
{
    return self * 8.851f;
}

static inline moment_t ForceOD_MomentFrom_lbf_in(float lbf_in)
{
    return lbf_in / 8.851f;
}

//******** Messages management ***********
static inline void ForceOD_MomentToMsg(const moment_t *const self, msg_t *const msg)
{
    msg->header.cmd = MOMENT;
    memcpy(msg->data, self, sizeof(moment_t));
    msg->header.size = sizeof(moment_t);
}

static inline void ForceOD_MomentFromMsg(moment_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

typedef float force_t;

// force are stored in Newton (N)
//******** Conversions ***********

// N
static inline float ForceOD_ForceTo_N(force_t self)
{
    return self;
}

static inline force_t ForceOD_ForceFrom_N(float n)
{
    return n;
}

// kgf
static inline float ForceOD_ForceTo_kgf(force_t self)
{
    return self * 0.102f;
}

static inline force_t ForceOD_ForceFrom_kgf(float kgf)
{
    return kgf / 0.102f;
}

// ozf
static inline float ForceOD_ForceTo_ozf(force_t self)
{
    return self * 141.612f;
}

static inline force_t ForceOD_ForceFrom_ozf(float ozf)
{
    return ozf / 141.612f;
}

// lbf
static inline float ForceOD_ForceTo_lbf(force_t self)
{
    return self * 8.851f;
}

static inline force_t ForceOD_ForceFrom_lbf(float lbf)
{
    return lbf / 8.851f;
}

//******** Messages management ***********
static inline void ForceOD_ForceToMsg(const force_t *const self, msg_t *const msg)
{
    msg->header.cmd = FORCE;
    memcpy(msg->data, self, sizeof(force_t));
    msg->header.size = sizeof(force_t);
}

static inline void ForceOD_ForceFromMsg(force_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_FORCE_H_ */
