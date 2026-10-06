/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstraction Layer for Zephyr (Cortex-M).
 ******************************************************************************/
#include "luos_hal.h"
#include <zephyr/kernel.h>
#include <cmsis_core.h>

void LuosHAL_Init(void)
{
    /* Zephyr owns the system tick and cycle counter; nothing to start. */
}

/* Mask all interrupts (including the radio IRQ) so the engine can guard its RX
 * ring against Serial_ReceptionWrite running from the radio ISR. Paired
 * (false then true) by the engine; not nested. */
void LuosHAL_SetIrqState(bool Enable)
{
    if (Enable)
    {
        __enable_irq();
    }
    else
    {
        __disable_irq();
    }
}

uint32_t LuosHAL_GetSystick(void)
{
    return k_uptime_get_32(); /* milliseconds */
}

uint64_t LuosHAL_GetTimestamp(void)
{
    return k_cyc_to_ns_floor64(k_cycle_get_64()); /* nanoseconds */
}

void LuosHAL_StartTimestamp(void) {}
void LuosHAL_StopTimestamp(void) {}
