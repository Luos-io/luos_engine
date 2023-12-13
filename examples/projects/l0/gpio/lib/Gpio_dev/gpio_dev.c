/******************************************************************************
 * @file gpio_dev
 * @brief driver example a simple gpio_dev
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "gpio_dev.h"
#include "ll_gpio.h"

#include "profile_state.h"
#include "profile_voltage.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
profile_state_t gpio[GPIO_NB];
profile_voltage_t analog[ANALOG_NB];

/*******************************************************************************
 * Function
 ******************************************************************************/
static void GpioDev_callback(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void GpioDev_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    // hardware initialization
    ll_digital_init();
    ll_analog_init();

    // services creation
    // Profile configuration
    analog[P1].access = READ_ONLY_ACCESS;
    analog[P7].access = READ_ONLY_ACCESS;
    analog[P8].access = READ_ONLY_ACCESS;
    analog[P9].access = READ_ONLY_ACCESS;
    // Service creation following profile
    ProfileVoltage_CreateService(&analog[P1], 0, "analog_read_P1", revision);
    ProfileVoltage_CreateService(&analog[P7], 0, "analog_read_P7", revision);
    ProfileVoltage_CreateService(&analog[P8], 0, "analog_read_P8", revision);
    ProfileVoltage_CreateService(&analog[P9], 0, "analog_read_P9", revision);

    // Input profile configuration
    gpio[P5].access = READ_ONLY_ACCESS;
    gpio[P6].access = READ_ONLY_ACCESS;
    // Service creation following profile
    ProfileState_CreateService(&gpio[P5], 0, "digit_read_P5", revision);
    ProfileState_CreateService(&gpio[P6], 0, "digit_read_P6", revision);

    // Output profile configuration
    gpio[P2].access = WRITE_ONLY_ACCESS;
    gpio[P3].access = WRITE_ONLY_ACCESS;
    gpio[P4].access = WRITE_ONLY_ACCESS;
    // Service creation following profile, for this one we use a target event using callback
    ProfileState_CreateService(&gpio[P2], GpioDev_callback, "digit_write_P2", revision);
    ProfileState_CreateService(&gpio[P3], GpioDev_callback, "digit_write_P3", revision);
    ProfileState_CreateService(&gpio[P4], GpioDev_callback, "digit_write_P4", revision);
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void GpioDev_Loop(void)
{
    // read values from GPIO
    ll_digital_read(gpio);
    // read values from ADC
    ll_analog_read(analog);
}

/******************************************************************************
 * @brief routine called by luos
 * @param None
 * @return None
 ******************************************************************************/
static void GpioDev_callback(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        // write values on digital port
        ll_digital_write(gpio);
    }
}
