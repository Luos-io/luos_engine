/******************************************************************************
 * @file laser + galvo product_config.h
 * @brief A complete product example of a laser + galvo controler with Gate customization.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef PRODUCT_CONFIG_H
#define PRODUCT_CONFIG_H

#include "luos_engine.h"

// Definition of a custom service type
typedef enum
{
    POINT_2D = LUOS_LAST_TYPE,
    POWER_TYPE
} custom_service_type_t;

// Definition of a custom service command
typedef enum
{
    LINEAR_POSITION_2D = LUOS_LAST_STD_CMD, // uint16_t x, uint16_t y
    BUFFER_MODE                             // 3 different modes: 0: SINGLE, 1: CONTINUOUS, 2: STREAM
} custom_service_cmd_t;

typedef enum
{
    SINGLE,
    CONTINUOUS,
    STREAM
} buffer_mode_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
} __attribute__((packed)) pos_2d_t;

#endif /* PRODUCT_CONFIG_H */
