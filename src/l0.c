#include "l0.h"

int l0_msg_handler(vm_t* vm, msg_t* input, msg_t* output) {

	  if (input->header.cmd == L0_LED) {
		  if (input->data[0] < 2) {
			  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, (input->data[0] == 0));
			  return LUOS_PROTOCOL_NB;
		  }
	  }
	  if (input->header.cmd == L0_TEMPERATURE) {
	      output->header.cmd = L0_TEMPERATURE;
	      output->header.target_mode = ID;
	      output->header.size = sizeof(float);
	      output->header.target = input->header.source;
	      float temp = ((110.0f - 30.0f) / ((float)(*TEMP110_CAL_VALUE) - (float)(*TEMP30_CAL_VALUE)) * ((float)L0_analog.temperature_sensor - (float)(*TEMP30_CAL_VALUE)) + 30.0f);
	      memcpy(input->data, &temp, sizeof(float));
	      return L0_TEMPERATURE;
	   }
	  return LUOS_PROTOCOL_NB;
}
