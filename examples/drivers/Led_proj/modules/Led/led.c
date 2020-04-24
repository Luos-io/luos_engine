#include "main.h"
#include "led.h"
#include <math.h>
#include "tim.h"
#include "string.h"
#include "luos.h"

volatile time_luos_t time;
volatile color_t target_rgb;
volatile color_t last_rgb;
volatile float coef[3] = {0.0};
static int elapsed_ms = 0;


void pwm_setvalue(color_t* rgb) {
    TIM3->CCR1 = (uint16_t)rgb->r * 10;
    TIM3->CCR2 = (uint16_t)rgb->g * 10;
    TIM2->CCR1 = (uint16_t)rgb->b * 10;
}

void rx_led_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == COLOR) {
        // change led target color
        memcpy(&last_rgb, &target_rgb, sizeof(color_t));
        color_from_msg(&target_rgb, msg);
        if (time_to_ms(time) > 0.0) {
            elapsed_ms = 0;
            coef[0] = (float)(target_rgb.r - last_rgb.r) / time_to_ms(time);
            coef[1] = (float)(target_rgb.g - last_rgb.g) / time_to_ms(time);
            coef[2] = (float)(target_rgb.b - last_rgb.b) / time_to_ms(time);
        }
        return;
    }
    if (msg->header.cmd == TIME) {
            // save transition time
            time_from_msg(&time, msg);
            return;
        }
}

void HAL_SYSTICK_Led_Callback(void) {
    // ************* motion planning *************
      if (time != 0) {
          static color_t rgb;
          if ((float)elapsed_ms > time_to_ms(time)) {
              // we finished our transition
              pwm_setvalue(&target_rgb);
              memcpy(&last_rgb, &target_rgb, sizeof(color_t));
          }else {
              rgb.r = (int)(coef[0] * (float)elapsed_ms + (float)last_rgb.r);
              rgb.g = (int)(coef[1] * (float)elapsed_ms + (float)last_rgb.g);
              rgb.b = (int)(coef[2] * (float)elapsed_ms + (float)last_rgb.b);
              elapsed_ms++;
              pwm_setvalue(&rgb);
          }
      } else {
          pwm_setvalue(&target_rgb);
      }
}

void led_init(void) {
    time = time_from_ms(0.0);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    luos_module_enable_rt(luos_module_create(rx_led_cb, COLOR_MOD, "rgb_led_mod"));
}

void led_loop(void) {
}
