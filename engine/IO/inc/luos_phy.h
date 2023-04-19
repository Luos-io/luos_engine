/******************************************************************************
 * @file phy.h
 * @brief This file have to be imported by phy layers to interract with Luos
 * @author Nicolas Rabault
 * @version 0.0.0
 ******************************************************************************/
#ifndef _LUOS_PHY_H_
#define _LUOS_PHY_H_

#include <stdint.h>
#include "struct_luos.h"
#include "struct_io.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * @struct luos_luos_phy_t
 * @brief the data structure allowing the phy to interract with LuosIO
 ******************************************************************************/
typedef struct
{
    // This is allowing to access the data as data or message
    union
    {
        uint8_t *data_pt; // Start pointer of the data on msg_buffer.
        msg_t *msg_pt;    // Pointer to the message.
    };
    uint16_t size;  // size of the data.
    void *phy_data; // Phy developpers can use this pointer to store any data they need.
} phy_tx_job_t;

typedef struct luos_luos_phy_t
{
    // *************** RX information ***************
    volatile int64_t rx_timestamp; // Timestamp of the last received message. We will use it to compute the date based on received latency.
    uint8_t *rx_buffer_base;       // Pointer to the base of the buffer where we will store the begining of received data.
    union
    {
        volatile uint8_t *rx_data; // Pointer to write received bytes.
        volatile msg_t *rx_msg;    // Pointer to the message where we will write received bytes.
    };
    volatile uint16_t received_data; // Number of bytes already received.
    volatile uint16_t rx_size;       // Size of data to receive.
    struct __attribute__((__packed__))
    {
        volatile uint8_t rx_alloc_job : 1; // If true, Luosio_Loop funciton will need to filter already received data, if filter is ok, set rx_keep, alloc needed space, move *rx_data to point at the good place on allocator, put the number of bytes remaining on rx_size, and copy already received data in the allocated space.
        volatile uint8_t rx_keep : 1;      // True if we want to keep the received data.
    };

    // RX Private data
    luos_localhost_t rx_phy_filter; // The phy concerned by this message.

    // *************** TX informations ***************
    phy_tx_job_t tx_job[MAX_MSG_NB]; // List of phy tx jobs to send.
    uint8_t tx_job_nb;               // Number of tx jobs to send.
} luos_phy_t;

void Phy_Init(void);
void Phy_Loop(void);
luos_phy_t *Phy_Create(void);

void Phy_Send(luos_phy_t *phy_ptr, uint8_t *data, uint16_t size);
void Phy_Computeheader(luos_phy_t *phy_ptr);
void Phy_ValidMsg(luos_phy_t *phy_ptr);

#endif /* _LUOS_PHY_H_ */
