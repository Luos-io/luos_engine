#include "main.h"
#include "load.h"
#include "HX711.h"
#include "string.h"

uint8_t new_data_ready = 0;
volatile force_t load = 0.0;
char have_to_tare = 0;

void rx_load_cb(module_t *module, msg_t *msg) {
    if (msg->header.cmd == ASK_PUB_CMD) {
        if (new_data_ready) {
            msg_t pub_msg;
            // fill the message infos
            pub_msg.header.target_mode = ID;
            pub_msg.header.target = msg->header.source;
            force_to_msg(&load, &pub_msg);
            luos_send(module, &pub_msg);
            new_data_ready =  0;
        }
        return;
    }
    if (msg->header.cmd == REINIT) {
        // tare
        have_to_tare = 1;
        return;
    }
    if (msg->header.cmd == RESOLUTION) {
        // put this value in scale
        float value = 0.0;
        memcpy(&value, msg->data, sizeof(value));
        hx711_set_scale(value);
        return;
    }
    if (msg->header.cmd == OFFSET) {
        // offset the load measurement using the scale parameter
        force_t value = 0.0;
        force_from_msg(&value, msg);
        hx711_set_offset((long)(value * hx711_get_scale()));
        return;
    }
}

void load_init(void) {
    hx711_init(128);
    luos_module_create(rx_load_cb, LOAD_MOD, "load_mod");
}

void load_loop(void) {
    if (hx711_is_ready()) {
        load = hx711_get_units(1);
        new_data_ready = 1;
    }
    if (have_to_tare) {
        hx711_tare(10);
        have_to_tare = 0;
    }
}
