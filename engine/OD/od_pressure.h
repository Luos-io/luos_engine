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
typedef float pressure_t;

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
    return self;
}

static inline pressure_t PressureOD_PressureFrom_Pa(float Pa)
{
    return Pa;
}

// Bars
static inline float PressureOD_PressureTo_Bars(pressure_t self)
{
    return self / 10000.0f;
}

static inline pressure_t PressureOD_PressureFrom_Bars(float Bars)
{
    return Bars * (float)10000;
}

// hPa
static inline float PressureOD_PressureTo_hPa(pressure_t self)
{
    return self / (float)100;
}

static inline pressure_t PressureOD_PressureFrom_hPa(float hPa)
{
    return hPa * 100;
}

//******** Messages management ***********
static inline void PressureOD_PressureToMsg(const pressure_t *const self, msg_t *const msg)
{
    msg->header.cmd = PRESSURE;
    memcpy(msg->data, self, sizeof(pressure_t));
    msg->header.size = sizeof(pressure_t);
}

static inline void PressureOD_PressureFromMsg(pressure_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_PRESSURE_H_ */