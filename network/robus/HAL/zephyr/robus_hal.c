/******************************************************************************
 * @file robus_HAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family XXX
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(robus_hal);

#include "robus_hal.h"
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>
#include "reception.h"
#include "context.h"

static volatile bool tx_en;
static volatile bool rx_en;

static const struct device *luos_serial = DEVICE_DT_GET(DT_NODELABEL(usart1));
static const struct device *luos_ptpa_gpio = DEVICE_DT_GET(DT_NODELABEL(gpioa));
static const struct device *luos_ptpb_gpio = DEVICE_DT_GET(DT_NODELABEL(gpiob));
static const struct device *luos_tx_gpio = DEVICE_DT_GET(DT_NODELABEL(gpioc));
static const struct device *luos_timeout = DEVICE_DT_GET(DT_NODELABEL(rtc));
static struct gpio_callback gpio_cb;

static char uart_out_buffer[CONFIG_LUOS_RING_BUF_SIZE];
static struct ring_buf out_ringbuf;
static struct counter_alarm_cfg alarm_cfg;

K_SEM_DEFINE(net_activity_sem, 0, 1);

/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);

static void RobusHAL_UART_ISR(const struct device *dev, void *user_data)
{
    RobusHAL_ResetTimeout(CONFIG_LUOS_TIMEOUT_USEC);

    if(!uart_irq_update(luos_serial))
    {
        return;
    }

    if(uart_irq_tx_ready(luos_serial)) 
    {
        uint8_t c;

        if(ring_buf_is_empty(&out_ringbuf)) 
        {
            uart_irq_tx_disable(luos_serial);
            RobusHAL_SetTxState(false);
        }
        else 
        {
            RobusHAL_ResetTimeout(0);
            ring_buf_get(&out_ringbuf, &c, 1);
            uart_fifo_fill(luos_serial, &c, 1);
            LOG_DBG("Transmitting the following byte from serial 0x%x", c);
        }
        k_sem_give(&net_activity_sem);
    }
    
	if (uart_irq_rx_ready(luos_serial)) 
    {
        uint8_t c;

		uart_fifo_read(luos_serial, &c, 1);

        if (rx_en == true)
        {
            ctx.rx.callback(&c);
        }

        LOG_DBG("Received the following byte from serial 0x%x", c);

        k_sem_give(&net_activity_sem);        
	}
}

static void RobusHAL_GPIO_ISR(const struct device *gpiob, struct gpio_callback *cb,
		                    uint32_t pins)
{
    LOG_DBG("PTP pin was asserted! ");

    if(pins & (1 << CONFIG_PTPA_PIN_NBR))
    {
        PortMng_PtpHandler(0);
    }
    else if (pins & (1 << CONFIG_PTPB_PIN_NBR))
    {
        PortMng_PtpHandler(1);
    }

    k_sem_give(&net_activity_sem);
}

static void RobusHAL_Counter_ISR(const struct device *counter_dev,
				      uint8_t chan_id, uint32_t ticks,
				      void *user_data)
{
    LOG_DBG("Reception timeout..");
    if ((ctx.tx.lock == true) && (RobusHAL_GetTxLockState() == false))
    {
        // Enable RX detection pin if needed
        RobusHAL_SetTxState(false);
        RobusHAL_SetRxState(true);
        Recep_Timeout();
    }
}

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Init(void)
{
    // IO Initialization
    RobusHAL_GPIOInit();

    // Com Initialization
    RobusHAL_ComInit(0);
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    /* baud rate is referred from the device tree*/
    (void)Baudrate;

    ring_buf_init(&out_ringbuf, sizeof(uart_out_buffer), uart_out_buffer);
    uart_irq_callback_user_data_set(luos_serial, RobusHAL_UART_ISR, NULL);
	uart_irq_rx_disable(luos_serial);
	uart_irq_tx_disable(luos_serial);
    RobusHAL_SetTxState(false);
    RobusHAL_SetRxState(true);
    RobusHAL_TimeoutInit();
}
/******************************************************************************
 * @brief Tx enable/disable
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
    if(tx_en == Enable)
    {
        return;
    }

    tx_en = Enable;
    if (Enable == true)
    {
        LOG_INF("TX State set to true");
        gpio_pin_configure(luos_tx_gpio, CONFIG_TX_PIN_NBR, 
                GPIO_OUTPUT | GPIO_INT_DISABLE | GPIO_PUSH_PULL);
    }
    else
    {
        LOG_INF("TX State set to false");
        gpio_pin_configure(luos_tx_gpio, CONFIG_TX_PIN_NBR, 
                GPIO_INPUT | GPIO_INT_DISABLE | GPIO_OPEN_DRAIN);
    }
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void RobusHAL_SetRxState(uint8_t Enable)
{
    if(rx_en == Enable)
    {
        return;
    }

    rx_en = Enable;
    if (Enable == true)
    {
        LOG_INF("RX State set to true");
        uart_irq_rx_enable(luos_serial);
    }
    else
    {
        LOG_INF("RX State set to false");
        uart_irq_rx_disable(luos_serial);
    }
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
    uint32_t room;
    bool empty; 

    //wait enough space into tx bufffer
    do 
    {
        room = ring_buf_space_get(&out_ringbuf);
    } while(room < size);

    empty = ring_buf_is_empty(&out_ringbuf);
    ring_buf_put(&out_ringbuf, (const uint8_t *)data, size);

    if(empty) 
    {
        RobusHAL_SetTxState(true);
        uart_irq_tx_enable(luos_serial);
    }

    RobusHAL_ResetTimeout(CONFIG_LUOS_TIMEOUT_USEC);
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
    alarm_cfg.flags = 0;
    alarm_cfg.ticks = counter_us_to_ticks(luos_timeout, CONFIG_LUOS_TIMEOUT_USEC);
    alarm_cfg.callback = RobusHAL_Counter_ISR;
    alarm_cfg.user_data = &alarm_cfg;
    RobusHAL_ResetTimeout(CONFIG_LUOS_TIMEOUT_USEC);
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    // disable Counter
    counter_stop(luos_timeout);
    counter_cancel_channel_alarm(luos_timeout, 0);

    // Reset counter
    if (nbrbit != 0)
    {
        counter_set_channel_alarm(luos_timeout, 0, &alarm_cfg);
        counter_start(luos_timeout);
    }
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

    gpio_pin_configure(luos_ptpa_gpio, CONFIG_PTPA_PIN_NBR, 
                    GPIO_INPUT | GPIO_INT_DISABLE | GPIO_OPEN_DRAIN | GPIO_PULL_DOWN);
    gpio_init_callback(&gpio_cb, RobusHAL_GPIO_ISR, BIT(CONFIG_PTPA_PIN_NBR));
    gpio_add_callback(luos_ptpa_gpio, &gpio_cb);

    gpio_pin_configure(luos_ptpb_gpio, CONFIG_PTPB_PIN_NBR,
                    GPIO_INPUT | GPIO_INT_DISABLE | GPIO_OPEN_DRAIN | GPIO_PULL_DOWN);
    gpio_init_callback(&gpio_cb, RobusHAL_GPIO_ISR, BIT(CONFIG_PTPB_PIN_NBR));
    gpio_add_callback(luos_ptpb_gpio, &gpio_cb);

    RobusHAL_SetPTPDefaultState(0);
    RobusHAL_SetPTPDefaultState(1);
}
/******************************************************************************
 * @brief Set PTP for branch Detection
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    if(!PTPNbr) 
    {
        gpio_pin_configure(luos_ptpa_gpio, CONFIG_PTPA_PIN_NBR, 
                        GPIO_INPUT | GPIO_INT_EDGE_RISING | GPIO_OPEN_DRAIN | GPIO_PULL_DOWN);
    }
    else 
    {
        gpio_pin_configure(luos_ptpb_gpio, CONFIG_PTPB_PIN_NBR, 
                        GPIO_INPUT | GPIO_INT_EDGE_RISING | GPIO_OPEN_DRAIN | GPIO_PULL_DOWN);
    } 
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    if(!PTPNbr) 
    {
        gpio_pin_configure(luos_ptpa_gpio, CONFIG_PTPA_PIN_NBR, 
                        GPIO_INPUT | GPIO_INT_EDGE_FALLING | GPIO_OPEN_DRAIN | GPIO_PULL_DOWN);
    }
    else 
    {
        gpio_pin_configure(luos_ptpb_gpio, CONFIG_PTPB_PIN_NBR, 
                        GPIO_INPUT | GPIO_INT_EDGE_FALLING | GPIO_OPEN_DRAIN | GPIO_PULL_DOWN);
    }
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    if(!PTPNbr) 
    {
        gpio_pin_configure(luos_ptpa_gpio, CONFIG_PTPA_PIN_NBR, 
                        GPIO_OUTPUT | GPIO_INT_DISABLE| GPIO_PUSH_PULL);
        gpio_pin_set(luos_ptpa_gpio, CONFIG_PTPA_PIN_NBR, 1);
    }
    else 
    {
        gpio_pin_configure(luos_ptpb_gpio, CONFIG_PTPB_PIN_NBR, 
                        GPIO_OUTPUT | GPIO_INT_DISABLE| GPIO_PUSH_PULL);
        gpio_pin_set(luos_ptpb_gpio, CONFIG_PTPB_PIN_NBR, 1);
    }
}

/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    if(!PTPNbr) 
    {
        return (uint8_t) gpio_pin_get(luos_ptpa_gpio, CONFIG_PTPA_PIN_NBR);
    }
    else 
    {
        return (uint8_t) gpio_pin_get(luos_ptpb_gpio, CONFIG_PTPB_PIN_NBR);
    }
}

/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
    uint16_t dbyte = *data;
    *(uint16_t *)crc ^= dbyte << 8;
    for (uint8_t j = 0; j < 8; ++j)
    {
        uint16_t mix = *(uint16_t *)crc & 0x8000;
        *(uint16_t *)crc = (*(uint16_t *)crc << 1);
        if (mix)
            *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
    }
}

/******************************************************************************
 * @brief Wait a signal from the net activity
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_WaitNetworkActivity(void)
{
    k_sem_take(&net_activity_sem, K_FOREVER);
}
