/******************************************************************************
 * @file phy.h
 * @brief interface with the phy
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOS_PHY_H_
#define _LUOS_PHY_H_

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct luos_luos_phy_t
 * @brief the data structure allowing the phy to interract with LuosIO
 ******************************************************************************/
typedef struct luos_luos_phy_t
{
    volatile int64_t rx_timestamp; // Timestamp of the last received message. We will use it to compute the date based on received latency.
    uint8_t *rx_buffer_base;       // Pointer to the base of the buffer where we will store the begining of received data.
    volatile uint8_t *rx_data;     // Pointer to write received bytes.
    volatile uint16_t rx_size;     // Size of data to receive data.
    struct __attribute__((__packed__))
    {
        volatile uint8_t rx_alloc_job : 1; // If true, loop will need to filter already received data, if filter is ok, set rx_keep, alloc space, move *rx_data to point at the good place on allocator, put the number of bytes remaining on rx_size, and copy already received data in the allocated space.
        volatile uint8_t rx_keep : 1;      // True if we want to keep the received data.
    };
} luos_phy_t;

void Phy_Init(void);
void Phy_Loop(void);
luos_phy_t *Phy_Create(void);

void Phy_Send(luos_phy_t *phy, uint8_t *data, uint16_t size);

#endif /* _LUOS_PHY_H_ */
