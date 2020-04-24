#include "led_strip.h"
#include "main.h"
#include "tim.h"
#include "luos.h"

#define MAX_LED_NUMBER 375
#define OVERHEAD (9 * 24) // Number of data to add to create a reset between frames
#define DECOMP_BUFF_SIZE (MAX_LED_NUMBER * 24 + OVERHEAD)

volatile color_t matrix[MAX_LED_NUMBER];
volatile char buf[DECOMP_BUFF_SIZE] = {0};
volatile int imgsize = MAX_LED_NUMBER;

void image_size (int size) {
    memset(&matrix[size], 0, (MAX_LED_NUMBER-size)*3);
    imgsize = size;
}

void rx_led_strip_cb(module_t *module, msg_t *msg) {
    static unsigned int next_data_id = 0;
    if (msg->header.cmd == COLOR) {
        // change led target color
        if (msg->header.size == 3) {
            // there is only one color copy it in the entire matrix
            for (int i; i<imgsize; i++) {
                memcpy(&matrix[i], msg->data, sizeof(color_t));
            }
        } else {
            // image management
            luos_receive_data(module, msg, (void*)matrix);
        }
        return;
    }
    if (msg->header.cmd == PARAMETERS) {
        // set the led strip size
        short size;
        memcpy(&size, msg->data, sizeof(short));
        image_size(size);
        return;
    }
}

void convert_color(color_t color, int led_nb) { // It could be GRB
    char remap[3] = {color.g, color.r, color.b};
    for (int y=0; y<3; y++) {
        for(int i=0; i<8; i++){
            if(remap[y] & (1<<(7-i))) {
                buf[(led_nb*24) + ((y*8)+i)] = 38;
            }
            else {
                buf[(led_nb*24) + ((y*8)+i)] = 19;
            }
        }
    }
}


void led_strip_init(void) {
    luos_module_create(rx_led_strip_cb, COLOR_MOD, "led_strip_mod");
    TIM2->CCR1 = 0;
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
    memset(buf, 0, DECOMP_BUFF_SIZE);
    memset(matrix, 0, MAX_LED_NUMBER*3);
    HAL_TIM_PWM_Start_DMA(&htim2,TIM_CHANNEL_1, buf, DECOMP_BUFF_SIZE);

}

void led_strip_loop(void) {
    // Convert matrix into stream data
    for (int i= 0; i<MAX_LED_NUMBER; i++) {
        convert_color(matrix[i], i);
    }
}
