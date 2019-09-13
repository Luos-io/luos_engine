#include <routingTable.h>
#include <string.h>
#include <sys_msg.h>

route_table_t route_table[MAX_MODULES_NUMBER];
volatile int last_module = 0;

// ********************* route_table search tools ************************
// Return an id from an alias (return 0 if no alias match)
int id_from_alias(char* alias) {
    for(int i = 0; i<=last_module; i++) {
        if (strcmp(route_table[i].alias, alias) == 0) {
            return i;
        }
    }
    return -1;
}

// Create a string from a module type
char* string_from_type(module_type_t type) {
    switch (type) {
        case STATE_MOD:
            return "State";
        break;
        case COLOR_MOD:
            return "Color";
        break;
        case SERVO_MOD:
            return "Servo";
        break;
        case ANGLE_MOD:
            return "Angle";
        break;
        case DISTANCE_MOD:
            return "DistanceSensor";
        break;
        case GATE_MOD:
            return "Gate";
        break;
        case DYNAMIXEL_MOD:
            return "DynamixelMotor";
        break;
        case STEPPER_MOD:
            return "Stepper";
        break;
        case DCMOTOR_MOD:
            return "DCMotor";
        break;
        break;
        case HANDY_MOD:
            return "Handy";
        break;
        case IMU_MOD:
            return "Imu";
        break;
        case LIGHT_MOD:
            return "LightSensor";
        break;
        case CONTROLLED_MOTOR_MOD:
            return "ControlledMotor";
        case VOID_MOD:
            return "Void";
        break;
        case LOAD_MOD:
            return "Load";
        break;
        case VOLTAGE_MOD:
            return "Voltage";
        break;
        default:
            return "unknown";
        break;
    }
}

// check if the module is a sensor or not
uint8_t is_sensor(module_type_t type) {
    if((type == ANGLE_MOD) ||
       (type == STATE_MOD) ||
       (type == DYNAMIXEL_MOD) ||
       (type == DISTANCE_MOD) ||
       (type == IMU_MOD) ||
       (type == LOAD_MOD) ||
       (type == CONTROLLED_MOTOR_MOD) ||
       (type == VOLTAGE_MOD) ||
       (type == LIGHT_MOD)) {
        return 1;
    }
    return 0;
}

// Return a module_type from an id
module_type_t type_from_id(uint16_t id) {
    return route_table[id].type;
}

// Return a module type from alias
module_type_t type_from_alias(char* alias) {
    uint16_t id = id_from_alias(alias);
    return type_from_id(id);
}

// ********************* route_table management tools ************************

// manage module name increment
void add_num_to_alias(char* alias, int num) {
    int intsize = 1;
    if (num > 9) {
        // The string size of num is 2
        intsize = 2;
    }
    if (num > 99) {
        // This is probably a mistake, put an error into the alias
        memset(alias, 0, 15);
        memcpy(alias, "error", strlen("error"));
        return;
    }
    // Change size to fit into 15 characters
    if (strlen(alias) > (15 - intsize)) {
        alias[(15 - intsize)] = '\0';
    } else {
        alias[strlen(alias)] = '\0';
    }
    // Add a number at the end of the alias
    sprintf(alias, "%s%d", alias, num);
}

// Manage introduction messages from modules
route_table_t deser_introduction(msg_t *msg) {
    route_table_t entry;
    const unsigned char type_pos = msg->header.size - 1;
    entry.type = msg->data[type_pos];
    for (int i=0; i<type_pos; i++) {
        entry.alias[i] = msg->data[i];
    }
    return entry;
}

int wait_route_table(module_t* module, msg_t* intro_msg) {
    const int timeout = 15; // timeout in ms
    luos_send(module, intro_msg);
    uint32_t timestamp = HAL_GetTick();
    while ((HAL_GetTick() - timestamp) < timeout) {
        if (route_table[intro_msg->header.target].type != 0) {
            return 1;
        }
    }
    return 0;
}

// Detect all modules and create a route table with it.
// If multiple modules have the same name it will be changed with a number in it automatically.
// At the end this function create a list of sensors id
void detect_modules(module_t* module) {
    msg_t intro_msg, auto_name;
    route_table_t* route_table = get_route_table();
    unsigned char i = 0;
    // clear network detection state and all previous info.
    flush_route_table();

    // now add local module to the route_table
    char hostString[25];
    sprintf(hostString, "%s", module->alias);
    add_on_route_table (1, module->vm->type, hostString);

    // Next, starts the topology detection.
    int nb_mod = topology_detection(module->vm);
    if (nb_mod > MAX_MODULES_NUMBER-1) nb_mod = MAX_MODULES_NUMBER-1;

    // Then, asks for introduction for every found modules.
    intro_msg.header.cmd = IDENTIFY_CMD;
    intro_msg.header.target_mode = IDACK;
    intro_msg.header.size = 0;

    for (int id=2; id<nb_mod+1; id++) {
        intro_msg.header.target = id;
        // Ask to introduce and wait for a reply
        if (wait_route_table(module, &intro_msg)) {
            // We get the answer
            if (id_from_alias(route_table[id].alias) != id ) {
                int annotation = 1;
                // this name already exist in the network change it and send it back.
                // find the new alias to give him
                char aliasbis[15] = {0};
                memcpy(aliasbis, route_table[id].alias, 15);
                add_num_to_alias(route_table[id].alias, annotation++);
                while (id_from_alias(route_table[id].alias) != id ) {
                      memcpy(route_table[id].alias, aliasbis, 15);
                      add_num_to_alias(route_table[id].alias, annotation++);
                }
                auto_name.header.target_mode = ID;
                auto_name.header.target = id;
                auto_name.header.cmd = WRITE_ALIAS;
                auto_name.header.size = strlen(route_table[id].alias);
                // Copy the alias into the data field of the message
                for (i=0; i < auto_name.header.size; i++) {
                    auto_name.data[i] = route_table[id].alias[i];
                }
                auto_name.data[auto_name.header.size] = '\0';
                // Send the message using the WRITE_ALIAS system command
                robus_send_sys(module->vm, &auto_name);
            }
        }
    }
}

// add a new module on the route_table
void add_on_route_table (int id, char type, char* alias) {
    route_table_t entry;
    entry.type = type;
    for (int i=0; i<MAX_ALIAS_SIZE; i++) {
            entry.alias[i] = alias[i];
    }
    insert_on_route_table (id, entry);
}

// add a new module on the route_table
void insert_on_route_table (int id, route_table_t entry) {
    route_table[id] = entry;
    last_module = id;
}

// remove a module from route_table
void remove_on_route_table (int id) {
    route_table[id].alias[0] = '\0';
    route_table[id].type = VOID_MOD;
}

// erase route_table
void flush_route_table() {
    memset(route_table, 0, sizeof(route_table));
    last_module = 0;
}

//return a route_table pointer
route_table_t* get_route_table(void) {
    return route_table;
}

//return the last ID registered into the route_table
volatile int get_last_module(void) {
    return last_module;
}
