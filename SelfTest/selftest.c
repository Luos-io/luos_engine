/******************************************************************************
 * @file luos selftest
 * @brief function to test luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "selftest.h"
#include "context.h"
#include "stdbool.h"
#include "luos_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum
{
    KO = -1,
    OK = 1
} result_t;

volatile uint8_t rx_flag  = 0;
volatile uint8_t ptp_flag = 0;

static inline void selftest_init(void);
static inline result_t selftest_com(void);
static inline result_t selftest_ptp(void);

/******************************************************************************
 * @brief set ptp selftest flag
 * @param None
 * @return None
 ******************************************************************************/
void selftest_SetPtpFlag(void)
{
    ptp_flag++;
}

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

    uint32_t tickstart = LuosHAL_GetSystick();
    while (!rx_flag)
    {
        if ((LuosHAL_GetSystick() - tickstart) > 2000)
        {
            return KO;
        }
    }
    while ((LuosHAL_GetSystick() - tickstart) < 2000)
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
    LuosHAL_PushPTP(0);
    uint32_t start_tick = LuosHAL_GetSystick();
    while (LuosHAL_GetSystick() - start_tick < 2);
    // release the ptp line
    if (!LuosHAL_GetPTPState(1))
    {
    	return KO;
    }
    LuosHAL_SetPTPDefaultState(0);
    LuosHAL_PushPTP(1);
    while (LuosHAL_GetSystick() - start_tick < 3);
    if (!LuosHAL_GetPTPState(0))
    {
    	return KO;
    }

    LuosHAL_SetPTPDefaultState(0);
    LuosHAL_SetPTPDefaultState(1);

    if (ptp_flag<2)
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
