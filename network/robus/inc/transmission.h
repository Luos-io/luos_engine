/******************************************************************************
 * @file sys_msg
 * @brief protocol system message management.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _TRANSMISSION_H_
#define _TRANSMISSION_H_

#include <stdint.h>
#include "luos_phy.h"
#include "robus_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    TX_DISABLE, /*!< transmission with ack */
    TX_OK,      /*!< transmission ok */
    TX_NOK      /*!< transmission fail */

} transmitStatus_t;

typedef struct
{
    volatile uint8_t lock;            // Transmit lock state
    const uint8_t *data;              // data to compare for collision detection
    volatile transmitStatus_t status; // status of the transmission
    volatile uint8_t collision;       // true is a collision occure during this transmission.
} TxCom_t;

typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            time_luos_t timestamp;
            uint16_t timestamped_crc;
        };                                               // This form is used if there is a timestamp in the message.
        uint16_t crc;                                    // This form is used if there is no timestamp in the message.
        uint8_t unmaped[sizeof(time_luos_t) + CRC_SIZE]; // This form is used to access the last part to transmit as an array of bytes.
    };
    uint16_t size;
} robus_encaps_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Transmit_Init(void);
uint16_t ll_crc_compute(const uint8_t *data, uint16_t size, uint16_t crc_seed);
void Transmit_SendAck(void);
void Transmit_Process(void);
void Transmit_End(void);

#endif /* _TRANSMISSION_H_ */
