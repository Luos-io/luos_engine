/******************************************************************************
 * @file luos selftest
 * @brief function to test luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "selftest.h"
#include "context.h"
#include "stdbool.h"
#include "robus_hal.h"
#include "struct_engine.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum
{
    KO = -1,
    OK = 1
} result_t;

volatile uint8_t rx_flag = 0;

static inline void selftest_init(void);
static inline result_t selftest_com(void);
static inline result_t selftest_ptp(void);

/******************************************************************************
 * @brief set rx selftest flag
 * @param None
 * @return None
 ******************************************************************************/
void selftest_SetRxFlag(void)
{
    rx_flag = 1;
}

/******************************************************************************
 * @brief Initialization of the luos library
 * @param None
 * @return None
 ******************************************************************************/
void selftest_init(void)
{
    Luos_Init();
    Robus_Init();
    revision_t revision = {.major = 1, .minor = 0, .build = 0};

    Luos_CreateService(NULL, VOID_TYPE, "Selftest", revision);
}

/******************************************************************************
 * @brief Test of the Tx / Rx lines
 * @param None
 * @return None
 ******************************************************************************/
result_t selftest_com(void)
{
    msg_t msg;
    msg.header.target      = 1;
    msg.header.target_mode = NODEID;
    msg.header.cmd         = REVISION;
    msg.header.size        = 5 * sizeof(uint8_t);
    msg.data[0]            = 0xAA;
    msg.data[1]            = 0x55;
    msg.data[2]            = 0xAA;
    msg.data[3]            = 0x55;
    msg.data[4]            = 0xAA;

    Luos_SendMsg(0, &msg);

    uint32_t tickstart = Luos_GetSystick();
    while (!rx_flag)
    {
        if ((Luos_GetSystick() - tickstart) > 2000)
        {
            return KO;
        }
    }
    while ((Luos_GetSystick() - tickstart) < 2000)
        ;

    if (ctx.tx.lock == true)
    {
        return KO;
    }

    return OK;
}

/******************************************************************************
 * @brief Test of the PTP lines
 * @param None
 * @return None
 ******************************************************************************/
result_t selftest_ptp(void)
{
    uint32_t start_tick = Luos_GetSystick();

    RobusHAL_SetPTPDefaultState(0);
    RobusHAL_SetPTPDefaultState(1);
    RobusHAL_PushPTP(0);

    while (Luos_GetSystick() - start_tick < 2)
        ;
    RobusHAL_SetPTPDefaultState(0);
    // Test  pinout and IRQ
    if (!RobusHAL_GetPTPState(0))
    {
        return KO;
    }

    RobusHAL_SetPTPDefaultState(0);
    RobusHAL_SetPTPDefaultState(1);
    RobusHAL_PushPTP(1);

    while (Luos_GetSystick() - start_tick < 3)
        ;
    RobusHAL_SetPTPDefaultState(1);
    // Test  pinout and IRQ
    if (!RobusHAL_GetPTPState(1))
    {
        return KO;
    }

    return OK;
}

/******************************************************************************
 * @brief Test of the PTP lines
 * @param None
 * @return None
 ******************************************************************************/
void selftest_run(void (*ok_callback)(void), void (*ko_callback)(void))
{
    result_t result = OK;

    selftest_init();

    if (result == OK)
    {
        result = selftest_com();
    }

    if (result == OK)
    {
        result = selftest_ptp();
    }

    switch (result)
    {
        case OK:
            ok_callback();
            break;
        case KO:
            ko_callback();
            break;
        default:
            ko_callback();
    }
}
