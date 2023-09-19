/******************************************************************************
 * @file OD_pressure
 * @brief object dictionnary pressure
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_PRESSURE_H_
#define OD_OD_PRESSURE_H_

#include <string.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    float raw;
} pressure_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// pressure values are stored in Pascals (Pa)
//******** Conversions ***********

// Pa
static inline float PressureOD_PressureTo_Pa(pressure_t self)
{
    return self.raw;
}

static inline pressure_t PressureOD_PressureFrom_Pa(float Pa)
{
    pressure_t self;
    self.raw = Pa;
    return self;
}

// Bars
static inline float PressureOD_PressureTo_Bars(pressure_t self)
{
    return self.raw / 100000.0f;
}

static inline pressure_t PressureOD_PressureFrom_Bars(float Bars)
{
    pressure_t self;
    self.raw = Bars * (float)100000.0f;
    return self;
}

// hPa
static inline float PressureOD_PressureTo_hPa(pressure_t self)
{
    return self.raw / (float)100.0f;
}

static inline pressure_t PressureOD_PressureFrom_hPa(float hPa)
{
    pressure_t self;
    self.raw = hPa * 100.0f;
    return self;
}

//******** Messages management ***********
static inline void PressureOD_PressureToMsg(const pressure_t *const self, msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    msg->header.cmd = PRESSURE;
    memcpy(msg->data, self, sizeof(pressure_t));
    msg->header.size = sizeof(pressure_t);
}

static inline void PressureOD_PressureFromMsg(pressure_t *const self, const msg_t *const msg)
{
    LUOS_ASSERT(self);
    LUOS_ASSERT(msg);
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_PRESSURE_H_ */
