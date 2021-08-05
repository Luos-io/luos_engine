/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family XXX
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>
#include "reception.h"
#include "context.h"

//MCU dependencies this HAL is for family XXX you can find

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 20
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t Timer_Prescaler = (MCUFREQ / DEFAULTBAUDRATE) / TIMERDIV; //(freq MCU/freq timer)/divider timer clock source

typedef struct
{
    uint16_t Pin;
    uint8_t *Port; //STUB
    uint8_t IRQ;
} Port_t;

Port_t PTP[NBR_PORT];

volatile uint16_t data_size_to_transmit = 0;
volatile uint8_t *tx_data               = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void LuosHAL_SystickInit(void);
static void LuosHAL_FlashInit(void);
static void LuosHAL_CRCInit(void);
static void LuosHAL_TimeoutInit(void);
static void LuosHAL_GPIOInit(void);
static void LuosHAL_FlashEraseLuosMemoryInfo(void);
static void LuosHAL_RegisterPTP(void);
static void __enable_irq();
static void __disable_irq();

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_Init(void)
{
    //Systick Initialization
    LuosHAL_SystickInit();

    //IO Initialization
    LuosHAL_GPIOInit();

    // Flash Initialization
    LuosHAL_FlashInit();

    // CRC Initialization
    LuosHAL_CRCInit();

    //Com Initialization
    LuosHAL_ComInit(DEFAULTBAUDRATE);
}
/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(uint8_t Enable)
{
    if (Enable == true)
    {
        __enable_irq();
    }
    else
    {
        __disable_irq();
    }
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return tick Counter
 ******************************************************************************/
static void LuosHAL_SystickInit(void)
{
    //Systick timer initialization
}
/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    return 0; //return  tick
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void LuosHAL_ComInit(uint32_t Baudrate)
{
    LUOS_COM_CLOCK_ENABLE();

    // Initialise USART1

    // Enable Reception interrupt

    //Enable NVIC IRQ

    //enable DMA
#ifndef USE_TX_IT
    //if DMA possible initialize DMA for a data transmission
#endif
    //Timeout Initialization
    Timer_Prescaler = (MCUFREQ / Baudrate) / TIMERDIV;
    LuosHAL_TimeoutInit();
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetTxState(uint8_t Enable)
{
    if (Enable == true)
    {
        //put Tx COM pin in push pull
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            //Tx enable
        }
    }
    else
    {
        //put Tx COM pin in Open drain
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            //Tx Disable
        }
#ifdef USE_TX_IT
        // Stop current transmit operation
        data_size_to_transmit = 0;
        //Disable IT tx empty
#else

        //stop DMA transmission DMA disable
#endif
        //disable tx complet IT
    }
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_SetRxState(uint8_t Enable)
{
    if (Enable == true)
    {
        //clear data register
        // Enable Rx com
        // Enable Rx IT
    }
    else
    {
        // disable Rx com
        // disable Rx IT
    }
}

/******************************************************************************
 * @brief Process data send or receive
 * @param None
 * @return None
 ******************************************************************************/
/*void LUOS_COM_IRQHANDLER()
{

}
#endif
//clear error flag
}*/
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComTransmit(uint8_t *data, uint16_t size)
{
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void LuosHAL_SetRxDetecPin(uint8_t Enable)
{
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        //clear tx detect IT
        if (Enable == true)
        {
            //clear flag
        }
        else
        {
            //set flag
        }
    }
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t LuosHAL_GetTxLockState(void)
{
    uint8_t result = false;

#ifdef USART_ISR_BUSY
    //check busy flag
    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
    result = true;
#else
    if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
    {
        //if pin low
        result = true;
        if (TX_LOCK_DETECT_IRQ == DISABLE)
        {
            if (result == true)
            {
                LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
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
static void LuosHAL_TimeoutInit(void)
{
    //initialize clock
    LUOS_TIMER_CLOCK_ENABLE();

    //timer init

    //NVIC IT
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ResetTimeout(uint16_t nbrbit)
{
    // clear NVIC IT pending
    // clear IT flag
    //reset counter Timer
    //relaod value counter
    //if nbrbit != 0
    //enable timer
}

/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
/*void LUOS_TIMER_IRQHANDLER()
{
    //if It timeout
    //clear IT
    if ((ctx.tx.lock == true) && (LuosHAL_GetTxLockState() == false))
    {
        Recep_Timeout();
    }
}*/
/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_GPIOInit(void)
{
    //Activate Clock for PIN choosen in luosHAL
    PORT_CLOCK_ENABLE();

    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        //Configure GPIO pins : RxEN_Pin
        //output
        //no pull
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        //Configure GPIO pins : TxEN_Pin
        //output
        //no pull
    }

    //Configure GPIO pin : TxPin
    //ALTERNATE function USART Tx
    //open drain
    //pull up

    //Configure GPIO pin : RxPin
    //ALTERNATE function USART Rx
    //open drain
    //pull up

    //configure PTP
    LuosHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        //IT falling
        // pull down
        // Setup PTP lines
        LuosHAL_SetPTPDefaultState(i);
        //activate NVIC IT for PTP
    }

    /*Configure GPIO pins : TX_LOCK_DETECT_Pin */
    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        //pull up
        //input
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            //it falling
            //NVIC enable
        }
    }
}
/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void LuosHAL_RegisterPTP(void)
{
#if (NBR_PORT >= 1)
    PTP[0].Pin  = PTPA_PIN;
    PTP[0].Port = PTPA_PORT;
    PTP[0].IRQ  = PTPA_IRQ;
#endif

#if (NBR_PORT >= 2)
    PTP[1].Pin  = PTPB_PIN;
    PTP[1].Port = PTPB_PORT;
    PTP[1].IRQ  = PTPB_IRQ;
#endif

#if (NBR_PORT >= 3)
    PTP[2].Pin  = PTPC_PIN;
    PTP[2].Port = PTPC_PORT;
    PTP[2].IRQ  = PTPC_IRQ;
#endif

#if (NBR_PORT >= 4)
    PTP[3].Pin  = PTPD_PIN;
    PTP[3].Port = PTPD_PORT;
    PTP[3].IRQ  = PTPD_IRQ;
#endif
}

/******************************************************************************
 * @brief callback for GPIO IT
 * @param GPIO IT line
 * @return None
 ******************************************************************************/
/*void PINOUT_IRQHANDLER(uint16_t GPIO_Pin)
{
    ////Process for Tx Lock Detec
    if (GPIO_Pin == TX_LOCK_DETECT_PIN)
    {
        ctx.tx.lock = true;
        //clear flag
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
}*/
/******************************************************************************
 * @brief Set PTP for Detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    //clear IT
    // Pull Down / IT mode / Rising Edge
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    //clear IT
    // Pull Down / IT mode / Falling Edge
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    // Clean edge/state detection and set the PTP pin as output
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t LuosHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_CRCInit(void)
{
    //CRC initialisation
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
#ifdef CRC_HW
    //CRC HW calculation
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
/******************************************************************************
 * @brief Flash Initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashInit(void)
{
}
/******************************************************************************
 * @brief Erase flash page where Luos keep permanente information
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashEraseLuosMemoryInfo(void)
{
    uint32_t page_error = 0;
    //routine to erase flash page
}
/******************************************************************************
 * @brief Write flash page where Luos keep permanente information
 * @param Address page / size to write / pointer to data to write
 * @return
 ******************************************************************************/
void LuosHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    // Before writing we have to erase the entire page
    // to do that we have to backup current falues by copying it into RAM
    uint8_t page_backup[PAGE_SIZE];
    memcpy(page_backup, (void *)ADDRESS_ALIASES_FLASH, PAGE_SIZE);

    // Now we can erase the page

    // Then add input data into backuped value on RAM
    uint32_t RAMaddr = (addr - ADDRESS_ALIASES_FLASH);
    memcpy(&page_backup[RAMaddr], data, size);

    // and copy it into flash

    //write data
}

/******************************************************************************
 * @brief read information from page where Luos keep permanente information
 * @param Address info / size to read / pointer callback data to read
 * @return
 ******************************************************************************/
void LuosHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    memcpy(data, (void *)(addr), size);
}

void __enable_irq()
{
}
void __disable_irq()
{
}

/******************************************************************************
 * @brief Set boot mode in shared flash memory
 * @param 
 * @return
 ******************************************************************************/
void LuosHAL_SetMode(uint8_t mode)
{
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param Address, node_id
 * @return
 ******************************************************************************/
void LuosHAL_SaveNodeID(uint16_t node_id)
{
}

/******************************************************************************
 * @brief software reboot the microprocessor
 * @param 
 * @return
 ******************************************************************************/
void LuosHAL_Reboot(void)
{
}

/*void LUOS_TIMER_CLOCK_ENABLE()
{
}*/