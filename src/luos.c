#include "luos.h"

int luos_msg_handler(vm_t* vm, msg_t* input, msg_t* output) {
	  if (input->header.cmd == IDENTIFY_CMD) {
		  output->header.cmd = INTRODUCTION_CMD;
		  output->header.target_mode = ID;
		  output->header.size = MAX_ALIAS_SIZE+1;
		  output->header.target = input->header.source;
	      for (int i=0; i<MAX_ALIAS_SIZE; i++) {
	    	  output->data[i] = vm->alias[i];
	      }
	      output->data[MAX_ALIAS_SIZE] = vm->type;
	      return IDENTIFY_CMD;
	  }
	  return LUOS_PROTOCOL_NB;
}
