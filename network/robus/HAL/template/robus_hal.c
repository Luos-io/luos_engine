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

// MCU dependencies this HAL is for family XXX you can find

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 20
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
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{

    /*************************************************************************
     *
     * This function initialize robus communication base on USART. you need
     * COM_TX and COM_RX pin initialization for usart in RobusHAL_GPIOInit
     * function
     *
     * USART :
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
     *
     ************************************************************************/

    // Timeout Initialization is done in this function
    Timer_Prescaler = (MCUFREQ / Baudrate) / TIMERDIV;
    RobusHAL_TimeoutInit();

#ifndef USE_TX_IT
    /*************************************************************************
     *
     * You may want to use DMA for to send data use #ifndef USE_TX_IT
     * initialize in this DMA transert
     * direction : memory to peripherial
     * mode : not circular mode
     * increment : memory
     *
     * Don't forget to :
     * enable clock for DMA
     * add DMA peripherial adress
     *
     ************************************************************************/
#endif
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
    /*************************************************************************
     *
     * This function allow Luos_engine to enable or disable usart Tx line
     * this is necessary because Rx and Tx line are common
     *
     ************************************************************************/
    if (Enable == true)
    {
        /********************************************************************
         *
         * Put COM_TX pin in push pull mode
         *
         *******************************************************************/
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            /********************************************************************
             *
             * Put TX_EN pin to high
             *
             *******************************************************************/
        }
    }
    else
    {
        /********************************************************************
         *
         * Put COM_TX pin in open drain mode
         *
         *******************************************************************/
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            /********************************************************************
             *
             * Put TX_EN pin to Low
             *
             *******************************************************************/
        }
        /********************************************************************
         *
         * disable Transmission complete interrupt
         *
         * If you not use DMA :
         * put data_size_to_transmit to 0
         * disable Transmission buffer empty interrupt
         *
         * If you not use DMA :
         * disable DMA
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
     *
     * This function allow Luos_engine to enable or disable Rx
     * Avoiding to receiving what luos_engine send
     *
     ************************************************************************/
    if (Enable == true)
    {
        /*************************************************************************
         *
         * enable reception buffer not empty interrupt
         * don't forget to clear the data register
         *
         ************************************************************************/
    }
    else
    {
        /*************************************************************************
         *
         * disable reception buffer not empty interrupt
         *
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
     *
     * This is the callback function when an IRQ usart transmission and/or
     * reception is call.
     * this is a Macro that can be replace in HAL config with the IRQ handler
     * define in the sdk or if you can define your own handler call this function
     * RobusHAL_ComIrqHandler
     *
     * This function process communication you have to perform some action
     * in a specifique order :
     *
     * 1. Reset timout with default value each time this function is call
     *
     * 2. check if IRQ and flag Rx active
     * Read on byte and pass it to ctx.rx.callback
     * this call back is a state machine to check and decode Luos frame
     *
     * 3. Check framing error
     *
     * 4. check if IRQ and flag Tx complete active
     * with or without DMA
     * if TX complete you must disable Tx and enable rx
     * without DMA
     * check if IRQ and flag Tx empty active
     * transmit next data
     *
     * don't forget to clean IRQ flag
     *
     ************************************************************************/

    /*1. Reset timeout to it's default value*/
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);

    /*2. reception management*/
    if ("reception buffer not empty interrupt is true and enable")
    {
        // get data from register
        ctx.rx.callback(&data); // send reception byte to state machine
        if (data_size_to_transmit == 0)
        {
            // clear error IT
            return;
        }
    }
    /* 3. Framming error IT*/
    else if ("reception framming error is true and enable")
    {
        ctx.rx.status.rx_framing_error = true;
    }

    /*4. Transmission management*/
    if ("Transmission complete interrupt is true and enable")
    {
        RobusHAL_SetRxState(true);
        RobusHAL_SetTxState(false);
        // clear IRQ and disable Transmission complete interrupt
    }
#ifdef USE_TX_IT
    else if ("Transmission empty buffer interrupt is true and enable")
    {
        data_size_to_transmit--;
        // transmit data function with the tx_data pointer

        if (data_size_to_transmit == 0) // Transmission complete, stop loading data and watch for the end of transmission
        {
            // clear IRQ and disable Transmission empty buffer interrupt
            // Enable Transmission complete interrupt
        }
    }
#endif * /

    // clear flag
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
     *
     * Luos engine use this function to send data from msg buffer there is
     * two way to send data depending on your configuration DMA or IT
     * But when 1 octet is send robus never use DMA
     *
     * This function send data over network :
     *
     * 1. Enable Tx Line
     *
     * 2. check size > 1 that mean it's a luos frame to send
     * with DMA
     * setup DMA communication pointer and size to send
     * without DMA
     * Put data in transmission buffer and enable IRQ
     *
     * 3. check size <= 1 that mean it's a ack to send
     * wait 5us befor send and ack for different MCU frequency compatibility
     * Put data in transmission buffer and enable IRQ
     *
     * 4. reset timout to default value
     *
     ************************************************************************/

    // 1. Enable Tx Line
    // be sure transmission buffer is empty
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
        // wait before send ack 5us
        // this is a patch du to difference MCU frequency
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
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
    /*************************************************************************
     *
     * This function is use in the case the MCU does not give the possibility
     * to know if there is a reception pending. leave empty if is possible
     *
     * this pin TX_LOCK_DETECT must be connect phisicaly to RX pin to detect
     * en falling edge of a start bit in usart communication.
     *
     ************************************************************************/
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        // clear tx detect IT
        if (Enable == true)
        {
            /*************************************************************************
             *
             * Enable flag transmission lock detect interrupt pin
             *
             ************************************************************************/
        }
        else
        {
            /*************************************************************************
             *
             * Disable flag transmission lock interrupt pin
             *
             ************************************************************************/
        }
    }
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t RobusHAL_GetTxLockState(void)
{
    uint8_t result = false;

    /*************************************************************************
     *
     * This function check if a reception is pending. this function is
     * mendatory to know if luos engine can send data
     *
     * there is two way to know if a reception is pending : the sdk give you
     * the status of reception if not use Tx lock pin
     ************************************************************************/

#ifdef("reception pending")
    // check busy flag
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
    result = true;
#else
    if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
    {
        // if pin low
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
     *
     * This function initialize robus communication timeout. in reception or
     * transmission robus will considere that a frame is finish if a start
     * frame has been send or receive and no interrupt is receive at a time
     * equivalent to twice the time of sending one byte
     *
     * Timeout :
     * Auto reload : DEFAULT_TIMEOUT
     * repetition : one
     *
     * Don't forget to :
     * Reset timer counter
     * enable clock for timer
     * Enable timer
     * Enable timer IRQ
     *
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
     *
     * This function reset robus communication timeout. to stop the timeout
     * put 0 to nbrbit. a callback function is call when the timing is match
     *
     ************************************************************************/

    // disable counter
    // clear pending interrup and flag
    // Reset counter
    if (nbrbit != 0)
    {
        // reload value
        // enable counter
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
     *
     * This is the callback function when a timing IRQ is call.
     *
     * this is a Macro that can be replace in HAL config with the IRQ handler
     * define in the sdk or if you can define your own handler call this function
     * RobusHAL_TimoutIrqHandler
     *
     *
     ************************************************************************/
    if ("timer flag interrupt true and enable")
    {
        // clear flag
        // disable counter
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
     *
     * This function initialize robus communication pinout.
     * USART Pin :  COM_TX and COM_RX
     * RS485 Pin :  TX_EN and RX_EN
     * PTP Pin :    PTPX
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

    // configure PTP
    RobusHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++)
    {
        // Configure GPIO pins : PTP_Pin
        // IT falling
        // Pull down
        // Setup PTP lines
        RobusHAL_SetPTPDefaultState(i);
        // Activagte IT
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
     *
     * This function associate PTP pin to structure Port_t PTP[NBR_PORT];
     * to works, robus need at least 2 PTP per board to daisy chaine
     * the network
     *
     * NBR_PORT is a define that you can adapt to you project in
     * node_config.h files
     *
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
     *
     * This is the callback function when a Pinout IRQ is call. the pin number
     * of the interrupt edge detected must be pass to this function
     *
     * this is a Macro that can be replace in HAL config with the IRQ handler
     * define in the sdk or if you can define your own handler call this function
     * RobusHAL_PinoutIrqHandler
     *
     * This callback is mainly use for PTP edge detection in the different
     * state :
     * PTPDefaultState
     * PTPReverseState
     *
     * it can be use to detection start bit with TX_LOCK_DETECT_PIN in the
     * case that pin is needed and wire to COM_RX
     *
     ************************************************************************/
    ////Process for Tx Lock Detec
    if (GPIO_Pin == TX_LOCK_DETECT_PIN)
    {
        ctx.tx.lock = true;
        // clear flag
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
 * @brief Set PTP for Detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    /*************************************************************************
     *
     * the default state of the PTP line :
     * Output
     * Pull Down
     * IT mode Rising Edge
     *
     * don't forget to clear a pending Pin IRQ
     *
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
     *
     * the detection of a back answer of a PTP line :
     * Output
     * Pull Down
     * IT mode flling Edge
     *
     * don't forget to clear a pending Pin IRQ
     *
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
     *
     * Push the line to high level in case of detection ask on PTP line:
     * Output
     * Pull Down
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
     *
     * Read the line level on PTP line:
     * Input
     * Pull Down
     *
     ************************************************************************/
    return // value of the PTP Line
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
    /*************************************************************************
     *
     * This function initialize robus communication CRC.
     * if the flag USE_CRC_HW and the CRC need to be initiate
     *
     * CRC :
     * Init Value : use uint8_t *crc
     * Generator polynomial lenght : 7
     * Polynomial lenght : 16 bits
     * Inversion : none
     *
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
     *
     * This function calculate CRC when byte from luos frame are received
     * or send.
     *
     * You can use hardware CRC calculatiion if you MCU provide the capability
     * to have one or use a software CRC already compute in this function
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