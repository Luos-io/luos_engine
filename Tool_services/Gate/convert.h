/******************************************************************************
 * @file convert
 * @brief Functions allowing to manage data convertion to a specific format
 * @author Luos
 ******************************************************************************/
#ifndef CONVERT_H_
#define CONVERT_H_

#include "luos.h"

/*
 * Servo
 */
typedef struct
{
    union
    {
        struct __attribute__((__packed__))
        {
            angular_position_t max_angle;
            float min_pulse_time;
            float max_pulse_time;
        };
        unsigned char unmap[3 * sizeof(float)];
    };
} servo_parameters_t;

// Luos data to Luos messages convertion
void Convert_DataToLuos(service_t *service, char *data);

// Luos service information to Data convertion
uint16_t Convert_StartData(char *data);
uint16_t Convert_StartServiceData(char *data, char *alias);
uint16_t Convert_MsgToData(msg_t *msg, char *data);
uint16_t Convert_EndServiceData(char *data);
void Convert_EndData(service_t *service, char *data, char *data_ptr);
void Convert_VoidData(service_t *service);

// Luos default information to Data convertion
void Convert_AssertToData(service_t *service, uint16_t source, luos_assert_t assertion);
void Convert_ExcludedServiceData(service_t *service);

// Luos routing table information to Json convertion
void Convert_RoutingTableData(service_t *service);

#endif /* CONVERT_H_ */
