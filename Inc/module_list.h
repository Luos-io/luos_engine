#ifndef MODULE_LIST_H
#define MODULE_LIST_H

#include <robus.h>

#define GATE_ID 1

typedef enum {
  GATE,
  SERVO,
  RGB_LED,
  POTENTIOMETER,
  BUTTON,
  DISTANCE,
  RELAY,
  DYNAMIXEL,
  STEPPER,
  DCMOTOR,
  GPIO
} module_type_t;

typedef enum {
  // Common register for all modules and gate
  IDENTIFY_CMD, // Gate asks a module to identify itself
  INTRODUCTION_CMD, // Module sends its alias and type to the gate

  ASK_PUB_CMD, // Gate asks a sensor module to publish its data
  //PUBLISH_CMD, // Module publishes its data in json to the gate. I don't know if we gonna keep that

  // Generic data register
  COLOR, // char[3] (R, G, B)
  ROTATION_POSITION, // float (°)
  ROTATION_SPEED, // float (°/s)
  COMPLIANT, // char (True/False)
  IO_ENABLE, // char (True/False)
  LINEAR_POSITION, // Float (mm)
  LINEAR_SPEED, // Float (mm/s)
  REINIT, // char (True/False)
  STOP, // char (True/False)
  POWER_RATIO, // float (-100.0% <-> 100.0%)
  L0_TEMPERATURE, // float (°c)
  L0_VOLTAGE, // float (V)
  L0_LED, // char (True/False)

  // Specific register
  DXL_WHEELMODE, // char (True/False)
  GPIO_GET_STATE, // gpio_t
  GPIO_SET_STATE, // char[3] (p2, p3, p4)

  LUOS_PROTOCOL_NB,
} module_register_t;


#endif /* MODULE_LIST_H */
