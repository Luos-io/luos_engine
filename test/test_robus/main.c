#include <stdio.h>
#include "main.h"
#include "robus.h"
#include "context.h"
#include "unit_test.h"
#include <default_scenario.h>

extern default_scenario_t default_sc;

void unittest_Robus_IDMaskCalculation()
{
    NEW_TEST_CASE("ID shift mask test");
    {
        Robus_MaskInit();

        Robus_IDMaskCalculation(7, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(0, ctx.IDShiftMask);

        Robus_IDMaskCalculation(8, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(0, ctx.IDShiftMask);

        Robus_IDMaskCalculation(9, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(1, ctx.IDShiftMask);
    }

    NEW_TEST_CASE("ID mask test");
    {
        Robus_MaskInit();
        Robus_IDMaskCalculation(7, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(0xC0, ctx.IDMask[0]);
        TEST_ASSERT_EQUAL(0x07, ctx.IDMask[1]);

        Robus_MaskInit();
        Robus_IDMaskCalculation(8, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(0, ctx.IDShiftMask);
        TEST_ASSERT_EQUAL(0x80, ctx.IDMask[0]);
        TEST_ASSERT_EQUAL(0x0F, ctx.IDMask[1]);

        Robus_MaskInit();
        Robus_IDMaskCalculation(9, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(1, ctx.IDShiftMask);
        TEST_ASSERT_EQUAL(0x1F, ctx.IDMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.IDMask[1]);
    }

    NEW_TEST_CASE("Limit 4096");
    {
        Robus_MaskInit();
        Robus_IDMaskCalculation(4096 - MAX_SERVICE_NUMBER, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(508, ctx.IDShiftMask);
    }

    NEW_TEST_CASE("Limit 0");
    {
        RESET_ASSERT();
        Robus_MaskInit();
        TRY
        {
            Robus_IDMaskCalculation(0, SERVICE_NUMBER);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        RESET_ASSERT();
    }
}

void unittest_Luos_Subscribe(void)
{
    NEW_TEST_CASE("Normal Add to node topic list");
    {
        //  Init default scenario context
        Init_Context();

        Luos_Subscribe(default_sc.App_1.app, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        Luos_Subscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
        Luos_Subscribe(default_sc.App_1.app, 6);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        Luos_Subscribe(default_sc.App_1.app, 18);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
    }
    NEW_TEST_CASE("Assert when adding last topic");
    {
        //  Init default scenario context
        Init_Context();

        RESET_ASSERT();

        Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC);
        TEST_ASSERT_FALSE(IS_ASSERT());

        TRY
        {
            Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC + 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
    NEW_TEST_CASE("Add same topic multiple times");
    {
        //  Init default scenario context
        Init_Context();

        Luos_Subscribe(default_sc.App_1.app, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        Luos_Subscribe(default_sc.App_2.app, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        Luos_Subscribe(default_sc.App_3.app, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);

        Luos_Subscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
        Luos_Subscribe(default_sc.App_2.app, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
        Luos_Subscribe(default_sc.App_3.app, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
    }
}

void unittest_Robus_TopicUnsubscribe(void)
{
    NEW_TEST_CASE("Remove from an empty list");
    {
        //  Init default scenario context
        Init_Context();

        error_return_t err = Luos_Unsubscribe(default_sc.App_1.app, 3);
        TEST_ASSERT_EQUAL(FAILED, err);
    }
    NEW_TEST_CASE("Normal Remove from topic list");
    {
        //  Init default scenario context
        Init_Context();

        Luos_Subscribe(default_sc.App_1.app, 0);
        Luos_Subscribe(default_sc.App_1.app, 4);
        Luos_Subscribe(default_sc.App_1.app, 6);
        Luos_Subscribe(default_sc.App_1.app, 18);

        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(0x41, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_1.app, 0);
        TEST_ASSERT_EQUAL(0x40, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_1.app, 6);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_1.app, 18);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[2]);
    }
    NEW_TEST_CASE("Demand to remove last topic");
    {
        //  Init default scenario context
        Init_Context();
        Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC - 1);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x08, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_1.app, LAST_TOPIC - 1);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[2]);
        error_return_t err = Luos_Unsubscribe(default_sc.App_1.app, LAST_TOPIC);
        TEST_ASSERT_EQUAL(err, FAILED);
    }
    NEW_TEST_CASE("Remove multiple times same topic");
    {
        //  Init default scenario context
        Init_Context();
        Luos_Subscribe(default_sc.App_1.app, 0);
        Luos_Subscribe(default_sc.App_1.app, 4);
        Luos_Subscribe(default_sc.App_2.app, 4);
        Luos_Subscribe(default_sc.App_3.app, 4);
        Luos_Subscribe(default_sc.App_2.app, 6);
        Luos_Subscribe(default_sc.App_3.app, 6);
        Luos_Subscribe(default_sc.App_1.app, 18);
        Luos_Subscribe(default_sc.App_2.app, 18);

        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_2.app, 4);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_3.app, 4);
        TEST_ASSERT_EQUAL(0x41, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_2.app, 6);
        TEST_ASSERT_EQUAL(0x41, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_3.app, 6);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_1.app, 18);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Luos_Unsubscribe(default_sc.App_2.app, 18);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[2]);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Big data reception
    UNIT_TEST_RUN(unittest_Robus_IDMaskCalculation);
    UNIT_TEST_RUN(unittest_Luos_Subscribe);
    UNIT_TEST_RUN(unittest_Robus_TopicUnsubscribe);

    UNITY_END();
}
