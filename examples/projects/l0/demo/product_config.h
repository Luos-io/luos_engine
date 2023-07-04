/******************************************************************************
 * @file product_config
 * @brief The official Luos demo
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PRODUCT_CONFIG_H
#define PRODUCT_CONFIG_H
#include "luos_engine.h"
#include "robus_network.h"

enum
{
    LEDSTRIP_POSITION_APP = LUOS_LAST_TYPE,
    RUN_MOTOR,
    DETECTION_BUTTON_APP,
    COPY_MOTOR
};

enum
{
    NO_MOTOR,
    MOTOR_1_POSITION,
    MOTOR_2_POSITION,
    MOTOR_3_POSITION,
};

typedef enum
{
    DISTANCE_DISPLAY,
    MOTOR_COPY_DISPLAY
} ledstrip_position_OperationMode_t;

/*
 * Select the operation mode of the ledstrip_position app
 */
typedef struct
{
    ledstrip_position_OperationMode_t parameter; // Default value is DISTANCE_DISPLAY
} ledstrip_position_Parameter_t;

#endif // PRODUCT_CONFIG_H
