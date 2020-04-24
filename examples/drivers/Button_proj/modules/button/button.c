#include "gpio.h"
#include "button.h"

void rx_btn_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        // fill the message infos
        msg_t pub_msg;
        pub_msg.header.cmd = IO_STATE;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target = msg->header.source;
        pub_msg.header.size = sizeof(char);
        pub_msg.data[0] = HAL_GPIO_ReadPin(BTN_GPIO_Port, BTN_Pin);
        luos_send(module, &pub_msg);
        return;
    }
}

void button_init(void) {
    luos_module_create(rx_btn_cb, STATE_MOD, "button_mod");
}

void button_loop(void) {

}
