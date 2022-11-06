/******************************************************************************
 * @file robus_HAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family XXX
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "robus_hal.h"
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>
#include "reception.h"
#include "context.h"

/*************************************************************************
 * This file is a template and documentation for a Robus network HAL layer.
 * Feel free to duplicate it and customize it to your needs.
 *************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 20
/*******************************************************************************
 * Variables
 ******************************************************************************/
//(freq MCU/freq timer)/divider timer clock source
uint32_t Timer_Prescaler;

typedef struct
{
    uint16_t Pin;
} Port_t;

Port_t PTP[NBR_PORT];

volatile uint16_t data_size_to_transmit = 0;
volatile uint8_t *tx_data               = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_CRCInit(void);
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_RegisterPTP(void);

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Init(void)
{
    // IO Initialization
    RobusHAL_GPIOInit();

    // CRC Initialization
    RobusHAL_CRCInit();

    // Com Initialization
    RobusHAL_ComInit(DEFAULTBAUDRATE);
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{

}
/******************************************************************************
 * @brief Tx enable/disable
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void RobusHAL_SetRxState(uint8_t Enable)
{
}
/******************************************************************************
 * @brief Process data send or receive
 * @param None
 * @return None
 ******************************************************************************/
/******************************************************************************
 * @brief Process data transmit
 * @param data pointer to data to send
 * @param size size to send
 * @return None
 ******************************************************************************/
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
}
/******************************************************************************
 * @brief set rx accuring detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
}
/******************************************************************************
 * @brief Get the TX Lock status
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t RobusHAL_GetTxLockState(void)
{
    return 0;
}
/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
}
/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void RobusHAL_RegisterPTP(void)
{
}
/******************************************************************************
 * @brief Set PTP for branch Detection
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    /*************************************************************************
     * Set the default state on the PTP line : (allowing to catch a ping on a branch)
     *      - Input
     *      - Pull Down
     *      - IT mode Rising Edge
     *
     * Don't forget to clear a pending Pin IRQ
     ************************************************************************/
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    return 0;
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
}