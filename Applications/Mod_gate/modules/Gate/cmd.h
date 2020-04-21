#ifndef CMD_H
#define CMD_H

#include <json_mnger.h>
#include "stdint.h"

extern volatile char detection_ask;

volatile char* get_json_buf(void);
void check_json(uint16_t carac_nbr);
void send_cmds(module_t* module);

#endif /* CMD_H */
