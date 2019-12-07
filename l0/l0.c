#include "l0.h"
#include <string.h>
#include "eeprom.h"
#include "main.h"

uint16_t VirtAddVarTab[NB_OF_VAR] = { 0 };

int l0_msg_handler(module_t* module, msg_t* input, msg_t* output) {
    if (input->header.cmd == NODE_LED) {
        if (input->data[0] < 2) {
            status_led(input->data[0]);
            return LUOS_PROTOCOL_NB;
        }
    }
    if ((input->header.cmd == NODE_TEMPERATURE) & (input->header.size == 0)) {
        output->header.target_mode = ID;
        output->header.target = input->header.source;
        temperature_t temp;
        temp = (((float)L0_analog.temperature_sensor * 300.0f / 330.0f) - (float)(*TEMP30_CAL_VALUE) );
        temp = temp * (110.0f - 30.0f);
        temp = temp /(float)(*TEMP110_CAL_VALUE - *TEMP30_CAL_VALUE);
        temp = temp + 30.0f;
        temperature_to_msg(&temp, output);
        // overlap default TEPERATURE type
        output->header.cmd = NODE_TEMPERATURE;
        return NODE_TEMPERATURE;
    }
    if ((input->header.cmd == NODE_VOLTAGE) & (input->header.size == 0)) {
        output->header.target_mode = ID;
        output->header.target = input->header.source;
        voltage_t volt = (((float)L0_analog.voltage_sensor * 3.3f) / 4096.0f) * VOLTAGEFACTOR;
        voltage_to_msg(&volt, output);
        // overlap default VOLTAGE type
        output->header.cmd = NODE_VOLTAGE;
        return NODE_VOLTAGE;
    }
    return LUOS_PROTOCOL_NB;
}

void status_led(char state) {
    HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, (state == 0));
}

void board_init(void) {
    // Unlock the Flash Program Erase controller
    HAL_FLASH_Unlock();
    // EEPROM Init
    for (uint16_t i = 0; i < NB_OF_VAR; i++) {
        VirtAddVarTab[i] = i;
    }
    EE_Init();
}

// ******** Alias management ****************
void write_alias(unsigned short id, char* alias) {
    const uint16_t addr = id * (MAX_ALIAS_SIZE +1);
    for (uint8_t i=0; i<MAX_ALIAS_SIZE; i++) {
        // here we save an uint8_t on an uint16_t
        EE_WriteVariable(addr + i, (uint16_t)alias[i]);
    }
}

char read_alias(unsigned short id, char* alias) {
     const uint16_t addr = id * (MAX_ALIAS_SIZE +1);
     uint16_t data;
     EE_ReadVariable(addr, &data);
     // Check name integrity
     if (((((char)data < 'A') | ((char)data > 'Z'))
             & (((char)data < 'a') | ((char)data > 'z')))
             | ((char)data == '\0')) {
         return 0;
     } else {
         alias[0] = (char)data;
     }
     for (uint8_t i=1; i<MAX_ALIAS_SIZE; i++) {
        EE_ReadVariable(addr + i, &data);
        alias[i] = (char)data;
     }
     return 1;
}
