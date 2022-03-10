#include "cmd.h"
#include "convert.h"
#include <stdio.h>
#include "gate.h"
#include "bootloader_ex.h"

// There is no stack here we use the latest command
volatile char buf[JSON_BUF_NUM][JSON_BUFF_SIZE] = {0};
volatile int current_table = 0;
volatile char cmd_ready = 0;
volatile char detection_ask = 0;

char *get_json_buf(void)
{
    return (char *)buf[current_table];
}
static void next_json(void)
{
    current_table++;
    if (current_table >= JSON_BUF_NUM)
    {
        current_table = 0;
    }
}

void check_json(uint16_t carac_nbr)
{
    if ((current_table > (JSON_BUF_NUM - 1)) ||
        (current_table < 0) ||
        (carac_nbr > (JSON_BUFF_SIZE - 1)))
    {
        while (1)
            ; // Check Json overflow => remove it
    }
    if (buf[current_table][carac_nbr] == '\r')
    {
        buf[current_table][carac_nbr] = '\0';
        // We have a complete Json here
        cmd_ready++;
        next_json();
    }
    else
    {
        // It could be a binary
    }
}

void send_cmds(container_t *container)
{
    msg_t msg;

    // check if we have a complete received command
    while (cmd_ready > 0)
    {
        int concerned_table = current_table - cmd_ready;
        if (concerned_table < 0)
        {
            concerned_table = JSON_BUF_NUM + concerned_table;
        }
        cJSON *root = cJSON_Parse((char *)buf[concerned_table]);
        // check json integrity
        if (root == NULL)
        {
            // Error
            cJSON_Delete(root);
            cmd_ready--;
            return;
        }
        // check if this is a detection cmd
        if (cJSON_GetObjectItem(root, "detection") != NULL)
        {
            detection_ask++;
        }
        if (cJSON_GetObjectItem(root, "baudrate") != NULL)
        {
            //create a message to setup the new baudrate
            if (cJSON_IsNumber(cJSON_GetObjectItem(root, "baudrate")))
            {
                uint32_t baudrate = (float)cJSON_GetObjectItem(root, "baudrate")->valueint;
                Luos_SendBaudrate(container, baudrate);
            }
        }
        if (cJSON_GetObjectItem(root, "benchmark") != NULL)
        {
            //manage benchmark
            if (cJSON_IsObject(cJSON_GetObjectItem(root, "benchmark")))
            {
                // Get all parameters
                cJSON *parameters = cJSON_GetObjectItem(root, "benchmark");
                uint32_t repetition = 0;
                if (cJSON_IsNumber(cJSON_GetObjectItem(parameters, "repetitions")))
                {
                    repetition = (int)cJSON_GetObjectItem(parameters, "repetitions")->valueint;
                }
                uint32_t target_id = (int)cJSON_GetObjectItem(parameters, "target")->valueint;
                cJSON *item = cJSON_GetObjectItem(parameters, "data");
                uint32_t size = (int)cJSON_GetArrayItem(item, 0)->valueint;
                if (size > 0)
                {
                    // find the first \r of the current buf
                    char *bin_data = (char *)buf[concerned_table];
                    int index = 0;
                    for (index = 0; index < JSON_BUFF_SIZE; index++)
                    {
                        if (bin_data[index] == '\r')
                        {
                            index++;
                            break;
                        }
                    }
                    if (index < JSON_BUFF_SIZE - 1)
                    {
                        // create a message from parameters
                        msg.header.cmd = REVISION;
                        msg.header.target_mode = IDACK;
                        msg.header.target = target_id;
                        // save current time
                        uint32_t begin_systick = Luos_GetSystick();
                        uint32_t failed_msg_nb = 0;
                        // Before trying to send anything make sure to finish any transmission
                        while (Luos_TxComplete() == FAILED)
                            ;
                        // Wait 10ms allowing receiving messages to finish
                        uint32_t tickstart = Luos_GetSystick();
                        while ((Luos_GetSystick() - tickstart) < 10)
                            ;
                        // Flush every messages pending
                        Luos_Flush();
                        // To get the number of message failed we will use statistics
                        // We have to reinit the number of dropped message before start
                        uint8_t drop_back = container->node_statistics->memory.msg_drop_number;
                        container->node_statistics->memory.msg_drop_number = 0;
                        uint8_t retry_back = *container->ll_container->ll_stat.max_retry;
                        *container->ll_container->ll_stat.max_retry = 0;
                        // send this message multiple time
                        int i = 0;
                        for (i = 0; i < repetition; i++)
                        {
                            Luos_SendData(container, &msg, &bin_data[index], (unsigned int)size);
                        }
                        // Wait transmission end
                        while (Luos_TxComplete() == FAILED)
                            ;
                        // Get the number of failures on transmission
                        failed_msg_nb = container->node_statistics->memory.msg_drop_number;
                        // Get the number of retry
                        // If retry == max retry number consider all messages as lost
                        if (*container->ll_container->ll_stat.max_retry >= NBR_RETRY)
                        {
                            // We failed to transmit this message count all as failed
                            failed_msg_nb = repetition;
                        }
                        container->node_statistics->memory.msg_drop_number = drop_back;
                        *container->ll_container->ll_stat.max_retry = retry_back;
                        uint32_t end_systick = Luos_GetSystick();
                        float data_rate = (float)size * (float)(repetition - failed_msg_nb) / (((float)end_systick - (float)begin_systick) / 1000.0) * 8;
                        float fail_rate = (float)failed_msg_nb * 100.0 / (float)repetition;
                        char json[60] = {0};
                        sprintf(json, "{\"benchmark\":{\"data_rate\":%.2f,\"fail_rate\":%.2f}}\n", data_rate, fail_rate);
                        json_send(json);
                    }
                }
            }
        }
        // bootloader commands
        cJSON *bootloader_json = cJSON_GetObjectItem(root, "bootloader");
        if (cJSON_IsObject(bootloader_json))
        {
            LuosBootloader_GateCmd(container, (char *)buf[concerned_table], bootloader_json);
        }

        cJSON *containers = cJSON_GetObjectItem(root, "containers");
        // Get containers
        if (cJSON_IsObject(containers))
        {
            // Loop into containers
            cJSON *container_jsn = containers->child;
            while (container_jsn != NULL)
            {
                // Create msg
                char *alias = container_jsn->string;
                uint16_t id = RoutingTB_IDFromAlias(alias);
                if (id == 65535)
                {
                    // If alias doesn't exist in our list id_from_alias send us back -1 = 65535
                    // So here there is an error in alias.
                    cJSON_Delete(root);
                    cmd_ready--;
                    return;
                }
                luos_type_t type = RoutingTB_TypeFromID(id);
                json_to_msg(container, id, type, container_jsn, &msg, (char *)buf[concerned_table]);
                // Get next container
                container_jsn = container_jsn->next;
            }
        }
        cJSON_Delete(root);
        cmd_ready--;
    }
}
