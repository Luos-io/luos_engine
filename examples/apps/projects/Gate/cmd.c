#include "cmd.h"
#include "convert.h"

// There is no stack here we use the latest command
volatile char buf[JSON_BUF_NUM][JSON_BUFF_SIZE] = {0};
volatile int current_table = 0;
volatile char cmd_ready = 0;
volatile char detection_ask = 0;

volatile char* get_json_buf(void) {
    return buf[current_table];
}
static void next_json(void){
    current_table++;
    if (current_table >= JSON_BUF_NUM) {
        current_table = 0;
    }
}

void check_json(uint16_t carac_nbr) {
    if ((current_table > (JSON_BUF_NUM-1)) ||
         (current_table<0) ||
         (carac_nbr > (JSON_BUFF_SIZE - 1))) {
        while(1);// Check Json overflow => remove it
    }
    if (buf[current_table][carac_nbr] == '\r') {
        buf[current_table][carac_nbr] = '\0';
        // We have a complete Json here
        cmd_ready++;
        next_json();
    } else {
        // It could be a binary
    }
}

void send_cmds(module_t* module) {
    msg_t msg;

    // check if we have a complete received command
    while (cmd_ready > 0) {
        int concerned_table = current_table-cmd_ready;
        if (concerned_table < 0) {
            concerned_table = JSON_BUF_NUM + concerned_table;
        }
        cJSON *root = cJSON_Parse((char*)buf[concerned_table]);
        // check json integrity
        if (root == NULL) {
            // Error
            cJSON_Delete(root);
            cmd_ready--;
            return;
        }
        // check if this is a detection cmd
        if (cJSON_GetObjectItem(root, "detection") != NULL) {
            detection_ask++;
        }
        if (cJSON_GetObjectItem(root, "baudrate") != NULL) {
            //create a message to setup the new baudrate
            if (cJSON_IsNumber(cJSON_GetObjectItem(root, "baudrate"))) {
                uint32_t baudrate = (float)cJSON_GetObjectItem(root, "baudrate")->valueint;
                luos_set_baudrate(module, baudrate);
            }
        }
        if (cJSON_GetObjectItem(root, "benchmark") != NULL) {
            //manage benchmark
            if (cJSON_IsObject(cJSON_GetObjectItem(root, "benchmark"))) {
                // Get all parameters
                volatile cJSON *parameters = cJSON_GetObjectItem(root, "benchmark");
                uint32_t repetition = 0;
                if (cJSON_IsNumber(cJSON_GetObjectItem(parameters, "repetitions"))) {
                    repetition = (int)cJSON_GetObjectItem(parameters, "repetitions")->valueint;
                }
                uint32_t target_id = (int)cJSON_GetObjectItem(parameters, "target")->valueint;
                cJSON *item = cJSON_GetObjectItem(parameters, "data");
                uint32_t size = (int)cJSON_GetArrayItem(item, 0)->valueint;
                if (size > 0){
                    // find the first \r of the current buf
                    char* bin_data = (char*)buf[concerned_table];
                    int index = 0;
                    for (index = 0; index < JSON_BUFF_SIZE; index++) {
                        if (bin_data[index] == '\r') {
                            index++;
                            break;
                        }
                    }
                    if (index < JSON_BUFF_SIZE-1) {
                        // create a message from parameters
                        msg.header.cmd = REVISION;
                        msg.header.target_mode = IDACK;
                        msg.header.target = target_id;
                        // save current time
                        uint32_t begin_systick = HAL_GetTick();
                        uint32_t failed_msg_nb = 0;
                        // send this message multiple time
                        int i = 0;
                        for (i = 0; i < repetition; i++) {
                            failed_msg_nb += luos_send_data(module, &msg, &bin_data[index], (unsigned int)size);
                        }
                        uint32_t end_systick = HAL_GetTick();
                        float data_rate = (float)size * (float)(repetition - failed_msg_nb) / (((float)end_systick - (float)begin_systick) / 1000.0) * 8;
                        float fail_rate = (float)failed_msg_nb*100.0/(float)repetition;
                        #ifdef USE_SERIAL
                            char json[60] = {0};
                            sprintf(json, "{\"benchmark\":{\"data_rate\":%.2f,\"fail_rate\":%.2f}}\n", data_rate, fail_rate);
                            serial_write(json, strlen(json));
                        #else
                            printf("{\"benchmark\":{\"data_rate\":%.2f,\"fail_rate\":%.2f}}\n", data_rate, fail_rate);
                        #endif
                    }
                }
            }
        }
        cJSON *modules = cJSON_GetObjectItem(root, "modules");
        // Get modules
        if (cJSON_IsObject(modules)) {
            // Loop into modules
            cJSON *module_jsn = modules->child;
            while (module_jsn != NULL) {
                // Create msg
                char *alias = module_jsn->string;
                uint16_t id = id_from_alias(alias);
                if(id == 65535) {
                    // If alias doesn't exist in our list id_from_alias send us back -1 = 65535
                    // So here there is an error in alias.
                    cJSON_Delete(root);
                    cmd_ready--;
                    return;
                }
                module_type_t type =  type_from_id(id);
                json_to_msg(module, id, type, module_jsn, &msg, (char*)buf[concerned_table]);
                // Get next module
                module_jsn = module_jsn->next;
            }
        }
        cJSON_Delete(root);
        cmd_ready--;
    }
}
