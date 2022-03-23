/******************************************************************************
 * @file OD_illuminance
 * @brief object dictionnary illuminance
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef OD_OD_ILLUMINANCE_H_
#define OD_OD_ILLUMINANCE_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef float illuminance_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
// illuminance are stored in lux (lx or lm/m2)
//******** Conversions ***********

// lx
static inline float IlluminanceOD_IlluminanceTo_Lux(illuminance_t self)
{
    return self;
}

static inline illuminance_t IlluminanceOD_IlluminanceFrom_Lux(float lx)
{
    return lx;
}

//******** Messages management ***********
static inline void IlluminanceOD_IlluminanceToMsg(const illuminance_t *const self, msg_t *const msg)
{
    msg->header.cmd = ILLUMINANCE;
    memcpy(msg->data, self, sizeof(illuminance_t));
    msg->header.size = sizeof(illuminance_t);
}

static inline void IlluminanceOD_IlluminanceFromMsg(illuminance_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

// GPIO struct
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint8_t unmap[3]; /*!< Unmaped form. */
    };
} color_t;

// color are stored in RGB
//******** Messages management ***********
static inline void IlluminanceOD_ColorToMsg(const color_t *const self, msg_t *const msg)
{
    msg->header.cmd = COLOR;
    memcpy(msg->data, self, sizeof(color_t));
    msg->header.size = sizeof(color_t);
}

static inline void IlluminanceOD_ColorFromMsg(color_t *const self, const msg_t *const msg)
{
    memcpy(self, msg->data, msg->header.size);
}

#endif /* OD_OD_ILLUMINANCE_H_ */
