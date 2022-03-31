/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family XXX
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "robus_hal.h"
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
typedef struct
{
    uint16_t Pin;
    uint8_t *Port; // STUB
    uint8_t IRQ;
} Port_t;

Port_t PTP[NBR_PORT];

volatile uint8_t *tx_data = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_SystickInit(void);
static void RobusHAL_FlashInit(void);
static void RobusHAL_CRCInit(void);
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_FlashEraseLuosMemoryInfo(void);
static void RobusHAL_RegisterPTP(void);
static void __enable_irq();
static void __disable_irq();

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Init(void)
{
    // Systick Initialization
    RobusHAL_SystickInit();

    // IO Initialization
    RobusHAL_GPIOInit();

    // Flash Initialization
    RobusHAL_FlashInit();

    // CRC Initialization
    RobusHAL_CRCInit();

    // Com Initialization
    RobusHAL_ComInit(DEFAULTBAUDRATE);
}

/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetIrqState(uint8_t Enable)
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
static void RobusHAL_SystickInit(void)
{
    // Systick timer initialization
}

/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return tick Counter
 ******************************************************************************/
uint32_t RobusHAL_GetSystick(void)
{
    clock_t tick = clock();
    return tick; // return  tick
}

/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    // LUOS_COM_CLOCK_ENABLE();

    // Initialise USART1

    // Enable Reception interrupt

    // Enable NVIC IRQ

    // enable DMA
#ifndef USE_TX_IT
    // if DMA possible initialize DMA for a data transmission
#endif
    // Timeout Initialization
    return;
}

/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
    if (Enable == true)
    {
    }
    else
    {
    }
#ifdef USE_TX_IT
    // Stop current transmit operation
    // Disable IT tx empty
#else

    // stop DMA transmission DMA disable
#endif
    // disable tx complet IT
}

/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void RobusHAL_SetRxState(uint8_t Enable)
{
    if (Enable == true)
    {
        // clear data register
        //  Enable Rx com
        //  Enable Rx IT
    }
    else
    {
        // disable Rx com
        // disable Rx IT
    }
}

/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
}

/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        // clear tx detect IT
        if (Enable == true)
        {
            // clear flag
        }
        else
        {
            // set flag
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

#ifdef USART_ISR_BUSY
    // check busy flag
#else
    if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
    {
    }
#endif
}

/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
    // initialize clock

    // timer init

    // NVIC IT
}

/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    // clear NVIC IT pending
    // clear IT flag
    // reset counter Timer
    // relaod value counter
    // if nbrbit != 0
    // enable timer
}

/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
    // Activate Clock for PIN choosen in luosHAL

    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        // Configure GPIO pins : RxEN_Pin
        // output
        // no pull
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        // Configure GPIO pins : TxEN_Pin
        // output
        // no pull
    }

    // Configure GPIO pin : TxPin
    // ALTERNATE function USART Tx
    // open drain
    // pull up

    // Configure GPIO pin : RxPin
    // ALTERNATE function USART Rx
    // open drain
    // pull up

    // configure PTP
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        // IT falling
        //  pull down
        //  Setup PTP lines
        // activate NVIC IT for PTP
    }

    /*Configure GPIO pins : TX_LOCK_DETECT_Pin */
    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        // pull up
        // input
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            // it falling
            // NVIC enable
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
                //PortMng_PtpHandler(i);
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
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    // clear IT
    //  Pull Down / IT mode / Rising Edge
}

/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    // clear IT
    //  Pull Down / IT mode / Falling Edge
}

/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    // Clean edge/state detection and set the PTP pin as output
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
    // CRC initialisation
}

/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
    for (uint8_t i = 0; i < 1; ++i)
    {
        uint16_t dbyte = data[i];
        *(uint16_t *)crc ^= dbyte << 8;
        for (uint8_t j = 0; j < 8; ++j)
        {
            uint16_t mix     = *(uint16_t *)crc & 0x8000;
            *(uint16_t *)crc = (*(uint16_t *)crc << 1);
            if (mix)
                *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
        }
    }
}

/******************************************************************************
 * @brief Flash Initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_FlashInit(void)
{
    for (uint16_t i = 0; i < FLASH_PAGE_NUMBER; i++)
    {
        for (uint16_t j = 0; j < FLASH_PAGE_SIZE; j++)
        {
            stub_flash_x86[i][j] = 0;
        }
    }
}

/******************************************************************************
 * @brief Erase flash page where Luos keep permanente information
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_FlashEraseLuosMemoryInfo(void)
{
    uint32_t page_error = 0;
    // routine to erase flash page
}

/******************************************************************************
 * @brief Write flash page where Luos keep permanente information
 * @param Address page / size to write / pointer to data to write
 * @return
 ******************************************************************************/
void RobusHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    // Before writing we have to erase the entire page
    // to do that we have to backup current falues by copying it into RAM
    uint8_t page_backup[PAGE_SIZE];
    // memcpy(page_backup, (void *)ADDRESS_ALIASES_FLASH, PAGE_SIZE);

    // Now we can erase the page

    // Then add input data into backuped value on RAM
    uint32_t RAMaddr = (addr - ADDRESS_ALIASES_FLASH);
    // memcpy(&page_backup[RAMaddr], data, size);

    // and copy it into flash

    // write data
}

/******************************************************************************
 * @brief read information from page where Luos keep permanente information
 * @param Address info / size to read / pointer callback data to read
 * @return
 ******************************************************************************/
void RobusHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    memset(data, 0xFF, size);
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
void RobusHAL_SetMode(uint8_t mode)
{
}

/******************************************************************************
 * @brief Save node ID in shared flash memory
 * @param Address, node_id
 * @return
 ******************************************************************************/
void RobusHAL_SaveNodeID(uint16_t node_id)
{
}

/******************************************************************************
 * @brief software reboot the microprocessor
 * @param
 * @return
 ******************************************************************************/
void RobusHAL_Reboot(void)
{
}

/******************************************************************************
 * @brief Luos GetTimestamp
 * @param None
 * @return uint64_t
 ******************************************************************************/
uint64_t RobusHAL_GetTimestamp(void)
{
    return (LuosHAL_GetSystick() * 1000);
}

/******************************************************************************
 * @brief Luos start Timestamp
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_StartTimestamp(void)
{
}

/******************************************************************************
 * @brief Luos stop Timestamp
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_StopTimestamp(void)
{
}

/*void LUOS_TIMER_CLOCK_ENABLE()
{
}*/