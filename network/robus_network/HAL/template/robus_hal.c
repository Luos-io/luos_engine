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
#define DEFAULT_TIMEOUT 30
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4
/*******************************************************************************
 * Variables
 ******************************************************************************/
//(freq MCU/freq timer)/divider timer clock source
uint32_t Timer_Prescaler = (MCUFREQ / DEFAULTBAUDRATE) / TIMERDIV;

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
 * @brief Luos HAL general loop
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Loop(void)
{
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{

    /*************************************************************************
     * This function is called once at the beginning of the program.
     * It is used to initialize the hardware and the variables.
     * You can add your own initialisation code here.
     *************************************************************************/

    /*************************************************************************
     * This function initialize Robus communication.
     * Robus is based on an half duplex USART hardware.
     * You need to initialize the USART hardware and the interrupt.
     * COM_TX and COM_RX pin initialization for usart is done in RobusHAL_GPIOInit().
     *
     * USART configuration parameters :
     * datawith : 8 bits
     * stopbit : 1 bits
     * parity  : none
     * Bauderate : DEFAULTBAUDRATE
     *
     * Don't forget to :
     * enable clock for USART
     * Enable USART
     * Enable USART IRQ
     * Enable Rx buffer empty IRQ
     ************************************************************************/

    // Timeout Initialization is done in this function
    Timer_Prescaler = (MCUFREQ / Baudrate) / TIMERDIV;
    RobusHAL_TimeoutInit();

#ifndef USE_TX_IT
    /*************************************************************************
     * Robus allow you to use TX interrupt to manage the transmission.
     * You may want to use it to enable DMA transfert to TX saving a lot of CPU time.
     * To do that initialize this DMA transert:
     *  - Direction : memory to peripherial
     *  - Mode : not circular mode
     *  - Increment : memory
     *
     * Don't forget to :
     *  - Enable clock for DMA
     *  - Add DMA peripherial adress
     ************************************************************************/
#endif
}
/******************************************************************************
 * @brief Tx enable/disable
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
    /*************************************************************************
     * This function allow Luos_engine to enable or disable usart Tx line
     * This is necessary because Rx and Tx line are common due to the half duplex
     ************************************************************************/
    if (Enable == true)
    {
        /********************************************************************
         * Put COM_TX pin in push pull mode
         *******************************************************************/
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            /********************************************************************
             * Put TX_EN pin to high enabling the Tx line on the Driver
             *******************************************************************/
        }
    }
    else
    {
        /********************************************************************
         * Put COM_TX pin in open drain mode
         *******************************************************************/
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            /********************************************************************
             * Put TX_EN pin to Low disabling the Tx line on the Driver
             *******************************************************************/
        }
        /********************************************************************
         * Disable Transmission complete interrupt
         *
         * If you don't use DMA :
         *  - Put data_size_to_transmit to 0
         *  - Disable Transmission buffer empty interrupt
         *
         * If you use DMA :
         *  - Disable DMA
         *
         *******************************************************************/
#ifdef USE_TX_IT
        // Stop current transmit operation
        data_size_to_transmit = 0;
        // Disable Transmission empty buffer interrupt
#else
        // Disable DMA
#endif
        // Disable Transmission complete interrupt
    }
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void RobusHAL_SetRxState(uint8_t Enable)
{
    /*************************************************************************
     * This function allow Luos_engine to enable or disable Rx
     * Avoiding to receiving what luos_engine send
     ************************************************************************/
    if (Enable == true)
    {
        /*************************************************************************
         * Enable reception buffer not empty interrupt
         * Don't forget to clear the data register
         ************************************************************************/
    }
    else
    {
        /*************************************************************************
         * Disable reception buffer not empty interrupt
         ************************************************************************/
    }
}
/******************************************************************************
 * @brief Process data send or receive
 * @param None
 * @return None
 ******************************************************************************/
void LUOS_COM_IRQHANDLER()
{

    /*************************************************************************
     * This is the callback function called when an transmission and/or
     * reception IRQ on UART is raised.
     *
     * This function name is a Macro containing the IRQ handler name.
     * This macro can have a default value in robus_hal_config.h file then users will be able to modify it on their project node_config.h file.
     *
     * This function process communication and perform some action
     * in a specifique order :
     *
     *      1. Reset the timeout with the default value each time this function is call
     *
     *      2. Check if IRQ flag Rx is active (this means that this IRAQ have been raised because we receive a byte)
     *          Read the received byte and pass it to Recep_data(byte).
     *          This function is the Robus reception state machine allowing to check and decode Luos frames.
     *
     *      3. Check framing error
     *
     *      4. Check if IRQ flag Tx complete active (this means that this IRQ have been raised because we have finished to transmit oen or a collection of bytes)
     *          Disable Tx and enable rx
     *          without DMA :
     *              Check if IRQ flag Tx empty is active
     *                  Transmit next data
     *
     * Don't forget to clean IRQ flags
     ************************************************************************/

    /*1. Reset timeout to it's default value*/
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);

    /*2. Reception management*/
    if ("Reception buffer not empty interrupt is true and enable")
    {
        // Get data from register
        Recep_data(&data); // Send received byte to the Robus state machine
        if (data_size_to_transmit == 0)
        {
            // Clear error IT
            return;
        }
    }
    /* 3. Framming error IT*/
    else if ("Reception framming error is true and enable")
    {
        ctx.rx.status.rx_framing_error = true;
    }

    /*4. Transmission management*/
    if ("Transmission complete interrupt is true and enable")
    {
        data_size_to_transmit = 0;
        RobusHAL_SetRxState(true);
        RobusHAL_SetTxState(false);
        // Clear IRQ disable Transmission complete flag
    }
#ifdef USE_TX_IT
    else if ("Transmission empty buffer interrupt is true and enable")
    {
        data_size_to_transmit--;
        // Transmit data function with the tx_data pointer

        if (data_size_to_transmit == 0) // Transmission complete, stop loading data and wait for the end of transmission
        {
            // Clear IRQ and disable Transmission empty buffer interrupt
            // Enable Transmission complete interrupt
        }
    }
#endif * /

    // Clear flags
}
/******************************************************************************
 * @brief Process data transmit
 * @param data pointer to data to send
 * @param size size to send
 * @return None
 ******************************************************************************/
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{

    /*************************************************************************
     * Luos engine use this function to send data from msg buffer.
     * There is two way to send data depending on your configuration DMA or IT.
     *
     * When only 1 byte is send HAL never use DMA. Robus do that to acknoledge the reception of the a message.
     *
     * This function send data over network :
     *
     *  1. Enable Tx Line
     *
     *  2. if size > 1
     *      - with DMA, setup communication pointer and size to send
     *      - without DMA, put data directly in in the transmission buffer and enable IRQ
     *
     *  3. if size <= 1 that mean that this mesasge is an acknole to send
     *      - Wait 5us before sending the ack. This allow slower MCU to be ready to receive our acknoledge.
     *      - Put the data in transmission buffer and enable IRQ
     *
     *  4. Reset timout to default value
     ************************************************************************/

    // 1. Enable Tx Line
    // Be sure transmission buffer is empty
    RobusHAL_SetTxState(true);

    // 2. A luos frame to send
    data_size_to_transmit = size - 1; // Reduce size by one because we send one directly
    if (size > 1)
    {
        tx_data = data; // Copy the data pointer globally alowing to keep it and run the transmission
#ifdef USE_TX_IT
        // Put data in transmission buffer and enable IRQ
        // Disable Transmission complete interrupt
#else
        data_size_to_transmit = 0; // Reset this value avoiding to check IT TC during collision
        // Set up DMA transfert
        // Enable Transmission complete interrupt
#endif
    }
    // 3. A ack to send
    else
    {
        data_size_to_transmit = 0;
        // Wait 5us before sending the ack
        while ("counter < TIMEOUT_ACK")
            ;
        RobusHAL_SetTxState(true); // Enable TX
        // Transmit the only byte we have
        // Enable Transmission complete interrupt because we only have one.
    }

    // 4. reset timout to default value
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
}
/******************************************************************************
 * @brief set rx accuring detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
    /*************************************************************************
     * This function is used if your MCU does not give the possibility
     * to know if there is a reception pending. Leave it empty if is possible.
     *
     * This pin TX_LOCK_DETECT must be phisicaly connected to RX pin.
     * We will use it to detect the falling edge of a start bit to know itf someone is using the network.
     ************************************************************************/
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        // Clear tx detect IT
        if (Enable == true)
        {
            /*************************************************************************
             * Enable flag transmission lock detect interrupt pin
             ************************************************************************/
        }
        else
        {
            /*************************************************************************
             * Disable flag transmission lock interrupt pin
             ************************************************************************/
        }
    }
}
/******************************************************************************
 * @brief Get the TX Lock status
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t RobusHAL_GetTxLockState(void)
{
    uint8_t result = false;

    /*************************************************************************
     * This function check if a reception is pending. this function is
     * mendatory to know if luos engine can send data.
     *
     * There is two way to know if a reception is pending :
     *      - the MCU gives you the status of reception
     *      - You use the RobusHAL_SetRxDetecPin function
     ************************************************************************/

#ifdef("reception pending")
    // Check the RX busy flag
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
    result = true;
#else
    if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
    {
        // If pin low
        result = true;
        if (TX_LOCK_DETECT_IRQ == DISABLE)
        {
            if (result == true)
            {
                RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
            }
        }
    }
#endif
    return result;
}
/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
    /*************************************************************************
     * This function initialize the Robus communication timeout.
     * Every Robus message need to have a timeout at the end allowing another message to be sent.
     * In Robus timeout are kind of messages preamble.
     * Each interrut will reset this timout.
     * This timeout will raise after (TIMEOUT_VAL * "byte time")
     *
     * Timeout :
     *      - Auto reload : DEFAULT_TIMEOUT
     *      - Repetition : one
     *
     * Don't forget to :
     *      - Reset timer counter
     *      - Enable clock for timer
     *      - Enable timer
     *      - Enable timer IRQ
     ************************************************************************/
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    /*************************************************************************
     * This function reset the robus timeout.
     * To stop the timeout put 0 to nbrbit.
     * A callback function will be called when the timing matchs.
     ************************************************************************/

    // Disable counter
    // Clear pending interrup and flag
    // Reset counter
    if (nbrbit != 0)
    {
        // Reload value
        // Enable counter
    }
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void LUOS_TIMER_IRQHANDLER()
{
    /*************************************************************************
     * This is the callback function when a timout IRQ is raised.
     *
     * This function name is a Macro containing the IRQ handler name.
     * This macro can have a default value in robus_hal_config.h file then users will be able to modify it on their project node_config.h file.
     ************************************************************************/
    if ("Timer flag interrupt true and enable")
    {
        // Clear flag
        // Disable counter
        if ((ctx.tx.lock == true) && (RobusHAL_GetTxLockState() == false))
        {
            // Enable RX detection pin if needed
            RobusHAL_SetTxState(false);
            RobusHAL_SetRxState(true);
            Recep_Timeout();
        }
    }
}
/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
    /*************************************************************************
     * This function initialize robus communication pins.
     *      USART Pin        :  COM_TX and COM_RX
     *      RS485 driver Pin :  TX_EN and RX_EN
     *      PTP Pin          :  PTPX
     *
     ************************************************************************/

    // Activate Clock for PIN choosen
    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        // Configure GPIO pins : RxEN_Pin
        // Output
        // No pull
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        // Configure GPIO pins : TxEN_Pin
        // Output
        // No pull
    }

    // Configure GPIO pin : COM_TX
    // ALTERNATE function USART Tx
    // Open drain
    // Pull up

    // Configure GPIO pin : COM_RX
    // ALTERNATE function USART Rx
    // Open drain
    // Pull up

    // Configure PTP pins
    RobusHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++)
    {
        // Configure GPIO pins : PTP_Pin
        // IT falling
        // Pull down
        // Setup PTP lines
        RobusHAL_SetPTPDefaultState(i);
        // Activate IRQ for PTP line
    }

    // Configure GPIO pins : TX_LOCK_DETECT_Pin
    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        // Pull up
        // Input
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            // IRQ falling edge
            // Enable IRQ
        }
    }
}
/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void RobusHAL_RegisterPTP(void)
{
    /*************************************************************************
     * This function associate PTP pin to the Port_t PTP[NBR_PORT] table.
     *
     * To works, robus need at least 2 PTP lines per board to daisy chain the nodes on the network during detection.
     *
     * NBR_PORT is a define that you can adapt to you project in node_config.h files
     ************************************************************************/
#if (NBR_PORT >= 1)
    PTP[0].Pin = PTPA_PIN;
#endif

#if (NBR_PORT >= 2)
    PTP[1].Pin = PTPB_PIN;
#endif

#if (NBR_PORT >= 3)
    PTP[2].Pin = PTPC_PIN;
#endif

#if (NBR_PORT >= 4)
    PTP[3].Pin = PTPD_PIN;
#endif
}
/******************************************************************************
 * @brief callback for GPIO IT
 * @param GPIO IT line
 * @return None
 ******************************************************************************/
void PINOUT_IRQHANDLER(uint16_t GPIO_Pin)
{
    /*************************************************************************
     * This is the callback function called when a external interrupt IRQ is raised.
     * The pin number of the interrupt edge detected must be pass to this function
     *
     * This function name is a Macro containing the IRQ handler name.
     * This macro can have a default value in robus_hal_config.h file then users will be able to modify it on their project node_config.h file.
     *
     * This callback is mainly uses for PTP edge detection in the different state :
     *      - PTPDefaultState
     *      - PTPReverseState
     *
     * This can also be used to detect a start bit with TX_LOCK_DETECT_PIN.
     ************************************************************************/
    // Process for Tx Lock Detec
    if (GPIO_Pin == TX_LOCK_DETECT_PIN)
    {
        ctx.tx.lock = true;
        // Clear flag
    }
    else
    {
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            if (GPIO_Pin == PTP[i].Pin)
            {
                PortMng_PtpHandler(i);
                break;
            }
        }
    }
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
    /*************************************************************************
     * Set the reverse state on the PTP line : (allowing to get the ping response)
     *      - Input
     *      - Pull Down
     *      - IT mode flling Edge
     *
     * Don't forget to clear a pending Pin IRQ
     ************************************************************************/
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    /*************************************************************************
     * Set the push state on the PTP line : (allowing to ping the branch)
     *      - Output
     *      - Level High
     *
     ************************************************************************/
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    /*************************************************************************
     * Read the line level on PTP pin:
     ************************************************************************/
    return // Value of the PTP Line
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
    /*************************************************************************
     * This function initialize the hardware CRC unit.
     * This function is used only if USE_CRC_HW is defined in robus_hal_config.h or node_config.h file.
     *
     * CRC :
     *      - Init Value : use uint8_t *crc
     *      - Generator polynomial lenght : 7
     *      - Polynomial lenght : 16 bits
     *      - Inversion : none
     ************************************************************************/
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
    /*************************************************************************
     * This function compute message CRC byte by byte.
     *
     * You can use hardware CRC calculatiion if your MCU provides it.
     * If it's not, you will ahve to use the software CRC code already writen in this function.
     *
     ************************************************************************/
#if (USE_CRC_HW == 1)
    // CRC init value = uint8_t *crc
    // CRC HW calculation
#else
    for (uint8_t i = 0; i < 1; ++i)
    {
        uint16_t dbyte = data[i];
        *(uint16_t *)crc ^= dbyte << 8;
        for (uint8_t j = 0; j < 8; ++j)
        {
            uint16_t mix = *(uint16_t *)crc & 0x8000;
            *(uint16_t *)crc = (*(uint16_t *)crc << 1);
            if (mix)
                *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
        }
    }
#endif
}