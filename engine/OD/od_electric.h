/******************************************************************************
 * @file OD_electric
 * @brief object dictionnary electric
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_ELECTRIC_H_
#define OD_OD_ELECTRIC_H_

#include <string.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    float raw;
} voltage_t;

typedef struct
{
    float raw;
} current_t;

typedef struct
{
    float raw;
} power_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// voltage are stored in volt (v)
//******** Conversions ***********

// mv
static inline float ElectricOD_VoltageTo_mV(voltage_t self)
{
    return self.raw * 1000.0f;
}

static inline voltage_t ElectricOD_VoltageFrom_mV(float mv)
{
    voltage_t self;
    self.raw = mv / 1000.0f;
    return self;
}

// v
static inline float ElectricOD_VoltageTo_V(voltage_t self)
{
    return self.raw;
}

static inline voltage_t ElectricOD_VoltageFrom_V(float v)
{
    voltage_t self;
    self.raw = v;
    return self;
}

//******** Messages management ***********
static inline void ElectricOD_VoltageToMsg(const voltage_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = VOLTAGE;
    memcpy(msg->data, self, sizeof(voltage_t));
    msg->header.size = sizeof(voltage_t);
}

static inline void ElectricOD_VoltageFromMsg(voltage_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

// current are stored in Ampere (A)
//******************************** Conversions *******************************

// ma
static inline float ElectricOD_CurrentTo_mA(current_t self)
{
    return self.raw * 1000.0f;
}

static inline current_t ElectricOD_CurrentFrom_mA(float ma)
{
    current_t self;
    self.raw = ma / 1000.0f;
    return self;
}

// A
static inline float ElectricOD_CurrentTo_A(current_t self)
{
    return self.raw;
}

static inline current_t ElectricOD_CurrentFrom_A(float a)
{
    current_t self;
    self.raw = a;
    return self;
}

//******** Messages management ***********
static inline void ElectricOD_CurrentToMsg(const current_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = CURRENT;
    memcpy(msg->data, self, sizeof(current_t));
    msg->header.size = sizeof(current_t);
}

static inline void ElectricOD_CurrentFromMsg(current_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

// power are stored in Watt (W)
//******************************** Conversions *******************************

// mw
static inline float ElectricOD_PowerTo_mW(power_t self)
{
    return self.raw * 1000.0f;
}

static inline power_t ElectricOD_PowerFrom_mW(float mw)
{
    power_t self;
    self.raw = mw / 1000.0f;
    return self;
}

// A
static inline float ElectricOD_PowerTo_W(power_t self)
{
    return self.raw;
}

static inline power_t ElectricOD_PowerFrom_W(float w)
{
    power_t self;
    self.raw = w;
    return self;
}

//******** Messages management ***********
static inline void ElectricOD_PowerToMsg(const power_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = POWER;
    memcpy(msg->data, self, sizeof(power_t));
    msg->header.size = sizeof(power_t);
}

static inline void ElectricOD_PowerFromMsg(power_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_ELECTRIC_H_ */
