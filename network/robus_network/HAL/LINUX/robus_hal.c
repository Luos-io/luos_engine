/******************************************************************************
 * @file robus_HAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @Family Linux (Raspberry Pi)
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "robus_hal.h"
#include "reception.h"
#include "context.h"
#include "luos_hal.h"
#include "port_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <gpiod.h>
#include <errno.h>
#include <time.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 100

/*******************************************************************************
 * Variables
 ******************************************************************************/
static int serial_fd                = -1;
static int timer_fd                 = -1;
static pthread_t rx_thread;
static volatile bool running        = false;
static volatile bool rx_enabled      = true;
static volatile bool tx_in_progress  = false;
static uint64_t timeout_ns_per_bit   = 0;

// GPIO via libgpiod v2
static struct gpiod_chip *gpio_chip            = NULL;
static struct gpiod_line_request *tx_en_req    = NULL;
static struct gpiod_line_request *ptp_reqs[NBR_PORT];
static unsigned int ptp_gpio_offsets[NBR_PORT];
static volatile uint8_t ptp_last_value[NBR_PORT];
static volatile bool ptp_edge_detected[NBR_PORT];
static volatile int8_t ptp_edge_direction[NBR_PORT]; // 1=rising only, -1=falling only, 0=both

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
static void *RobusHAL_RxThread(void *arg);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_CRCInit(void);

/******************************************************************************
 * @brief Robus HAL initialization
 ******************************************************************************/
void RobusHAL_Init(void)
{
    RobusHAL_GPIOInit();
    RobusHAL_CRCInit();
    RobusHAL_ComInit(ROBUS_NETWORK_BAUDRATE);
}

/******************************************************************************
 * @brief Robus HAL main loop
 ******************************************************************************/
void RobusHAL_Loop(void)
{
}

/******************************************************************************
 * @brief Initialize serial communication
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    serial_fd = open(ROBUS_COM_DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd < 0)
    {
        perror("RobusHAL: Failed to open serial port");
        return;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(serial_fd, &tty) != 0)
    {
        perror("RobusHAL: tcgetattr failed");
        close(serial_fd);
        serial_fd = -1;
        return;
    }

    cfmakeraw(&tty);
    // Only 1 Mbps is supported. For other baud rates, add a Baudrate-to-Bx mapping.
    _Static_assert(ROBUS_NETWORK_BAUDRATE == 1000000, "Linux HAL only supports 1 Mbps");
    (void)Baudrate;
    cfsetispeed(&tty, B1000000);
    cfsetospeed(&tty, B1000000);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0)
    {
        perror("RobusHAL: tcsetattr failed");
        close(serial_fd);
        serial_fd = -1;
        return;
    }
    tcflush(serial_fd, TCIOFLUSH);

    // Ensure non-blocking after tcsetattr (some drivers clear it)
    int flags = fcntl(serial_fd, F_GETFL, 0);
    fcntl(serial_fd, F_SETFL, flags | O_NONBLOCK);

    timeout_ns_per_bit = 1000000000ULL / Baudrate;

    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd < 0)
    {
        perror("RobusHAL: timerfd_create failed");
        close(serial_fd);
        serial_fd = -1;
        return;
    }

    running = true;
    if (pthread_create(&rx_thread, NULL, RobusHAL_RxThread, NULL) != 0)
    {
        perror("RobusHAL: pthread_create failed");
        running = false;
        close(timer_fd);
        close(serial_fd);
        timer_fd  = -1;
        serial_fd = -1;
        return;
    }
}

/******************************************************************************
 * @brief Set TX state (RS485 direction)
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
    if (tx_en_req == NULL)
    {
        return;
    }
    enum gpiod_line_value val = Enable ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
    gpiod_line_request_set_value(tx_en_req, TX_EN_GPIO_LINE, val);
}

/******************************************************************************
 * @brief Set RX state
 ******************************************************************************/
void RobusHAL_SetRxState(uint8_t Enable)
{
    rx_enabled = (Enable == true);
}

/******************************************************************************
 * @brief Transmit data over serial
 ******************************************************************************/
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    if (serial_fd < 0)
    {
        return;
    }

    // Guard against recursion: Transmit_End() -> Transmit_Process() ->
    // ComTransmit(). On MCU this doesn't happen because ComTransmit starts
    // async DMA and returns. On Linux it's synchronous, so the chain would
    // send all queued messages in a burst without yielding to the RX thread.
    // When recursive, just return — the next TX will be triggered by the
    // main loop calling Transmit_Process() after yielding to the RX thread.
    if (tx_in_progress)
    {
        // Transmit_Process already set ctx.tx.status and ctx.rx.callback before
        // calling us. Undo both so Transmit_End() won't remove this unsent job
        // and incoming bytes aren't misinterpreted as collision echoes.
        // The next timer-triggered Transmit_End→Transmit_Process will send it.
        ctx.tx.status   = TX_DISABLE;
        ctx.rx.callback = Recep_GetHeader;
        return;
    }
    tx_in_progress = true;

    // Enable RS485 transceiver TX direction
    RobusHAL_SetTxState(true);
    ROBUS_DBG("[HAL_TX] Sending %d bytes\n", size);

    uint16_t remaining = size;
    uint8_t *ptr       = data;
    struct timespec tx_start;
    clock_gettime(CLOCK_MONOTONIC, &tx_start);
    while (remaining > 0)
    {
        ssize_t written = write(serial_fd, ptr, remaining);
        if (written > 0)
        {
            ptr += written;
            remaining -= written;
        }
        else if (written < 0 && errno != EAGAIN && errno != EINTR)
        {
            break;
        }
    }

    // Wait until bytes have actually cleared the UART, then drop DE.
    // tcdrain() is unusable on BCM-family PL011: it polls FR.BUSY which is
    // sticky on this hardware and blocks ~7.9 ms per call regardless of
    // frame size. TIOCOUTQ confirms the kernel TX buffer is drained well
    // before tcdrain returns. Sleep for the computed wire time (10 bits
    // per byte at the configured baudrate) plus a CPU-scheduling margin.
    #define TX_WIRE_MARGIN_NS 50000ULL  // 50 µs for CFS wakeup jitter
    uint64_t wire_ns = (uint64_t)size * 10ULL * timeout_ns_per_bit + TX_WIRE_MARGIN_NS;
    uint64_t deadline_ns = (uint64_t)tx_start.tv_sec * 1000000000ULL
                         + (uint64_t)tx_start.tv_nsec + wire_ns;
    struct timespec deadline = {
        .tv_sec  = deadline_ns / 1000000000ULL,
        .tv_nsec = deadline_ns % 1000000000ULL,
    };
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL) == EINTR)
    {
    }

    // Switch RS485 transceiver back to RX mode
    RobusHAL_SetTxState(false);

    // Consume echo bytes: on half-duplex RS485 we receive our own TX.
    // Read exactly 'size' bytes (our echo) so the RX thread only sees responses.
    {
        uint8_t discard;
        int remaining = size;
        while (remaining > 0)
        {
            int n = read(serial_fd, &discard, 1);
            if (n == 1)
            {
                remaining--;
            }
            else if (n < 0 && errno == EAGAIN)
            {
                struct pollfd pfd = {.fd = serial_fd, .events = POLLIN};
                poll(&pfd, 1, 5);
            }
            else
            {
                break;
            }
        }
    }
    ROBUS_DBG("[HAL_TX] Done, consumed %d echo bytes, TX_EN OFF\n", size);

    // On MCU, echo bytes flow through Recep_GetCollision() which advances
    // the collision state machine. On Linux, echoes are consumed above at
    // the HAL level, so the collision state machine never runs. We must
    // complete the TX path that MCU collision detection normally handles:
    // 1. Reset RX callback to prevent false collision on next message
    // 2. Mark TX as successful so Transmit_End() removes the job
    // 3. Call Transmit_End() to clean up (recursion guard above prevents chaining)
    if (ctx.rx.callback == Recep_GetCollision)
    {
        ctx.rx.callback = Recep_GetHeader;
    }
    ctx.tx.status = TX_OK;
    Transmit_End();

    // Keep tx.lock true to prevent Transmit_Process from sending the next
    // message within the same Phy_Dispatch loop. On MCU, DMA keeps tx.lock
    // true until the TX complete ISR fires. Here we simulate that by holding
    // the lock until ComTransmit returns, so the next TX only happens from
    // the RX thread's timer or the next main loop iteration.
    ctx.tx.lock    = true;
    tx_in_progress = false;

    // Arm timer to release tx.lock and trigger the next Transmit_Process
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
}

/******************************************************************************
 * @brief Get TX lock state (bus busy detection)
 ******************************************************************************/
uint8_t RobusHAL_GetTxLockState(void)
{
    return false;
}

/******************************************************************************
 * @brief Reset/set the frame timeout timer
 ******************************************************************************/
void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    if (timer_fd < 0)
    {
        return;
    }
    if (nbrbit == 0)
    {
        struct itimerspec its = {0};
        timerfd_settime(timer_fd, 0, &its, NULL);
        ROBUS_DBG("[HAL_TMR] DISARMED (fd=%d)\n", timer_fd);
        return;
    }
    uint64_t timeout_ns     = (uint64_t)nbrbit * timeout_ns_per_bit;
    struct itimerspec its    = {
        .it_value.tv_sec  = timeout_ns / 1000000000,
        .it_value.tv_nsec = timeout_ns % 1000000000,
        .it_interval      = {0, 0},
    };
    int ret = timerfd_settime(timer_fd, 0, &its, NULL);
    ROBUS_DBG("[HAL_TMR] Set timeout %llu ns (fd=%d, ret=%d)\n",
              (unsigned long long)timeout_ns, timer_fd, ret);
}

/******************************************************************************
 * @brief Set RX detection pin (no-op on Linux)
 ******************************************************************************/
void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
}

/******************************************************************************
 * @brief Set PTP to default state (input, polled for edges)
 ******************************************************************************/
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    if (PTPNbr >= NBR_PORT || gpio_chip == NULL)
    {
        return;
    }
    ROBUS_DBG("[HAL_PTP] SetDefault port %d\n", PTPNbr);

    if (ptp_reqs[PTPNbr] != NULL)
    {
        gpiod_line_request_release(ptp_reqs[PTPNbr]);
        ptp_reqs[PTPNbr] = NULL;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_DOWN);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    unsigned int offset                = ptp_gpio_offsets[PTPNbr];
    gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "robus_ptp");

    ptp_reqs[PTPNbr] = gpiod_chip_request_lines(gpio_chip, req_cfg, line_cfg);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    if (ptp_reqs[PTPNbr] != NULL)
    {
        enum gpiod_line_value val  = gpiod_line_request_get_value(ptp_reqs[PTPNbr], ptp_gpio_offsets[PTPNbr]);
        ptp_last_value[PTPNbr]     = (val == GPIOD_LINE_VALUE_ACTIVE) ? 1 : 0;
        ptp_edge_direction[PTPNbr] = 1; // Rising edge only (detect poke = LOW→HIGH)
        ptp_edge_detected[PTPNbr]  = true;
    }
}

/******************************************************************************
 * @brief Set PTP to reverse state (input, polled for edges)
 ******************************************************************************/
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    if (PTPNbr >= NBR_PORT || gpio_chip == NULL)
    {
        return;
    }

    if (ptp_reqs[PTPNbr] != NULL)
    {
        gpiod_line_request_release(ptp_reqs[PTPNbr]);
        ptp_reqs[PTPNbr] = NULL;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_DOWN);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    unsigned int offset                = ptp_gpio_offsets[PTPNbr];
    gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "robus_ptp");

    ptp_reqs[PTPNbr] = gpiod_chip_request_lines(gpio_chip, req_cfg, line_cfg);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    if (ptp_reqs[PTPNbr] != NULL)
    {
        enum gpiod_line_value val  = gpiod_line_request_get_value(ptp_reqs[PTPNbr], ptp_gpio_offsets[PTPNbr]);
        ptp_last_value[PTPNbr]     = (val == GPIOD_LINE_VALUE_ACTIVE) ? 1 : 0;
        ptp_edge_direction[PTPNbr] = -1; // Falling edge only (detect release = HIGH→LOW)
        ptp_edge_detected[PTPNbr]  = true;
    }
}

/******************************************************************************
 * @brief Push PTP line (output HIGH)
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    if (PTPNbr >= NBR_PORT || gpio_chip == NULL)
    {
        return;
    }
    ROBUS_DBG("[HAL_PTP] Push port %d\n", PTPNbr);

    if (ptp_reqs[PTPNbr] != NULL)
    {
        gpiod_line_request_release(ptp_reqs[PTPNbr]);
        ptp_reqs[PTPNbr] = NULL;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_ACTIVE);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    unsigned int offset                = ptp_gpio_offsets[PTPNbr];
    gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "robus_ptp");

    ptp_reqs[PTPNbr] = gpiod_chip_request_lines(gpio_chip, req_cfg, line_cfg);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    ptp_edge_detected[PTPNbr] = false;
}

/******************************************************************************
 * @brief Get PTP line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    if (PTPNbr >= NBR_PORT || ptp_reqs[PTPNbr] == NULL)
    {
        return 0;
    }
    enum gpiod_line_value val = gpiod_line_request_get_value(ptp_reqs[PTPNbr], ptp_gpio_offsets[PTPNbr]);
    return (val == GPIOD_LINE_VALUE_ACTIVE) ? 1 : 0;
}

/******************************************************************************
 * @brief Compute CRC16 (software)
 ******************************************************************************/
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
    uint16_t dbyte = data[0];
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
 * @brief GPIO Initialization
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
    gpio_chip = gpiod_chip_open(ROBUS_GPIO_CHIP);
    if (gpio_chip == NULL)
    {
        perror("RobusHAL: Failed to open GPIO chip");
        return;
    }

    // TX_EN: output, initially inactive
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    unsigned int tx_offset             = TX_EN_GPIO_LINE;
    gpiod_line_config_add_line_settings(line_cfg, &tx_offset, 1, settings);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "robus_tx_en");

    tx_en_req = gpiod_chip_request_lines(gpio_chip, req_cfg, line_cfg);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    // PTP lines: start as input with pull-down (no events yet)
    ptp_gpio_offsets[0] = PTPA_GPIO_LINE;
#if (NBR_PORT > 1)
    ptp_gpio_offsets[1] = PTPB_GPIO_LINE;
#endif

    for (uint8_t i = 0; i < NBR_PORT; i++)
    {
        ptp_reqs[i]          = NULL;
        ptp_last_value[i]     = 0;
        ptp_edge_detected[i]  = false;
        ptp_edge_direction[i] = 1; // Rising edge by default
    }
}

/******************************************************************************
 * @brief CRC Initialization (nothing to do for software CRC)
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
}

/******************************************************************************
 * @brief RX thread -- replaces UART, timer, and PTP ISRs
 ******************************************************************************/
static void *RobusHAL_RxThread(void *arg)
{
    (void)arg;
    uint8_t byte;

    while (running)
    {
        ROBUS_DBG("[HAL_LOOP] top\n");
        struct pollfd fds[2];
        int nfds = 0;

        fds[nfds].fd      = serial_fd;
        fds[nfds].events  = POLLIN;
        fds[nfds].revents = 0;
        int serial_idx    = nfds++;

        fds[nfds].fd      = timer_fd;
        fds[nfds].events  = POLLIN;
        fds[nfds].revents = 0;
        int timer_idx     = nfds++;

        // Use short timeout to sample PTP GPIO values frequently
        int ret = poll(fds, nfds, 1);

        ROBUS_DBG("[HAL_LOCK] acquiring mutex...\n");
        pthread_mutex_lock(&luos_recursive_mutex);
        ROBUS_DBG("[HAL_LOCK] acquired\n");

        if (ret > 0)
        {
            // --- Serial byte received (processed FIRST so pending messages
            //     like PORT_DATA are consumed before PTP triggers next step) ---
            if (fds[serial_idx].revents & POLLIN)
            {
                int count = 0;
                while (read(serial_fd, (uint8_t *)&byte, 1) == 1)
                {
                    count++;
                    if (rx_enabled)
                    {
                        // If the previous byte closed a valid frame, reset state
                        // so this byte starts a new frame. Robus normally relies
                        // on the DEFAULT_TIMEOUT inter-frame gap to trigger
                        // Recep_Timeout → Recep_Reset, but on RS485 without
                        // arbitration two nodes can TX back-to-back with a gap
                        // much shorter than the timeout, leaving the state in
                        // Recep_Drop and silently discarding the next frame.
                        if (ctx.rx.callback == Recep_Drop)
                        {
                            Recep_Reset();
                        }
                        ROBUS_DBG("[HAL_RX] 0x%02X\n", byte);
                        RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
                        Recep_data(&byte);
                    }
                }
                ROBUS_DBG("[HAL_RX] read loop done, got %d bytes\n", count);
            }

            // --- Timer expired ---
            if (fds[timer_idx].revents & POLLIN)
            {
                uint64_t expirations;
                read(timer_fd, &expirations, sizeof(expirations));
                ROBUS_DBG("[HAL_RX] Timeout\n");
                if (ctx.tx.lock == true && RobusHAL_GetTxLockState() == false)
                {
                    RobusHAL_SetTxState(false);
                    RobusHAL_SetRxState(true);
                }
                Recep_Timeout();
            }
        }

        // --- PTP: detect edges by reading GPIO value directly ---
        // Processed LAST so serial data (PORT_DATA) is consumed before
        // PTP falling edge triggers the next detection step.
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            if (!ptp_edge_detected[i] || ptp_reqs[i] == NULL)
            {
                continue;
            }
            enum gpiod_line_value val = gpiod_line_request_get_value(ptp_reqs[i], ptp_gpio_offsets[i]);
            uint8_t current           = (val == GPIOD_LINE_VALUE_ACTIVE) ? 1 : 0;
            if (current != ptp_last_value[i])
            {
                int8_t direction = (int8_t)current - (int8_t)ptp_last_value[i]; // +1=rising, -1=falling
                ptp_last_value[i] = current;
                // Only fire if direction matches what was requested
                if (ptp_edge_direction[i] == 0 || ptp_edge_direction[i] == direction)
                {
                    ROBUS_DBG("[HAL_PTP] Edge on port %d (val=%d)\n", i, current);
                    PortMng_PtpHandler(i);
                }
            }
        }

        pthread_mutex_unlock(&luos_recursive_mutex);
        ROBUS_DBG("[HAL_LOCK] released\n");
    }
    return NULL;
}
