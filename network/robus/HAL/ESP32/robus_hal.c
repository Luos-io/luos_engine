/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family ESP32
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "robus_hal.h"
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>
#include "reception.h"
#include "context.h"

// MCU dependencies this HAL is for family ESP32 you can find
// the HAL ESP-IDF on Expressiv web site
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "driver/timer.h"
#include "driver/uart.h"

#include "hal/gpio_hal.h"
#include "hal/timer_hal.h"
#include "hal/uart_hal.h"

#include "soc/uart_periph.h"
#include "esp_rom_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 30
#define TIMEOUT_ACK     DEFAULT_TIMEOUT / 4

#define RX_BUFFER_SIZE 140
#define TX_BUFFER_SIZE 140
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t Timer_Prescaler = (uint32_t)(MCUFREQ / DEFAULTBAUDRATE); //(freq MCU/freq timer)/divider timer clock source

typedef struct
{
    uint8_t Pin;
} Port_t;

Port_t PTP[NBR_PORT];

uint16_t data_size_to_transmit = 0;
uint8_t *tx_data               = 0;
uint32_t Rsize                 = 0;

volatile uint8_t RxEn         = false;
volatile uint8_t TxEn         = true;
volatile uint32_t enable_mask = 0;

/*******************************************************************************
 * esp
 ******************************************************************************/
gpio_config_t PinConfig;

static uart_isr_handle_t handle_console;

uart_hal_context_t uart_hal_context = {
    .dev = UART_LL_GET_HW(LUOS_COM)};

gpio_hal_context_t gpio_hal_context = {
    .dev = GPIO_HAL_GET_HW(GPIO_PORT_0)};

timer_hal_context_t timeout_hal_context = {
    .dev = TIMER_LL_GET_HW(LUOS_TIMER_GROUP),
    .idx = LUOS_TIMER,
};

/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_CRCInit(void);
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_RegisterPTP(void);
void RobusHAL_PinoutIrqHandler(void *arg);
timer_isr_t RobusHAL_TimeoutIrqHandler(void *arg);
void RobusHAL_ComIrqHandler(void *arg);
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
 * @brief Luos HAL general disable IRQ
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_SetIrqState(uint8_t Enable)
{
    static volatile uint8_t irq_mutex = true;

    if ((Enable == true) && (irq_mutex != true))
    {
        irq_mutex   = true;
        enable_mask = 0;
        if (TxEn == true)
        {
            enable_mask |= UART_INTR_TX_DONE;
        }
        if (RxEn == true)
        {
            enable_mask |= (UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT);
        }
        uart_hal_ena_intr_mask(&uart_hal_context, enable_mask);
        timer_hal_intr_enable(&timeout_hal_context);
    }
    else if ((Enable == false) && (irq_mutex != true))
    {
        irq_mutex = true;
#ifdef CONFIG_IDF_TARGET_ESP32
        uart_hal_disable_intr_mask(&uart_hal_context, (UART_INTR_TX_DONE));
#else
        uart_hal_disable_intr_mask(&uart_hal_context, (UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT | UART_INTR_TX_DONE));

#endif
        timer_hal_intr_disable(&timeout_hal_context);
    }
    irq_mutex = false;
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    uart_config_t uart_config = {
        .baud_rate  = Baudrate,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    if (uart_is_driver_installed(LUOS_COM) == true)
    {
        ESP_ERROR_CHECK(uart_driver_delete(LUOS_COM));
    }
    ESP_ERROR_CHECK(uart_driver_install(LUOS_COM, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 0, NULL, ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(uart_param_config(LUOS_COM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(LUOS_COM, COM_TX_PIN, COM_RX_PIN, GPIO_NUM_NC, GPIO_NUM_NC));

    uart_hal_set_rxfifo_full_thr(&uart_hal_context, 1);
    uart_hal_rxfifo_rst(&uart_hal_context);
    uart_hal_txfifo_rst(&uart_hal_context);
    ESP_ERROR_CHECK(uart_isr_free(LUOS_COM));
    ESP_ERROR_CHECK(uart_isr_register(LUOS_COM, &RobusHAL_ComIrqHandler, NULL, ESP_INTR_FLAG_IRAM, &handle_console));

    uart_hal_disable_intr_mask(&uart_hal_context, UART_LL_INTR_MASK);

    RobusHAL_SetTxState(false);
    RobusHAL_SetRxState(true);

    // Timeout Initialization
    Timer_Prescaler = (uint32_t)(MCUFREQ / Baudrate); // / TIMERDIV;
    RobusHAL_TimeoutInit();
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_SetTxState(uint8_t Enable)
{
    if (TxEn == Enable)
    {
        return;
    }
    TxEn = Enable;
    if (Enable == true)
    {
        // Put Tx in push pull
        esp_rom_gpio_connect_out_signal(COM_TX_PIN, UART_PERIPH_SIGNAL(LUOS_COM, SOC_UART_TX_PIN_IDX), 0, 0);
        if (TX_EN_PIN != DISABLE)
        {
            gpio_hal_set_level(&gpio_hal_context, TX_EN_PIN, 1);
        }
    }
    else
    {
        // Put Tx in open drain
        gpio_hal_output_disable(&gpio_hal_context, COM_TX_PIN);
        esp_rom_gpio_pad_select_gpio(COM_TX_PIN);
        if (TX_EN_PIN != DISABLE)
        {
            gpio_hal_set_level(&gpio_hal_context, TX_EN_PIN, 0);
        }
        uart_hal_disable_intr_mask(&uart_hal_context, UART_INTR_TX_DONE);
        uart_hal_txfifo_rst(&uart_hal_context);
    }
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
_CRITICAL void RobusHAL_SetRxState(uint8_t Enable)
{
    if (RxEn == Enable)
    {
        return;
    }
    RxEn = Enable;
#ifdef CONFIG_IDF_TARGET_ESP32
    if (Enable == true)
    {
        uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT);
    }
#else
    if (Enable == true)
    {
        uart_hal_rxfifo_rst(&uart_hal_context);
        uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT);
        uart_hal_ena_intr_mask(&uart_hal_context, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT);
    }
    else
    {
        uart_hal_disable_intr_mask(&uart_hal_context, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT);
    }
#endif
}
/******************************************************************************
 * @brief Process data send or receive
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ComIrqHandler(void *arg)
{
    uint8_t data[RX_BUFFER_SIZE];
    int size = 0;

    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);

    uint32_t Flag      = uart_hal_get_intsts_mask(&uart_hal_context);
    uint32_t UartIntEn = uart_hal_get_intr_ena_status(&uart_hal_context);

    if ((((Flag & UART_INTR_RXFIFO_FULL) == UART_INTR_RXFIFO_FULL) && ((UartIntEn & UART_INTR_RXFIFO_FULL) == UART_INTR_RXFIFO_FULL)) || (((Flag & UART_INTR_RXFIFO_TOUT) == UART_INTR_RXFIFO_TOUT) && ((UartIntEn & UART_INTR_RXFIFO_TOUT) == UART_INTR_RXFIFO_TOUT)))
    {
        uart_hal_read_rxfifo(&uart_hal_context, &data[0], &size);
        uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT);
        if (RxEn == true)
        {
            Recep_data(&data[size - 1]);
        }
        else
        {
            return;
        }
    }
    else if (Flag & UART_INTR_FRAM_ERR)
    {
        uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_FRAM_ERR);
        ctx.rx.status.rx_framing_error = true;
    }

    if (((Flag & UART_INTR_TX_DONE) == UART_INTR_TX_DONE) && ((UartIntEn & UART_INTR_TX_DONE) == UART_INTR_TX_DONE))
    {
        if (data_size_to_transmit == 0)
        {
            RobusHAL_SetTxState(false);
            RobusHAL_SetRxState(true);
            tx_data = 0;
        }
        else
        {
#ifndef CONFIG_IDF_TARGET_ESP32
            RobusHAL_ResetTimeout(0);
#endif
            uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_TX_DONE);
            uart_hal_write_txfifo(&uart_hal_context, tx_data, data_size_to_transmit, &Rsize);
            data_size_to_transmit = 0;
        }
    }
    uart_hal_clr_intsts_mask(&uart_hal_context, UART_LL_INTR_MASK);
}
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    while ((uart_hal_get_intsts_mask(&uart_hal_context) & UART_INTR_TXFIFO_EMPTY) != 0)
        ;
    if (size > 1)
    {
        if (size <= 128)
        {
            uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_TX_DONE);
            uart_hal_ena_intr_mask(&uart_hal_context, UART_INTR_TX_DONE);
            RobusHAL_SetTxState(true);
            uart_hal_write_txfifo(&uart_hal_context, data, size, &Rsize);
            data_size_to_transmit = 0;
        }
        else
        {
            tx_data = data + 128;
            uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_TX_DONE);
            uart_hal_ena_intr_mask(&uart_hal_context, UART_INTR_TX_DONE);
            RobusHAL_SetTxState(true);
            uart_hal_write_txfifo(&uart_hal_context, data, 128, &Rsize);
            data_size_to_transmit = size - 128;
        }
    }
    else
    {
        // wait before send ack
        // this is a patch du to difference MCU frequency
        if (size != 0)
        {
            uint32_t ActualTime = esp_timer_get_time();
            while ((esp_timer_get_time() - ActualTime) < (uint32_t)(TIMEOUT_ACK))
                ;
            uart_hal_clr_intsts_mask(&uart_hal_context, UART_INTR_TX_DONE);
            uart_hal_ena_intr_mask(&uart_hal_context, UART_INTR_TX_DONE);
            RobusHAL_SetTxState(true);
            uart_hal_write_txfifo(&uart_hal_context, data, size, &Rsize);
            data_size_to_transmit = 0;
        }
    }
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
_CRITICAL void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
    if (TX_LOCK_DETECT_PIN != DISABLE)
    {
    }
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
_CRITICAL uint8_t RobusHAL_GetTxLockState(void)
{
    uint8_t result = false;

    if (uart_hal_context.dev->status.rxd == 0)
    {
        RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
        result = true;
    }
    return result;
}
/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
    timer_config_t Timeout;

    Timeout.alarm_en    = TIMER_ALARM_EN;      /*!< Timer alarm enable */
    Timeout.counter_en  = TIMER_PAUSE;         /*!< Counter enable */
    Timeout.intr_type   = TIMER_INTR_LEVEL;    /*!< Interrupt mode */
    Timeout.counter_dir = TIMER_COUNT_UP;      /*!< Counter direction  */
    Timeout.auto_reload = TIMER_AUTORELOAD_EN; /*!< Timer auto-reload */
    Timeout.divider     = Timer_Prescaler - 1; /*!< Counter clock divider. The divider's range is from from 2 to 65536. */
    timer_init(LUOS_TIMER_GROUP, LUOS_TIMER, &Timeout);
    timer_isr_callback_add(LUOS_TIMER_GROUP, LUOS_TIMER, &RobusHAL_TimeoutIrqHandler, NULL, ESP_INTR_FLAG_IRAM);
    timer_hal_set_alarm_value(&timeout_hal_context, DEFAULT_TIMEOUT);
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    // disable Counter
    timer_hal_set_counter_value(&timeout_hal_context, 0);
    timer_hal_set_counter_enable(&timeout_hal_context, TIMER_PAUSE);
    timer_hal_clear_intr_status(&timeout_hal_context);
    timer_hal_intr_disable(&timeout_hal_context);

    // Reset counter
    if (nbrbit != 0)
    {
        timer_hal_intr_enable(&timeout_hal_context);
        timer_hal_set_counter_enable(&timeout_hal_context, TIMER_START);
    }
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL timer_isr_t RobusHAL_TimeoutIrqHandler(void *arg)
{
    timer_hal_set_counter_enable(&timeout_hal_context, TIMER_PAUSE);
    timer_hal_clear_intr_status(&timeout_hal_context);
    if ((ctx.tx.lock == true) && (RobusHAL_GetTxLockState() == false))
    {
        // Enable RX detection pin if needed
        uart_hal_rxfifo_rst(&uart_hal_context);
        RobusHAL_SetTxState(false);
        RobusHAL_SetRxState(true);
        Recep_Timeout();
    }
    return 0;
}
/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
    if (RX_EN_PIN != DISABLE)
    {
        /*Configure GPIO pins : RxEN_Pin */
        gpio_reset_pin(RX_EN_PIN);
        PinConfig.intr_type    = GPIO_INTR_DISABLE;
        PinConfig.mode         = GPIO_MODE_OUTPUT;
        PinConfig.pin_bit_mask = (1ULL << RX_EN_PIN);
        PinConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
        PinConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
        gpio_config(&PinConfig);
    }

    if (TX_EN_PIN != DISABLE)
    {
        /*Configure GPIO pins : TxEN_Pin */
        gpio_reset_pin(TX_EN_PIN);
        PinConfig.intr_type    = GPIO_INTR_DISABLE;
        PinConfig.mode         = GPIO_MODE_OUTPUT;
        PinConfig.pin_bit_mask = (1ULL << TX_EN_PIN);
        PinConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
        PinConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
        gpio_config(&PinConfig);
    }

    // configure PTP
    RobusHAL_RegisterPTP();
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        // Setup PTP lines
        gpio_reset_pin(PTP[i].Pin);
        PinConfig.intr_type    = GPIO_INTR_DISABLE;
        PinConfig.mode         = GPIO_MODE_OUTPUT_OD;
        PinConfig.pin_bit_mask = (1ULL << PTP[i].Pin);
        PinConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
        PinConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
        gpio_config(&PinConfig);
        gpio_intr_disable(PTP[i].Pin);
        // activate IT for PTP
        gpio_isr_handler_add(PTP[i].Pin, &RobusHAL_PinoutIrqHandler, (void *)PTP[i].Pin);
        RobusHAL_SetPTPDefaultState(i);
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
_CRITICAL void RobusHAL_PinoutIrqHandler(void *arg)
{
    if ((TX_LOCK_DETECT_PIN != DISABLE) && ((uint32_t)(arg) == TX_LOCK_DETECT_PIN))
    {
        ctx.tx.lock = true;
        RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
    }
    else
    {
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            if ((uint32_t)(arg) == PTP[i].Pin)
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
_CRITICAL void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    // Pull Down / IT mode / Rising Edge
    gpio_intr_disable(PTP[PTPNbr].Pin);
    gpio_set_direction(PTP[PTPNbr].Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PTP[PTPNbr].Pin, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(PTP[PTPNbr].Pin, GPIO_INTR_POSEDGE);
    gpio_intr_enable(PTP[PTPNbr].Pin);
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    // Pull Down / IT mode / Falling Edge
    gpio_intr_disable(PTP[PTPNbr].Pin);
    gpio_set_direction(PTP[PTPNbr].Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PTP[PTPNbr].Pin, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(PTP[PTPNbr].Pin, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(PTP[PTPNbr].Pin);
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    gpio_intr_disable(PTP[PTPNbr].Pin);
    gpio_set_direction(PTP[PTPNbr].Pin, GPIO_MODE_OUTPUT);
    gpio_set_intr_type(PTP[PTPNbr].Pin, GPIO_INTR_DISABLE);

    // Clean edge/state detection and set the PTP pin as output
    gpio_hal_set_level(&gpio_hal_context, PTP[PTPNbr].Pin, 1);
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
_CRITICAL uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
    gpio_intr_disable(PTP[PTPNbr].Pin);
    gpio_set_direction(PTP[PTPNbr].Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PTP[PTPNbr].Pin, GPIO_FLOATING);
    gpio_set_intr_type(PTP[PTPNbr].Pin, GPIO_INTR_DISABLE);

    return gpio_get_level(PTP[PTPNbr].Pin);
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
#if (USE_CRC_HW == 1)
#endif
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
_CRITICAL void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
#if (USE_CRC_HW == 1)

#else
    uint16_t dbyte = *data;
    *(uint16_t *)crc ^= dbyte << 8;
    for (uint8_t j = 0; j < 8; ++j)
    {
        uint16_t mix = *(uint16_t *)crc & 0x8000;
        *(uint16_t *)crc = (*(uint16_t *)crc << 1);
        if (mix)
            *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
    }
#endif
}
