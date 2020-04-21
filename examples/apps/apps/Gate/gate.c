#include "main.h"
#include "gate.h"
#include "json_mnger.h"
#include "luos_board.h"

module_t *module;
msg_t msg;
uint8_t RxData;
module_t *module_pointer;
volatile msg_t pub_msg;
volatile int pub = LUOS_PROTOCOL_NB;

#ifdef USE_SERIAL
int serial_write(char *data, int len)
{
   for (unsigned short i = 0; i < len; i++) {
        while (!LL_USART_IsActiveFlag_TXE(USART3));
        LL_USART_TransmitData8(USART3,*(data+i));
    }
   return 0;
}
#endif

void USART3_4_IRQHandler(void)
{
    // check if we receive an IDLE on usart3
    if (LL_USART_IsActiveFlag_IDLE(USART3)) {
        LL_USART_ClearFlag_IDLE(USART3);
        // check DMA data
        check_json(JSON_BUFF_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_3) - 1);

        // reset DMA
        __disable_irq();
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
        char* addr = get_json_buf();
        LL_DMA_SetM2MDstAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)addr);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, JSON_BUFF_SIZE);
        LL_DMA_SetM2MSrcAddress(DMA1, LL_DMA_CHANNEL_3,(uint32_t)&USART3->RDR);
        LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)addr);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
        LL_USART_EnableDMAReq_RX(USART3);
        __enable_irq();
    }
}

void gate_init(void) {
    LL_USART_ClearFlag_IDLE(USART3);
    LL_USART_EnableIT_IDLE(USART3);
    NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);
    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_DisableIT_TE(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_SetM2MDstAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)get_json_buf());
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, JSON_BUFF_SIZE);
    LL_DMA_SetM2MSrcAddress(DMA1, LL_DMA_CHANNEL_3,(uint32_t)&USART3->RDR);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_USART_EnableDMAReq_RX(USART3);
    module = luos_module_create(0, GATE_MOD, "gate");
    status_led(0);
}

void gate_loop(void) {
    static unsigned int keepAlive = 0;
    static volatile uint8_t detection_done = 0;
    static char state = 0;

    // Check if there is a dead module
    if (module->vm->dead_module_spotted) {
        char json[JSON_BUFF_SIZE] = {0};
        exclude_module_to_json(module->vm->dead_module_spotted, json);
        #ifdef USE_SERIAL
            serial_write(json, strlen(json));
        #else
            printf(json);
        #endif
        module->vm->dead_module_spotted = 0;
    }
    if (detection_done) {
        char json[JSON_BUFF_SIZE] = {0};
        state = !state;
        status_led(state);
        format_data(module, json);
        if (json[0] != '\0') {
            #ifdef USE_SERIAL
                serial_write(json, strlen(json));
            #else
                printf(json);
            #endif
            keepAlive = 0;
        } else {
            if (keepAlive > 200) {
                #ifdef USE_SERIAL
                    sprintf(json, "{}\n");
                    serial_write(json, strlen(json));
                #else
                    printf("{}\n");
                #endif

            } else {
                keepAlive++;
            }
        }
        collect_data(module);
    }
    if (pub != LUOS_PROTOCOL_NB) {
        luos_send(module_pointer, &pub_msg);
        pub = LUOS_PROTOCOL_NB;
    }
    // check if serial input messages ready and convert it into a luos message
    send_cmds(module);
    if (detection_ask) {
        char json[JSON_BUFF_SIZE*2] = {0};
        detect_modules(module);
        route_table_to_json(json);
        #ifdef USE_SERIAL
            serial_write(json, strlen(json));
        #else
            printf(json);
        #endif
        detection_done = 1;
        detection_ask = 0;
    }
    HAL_Delay(get_delay());
}
