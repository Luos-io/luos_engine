#ifndef __LUOS_BOARD_H
#define __LUOS_BOARD_H

#include "luos.h"
#include "adc.h"

#define TEMP110_CAL_VALUE ((uint16_t *)((uint32_t)0x1FFF75CA))
#define TEMP30_CAL_VALUE ((uint16_t *)((uint32_t)0x1FFF75A8))
//#define VOLTAGEFACTOR (10.0f + 68.0f) / 10.0f
#define VOLTAGEFACTOR ((100.0f + 20.0f) / 100.0f)
#define LUOS_UUID ((uint32_t *)0x1FFF7590)
#define VREF 2.5f
// This structure need to list all ADC configured in the good order determined by the
// ADC_CHANEL number in increasing order
typedef struct __attribute__((__packed__))
{
    union {
        struct __attribute__((__packed__))
        {
            uint32_t voltage_sensor;
            uint32_t temperature_sensor;
        };
        uint32_t unmap[2]; /*!< Uncmaped form. */
    };
} node_analog_t;

volatile node_analog_t node_analog;
ADC_HandleTypeDef luos_adc;
DMA_HandleTypeDef luos_dma_adc;

int node_msg_handler(module_t *module, msg_t *input, msg_t *output);
void status_led(char state);
void node_init(void);
void node_loop(void);
void write_alias(unsigned short local_id, char *alias);
char read_alias(unsigned short local_id, char *alias);

/**
  * \fn void board_disable_irq(void)
  * \brief disable IRQ
  *
  * \return error
  */
void node_disable_irq(void);

/**
  * \fn void board_enable_irq(void)
  * \brief enable IRQ
  *
  * \return error
  */
void node_enable_irq(void);
#endif /*__ __LUOS_BOARD_H */
