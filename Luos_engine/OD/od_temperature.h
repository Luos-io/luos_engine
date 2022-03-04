/******************************************************************************
 * @file OD_temperature
 * @brief object dictionnary temperature
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_TEMPERATURE_H_
#define OD_OD_TEMPERATURE_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef float temperature_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// temperature are stored in degree celcius (°C)
//******** Conversions ***********

// °C
static inline float TemperatureOD_TemperatureTo_deg_c(temperature_t self)
{
    return self;
}

static inline temperature_t TemperatureOD_TemperatureFrom_deg_c(float deg_c)
{
    return deg_c;
}

// °F
static inline float TemperatureOD_TemperatureTo_deg_f(temperature_t self)
{
    return self * 1.8f + 32.0f;
}

static inline temperature_t TemperatureOD_TemperatureFrom_deg_f(float deg_f)
{
    return (deg_f - 32.0f) / 1.8f;
}

// °K
static inline float TemperatureOD_TemperatureTo_deg_k(temperature_t self)
{
    return self + 273.15f;
}

static inline temperature_t TemperatureOD_TemperatureFrom_deg_k(float deg_k)
{
    return deg_k - 273.15f;
}

//******** Messages management ***********
static inline void TemperatureOD_TemperatureToMsg(const temperature_t *const self, msg_t *const msg)
{
    msg->header.cmd = TEMPERATURE;
    memcpy(msg->data, self, sizeof(temperature_t));
    msg->header.size = sizeof(temperature_t);
}

static inline void TemperatureOD_TemperatureFromMsg(temperature_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_TEMPERATURE_H_ */
