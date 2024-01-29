/******************************************************************************
 * @file custom-json
 * @brief Function allowing users to add custom data management to Luos Json conversion
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "tiny-json.h"
#include "luos_engine.h"

void Convert_CustomJsonToMsg(service_t *service, uint16_t target_id, char *property, const json_t *jobj, char *json_str);
void Convert_CustomMsgToJson(msg_t *msg, char *data);
const char *Convert_CustomStringFromType(luos_type_t type);
