/******************************************************************************
 * @file serial_network_hal (ZEPHYR / radio)
 * @brief The radio is a best-effort byte pipe; the serial network handles all
 *        framing (0x7E/size/0x81) and reassembles from the engine RX ring.
 *        One Luos frame (<=139 B) fits one radio packet (<=255 B): SerialHAL_Send
 *        issues exactly one radio_send().
 *
 *        TX completion is SYNCHRONOUS: serial_network.c busy-waits on its
 *        `sending` flag in places that never pump this HAL (notably the
 *        `while (sending == true);` spin at the top of Serial_RunTopology), and
 *        every other Luos serial HAL clears `sending` from the TX-complete IRQ.
 *        So SerialHAL_Send must leave `sending` cleared on return: it transmits,
 *        waits for the radio TX-done IRQ, then calls Serial_TransmissionEnd()
 *        from this (caller) context. It must NOT be called from the radio ISR,
 *        which would re-enter radio_send and corrupt the radio state machine.
 ******************************************************************************/
#include "serial_network_hal.h"
#include "_serial_network.h"
#include "radio.h"

static volatile bool tx_done;

/* Radio IRQ context: push received bytes straight into the engine RX ring. */
static void on_radio_rx(const uint8_t *data, uint8_t len)
{
    Serial_ReceptionWrite((uint8_t *)data, (uint32_t)len);
}

/* Radio IRQ context: a transmit has completed. */
static void on_radio_tx_done(void)
{
    tx_done = true;
}

void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size)
{
    (void)rx_buffer;    /* the engine owns the ring; bytes arrive via Serial_ReceptionWrite */
    (void)buffer_size;
    tx_done = false;
    radio_init(on_radio_rx, on_radio_tx_done);
}

void SerialHAL_Loop(void)
{
    /* TX completion is handled synchronously in SerialHAL_Send. */
}

void SerialHAL_Send(uint8_t *data, uint16_t size)
{
    tx_done = false;
    while (!radio_send(data, (uint8_t)size))
    {
        /* a previous transmit is still draining; wait for the radio to free up */
    }
    while (!tx_done)
    {
        /* wait for this transmit to complete (tx_done set in the radio IRQ) */
    }
    Serial_TransmissionEnd();
}

uint8_t SerialHAL_GetPort(void)
{
    return 0;
}
