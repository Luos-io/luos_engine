/******************************************************************************
 * @file serial_network_hal (ZEPHYR / radio)
 * @brief The radio is a best-effort byte pipe; the serial network handles all
 *        framing (0x7E/size/0x81) and reassembles from the engine RX ring.
 *        One Luos frame (<=139 B) fits one radio packet (<=255 B): SerialHAL_Send
 *        issues exactly one radio_send().
 ******************************************************************************/
#include "serial_network_hal.h"
#include "_serial_network.h"
#include "radio.h"

static volatile bool tx_done_pending;

/* Radio IRQ context: push received bytes straight into the engine RX ring. */
static void on_radio_rx(const uint8_t *data, uint8_t len)
{
    Serial_ReceptionWrite((uint8_t *)data, (uint32_t)len);
}

/* Radio IRQ context: defer TX completion out of the ISR (see SerialHAL_Loop). */
static void on_radio_tx_done(void)
{
    tx_done_pending = true;
}

void SerialHAL_Init(uint8_t *rx_buffer, uint32_t buffer_size)
{
    (void)rx_buffer;    /* the engine owns the ring; bytes arrive via Serial_ReceptionWrite */
    (void)buffer_size;
    tx_done_pending = false;
    radio_init(on_radio_rx, on_radio_tx_done);
}

/* Called from Serial_Loop() in main context. Calling Serial_TransmissionEnd here
 * (rather than from on_radio_tx_done) keeps radio_send out of the radio ISR. */
void SerialHAL_Loop(void)
{
    if (tx_done_pending)
    {
        tx_done_pending = false;
        Serial_TransmissionEnd();
    }
}

void SerialHAL_Send(uint8_t *data, uint16_t size)
{
    radio_send(data, (uint8_t)size); /* size <= 139; best-effort single packet */
}

uint8_t SerialHAL_GetPort(void)
{
    return 0;
}
