/******************************************************************************
 * @file OD_electric
 * @brief object dictionnary electric
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_ELECTRIC_H_
#define OD_OD_ELECTRIC_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef float voltage_t;
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
    return self * 1000.0f;
}

static inline voltage_t ElectricOD_VoltageFrom_mV(float mv)
{
    return mv / 1000.0f;
}

// v
static inline float ElectricOD_VoltageTo_V(voltage_t self)
{
    return self;
}

static inline voltage_t ElectricOD_VoltageFrom_V(float v)
{
    return v;
}

//******** Messages management ***********
static inline void ElectricOD_VoltageToMsg(const voltage_t *const self, msg_t *const msg)
{
    msg->header.cmd = VOLTAGE;
    memcpy(msg->data, self, sizeof(voltage_t));
    msg->header.size = sizeof(voltage_t);
}

static inline void ElectricOD_VoltageFromMsg(voltage_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

typedef float current_t;

// current are stored in Ampere (A)
//******** Conversions ***********

// ma
static inline float ElectricOD_CurrentTo_mA(current_t self)
{
    return self * 1000.0f;
}

static inline current_t ElectricOD_CurrentFrom_mA(float ma)
{
    return ma / 1000.0f;
}

// A
static inline float ElectricOD_CurrentTo_A(current_t self)
{
    return self;
}

static inline current_t ElectricOD_CurrentFrom_A(float a)
{
    return a;
}

//******** Messages management ***********
static inline void ElectricOD_CurrentToMsg(const current_t *const self, msg_t *const msg)
{
    msg->header.cmd = CURRENT;
    memcpy(msg->data, self, sizeof(current_t));
    msg->header.size = sizeof(current_t);
}

static inline void ElectricOD_CurrentFromMsg(current_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

typedef float power_t;

// power are stored in Watt (W)
//******** Conversions ***********

// mw
static inline float ElectricOD_PowerTo_mW(power_t self)
{
    return self * 1000.0f;
}

static inline power_t ElectricOD_PowerFrom_mW(float mw)
{
    return mw / 1000.0f;
}

// A
static inline float ElectricOD_PowerTo_W(power_t self)
{
    return self;
}

static inline power_t ElectricOD_PowerFrom_W(float w)
{
    return w;
}

//******** Messages management ***********
static inline void ElectricOD_PowerToMsg(const power_t *const self, msg_t *const msg)
{
    msg->header.cmd = POWER;
    memcpy(msg->data, self, sizeof(power_t));
    msg->header.size = sizeof(power_t);
}

static inline void ElectricOD_PowerFromMsg(current_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_ELECTRIC_H_ */
