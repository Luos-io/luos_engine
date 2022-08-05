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
        Robus_IDMaskCalculation(4096 - SERVICE_NUMBER, SERVICE_NUMBER);
        TEST_ASSERT_EQUAL(511, ctx.IDShiftMask);
    }

    NEW_TEST_CASE("Limit 0");
    {
        Robus_MaskInit();
        Robus_IDMaskCalculation(0, SERVICE_NUMBER);
        TEST_ASSERT_TRUE(IS_ASSERT());
        RESET_ASSERT();
    }
}

void unittest_Robus_TopicSubscribe(void)
{
    NEW_TEST_CASE("Normal Add to node topic list");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 6);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 18);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
    }
    NEW_TEST_CASE("Assert when adding last topic");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        RESET_ASSERT();

        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, LAST_TOPIC);
        TEST_ASSERT_FALSE(IS_ASSERT());

        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, LAST_TOPIC + 1);
        TEST_ASSERT_TRUE(IS_ASSERT());

        RESET_ASSERT();
    }
    NEW_TEST_CASE("Add same topic multiple times");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        Robus_TopicSubscribe(default_sc.App_2.app->ll_service, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        Robus_TopicSubscribe(default_sc.App_3.app->ll_service, 0);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);

        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
        Robus_TopicSubscribe(default_sc.App_2.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
        Robus_TopicSubscribe(default_sc.App_3.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x11, ctx.TopicMask[0]);
    }
}

void unittest_Robus_TopicUnsubscribe(void)
{
    NEW_TEST_CASE("Remove from an empty list");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        error_return_t err = Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, 3);
        TEST_ASSERT_EQUAL(FAILED, err);
    }
    NEW_TEST_CASE("Normal Remove from topic list");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 0);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 4);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 6);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 18);

        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x41, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, 0);
        TEST_ASSERT_EQUAL(0x40, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, 6);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, 18);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[2]);
    }
    NEW_TEST_CASE("Demand to remove last topic");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, LAST_TOPIC - 1);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x08, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, LAST_TOPIC - 1);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[2]);
        error_return_t err = Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, LAST_TOPIC);
        TEST_ASSERT_EQUAL(err, FAILED);
    }
    NEW_TEST_CASE("Remove multiple times same topic");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 0);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 4);
        Robus_TopicSubscribe(default_sc.App_2.app->ll_service, 4);
        Robus_TopicSubscribe(default_sc.App_3.app->ll_service, 4);
        Robus_TopicSubscribe(default_sc.App_2.app->ll_service, 6);
        Robus_TopicSubscribe(default_sc.App_3.app->ll_service, 6);
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 18);
        Robus_TopicSubscribe(default_sc.App_2.app->ll_service, 18);

        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_2.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x51, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_3.app->ll_service, 4);
        TEST_ASSERT_EQUAL(0x41, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_2.app->ll_service, 6);
        TEST_ASSERT_EQUAL(0x41, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_3.app->ll_service, 6);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_1.app->ll_service, 18);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x04, ctx.TopicMask[2]);
        Robus_TopicUnsubscribe(default_sc.App_2.app->ll_service, 18);
        TEST_ASSERT_EQUAL(0x01, ctx.TopicMask[0]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[1]);
        TEST_ASSERT_EQUAL(0x00, ctx.TopicMask[2]);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    ASSERT_ACTIVATION(1);

    // Big data reception
    UNIT_TEST_RUN(unittest_Robus_IDMaskCalculation);
    UNIT_TEST_RUN(unittest_Robus_TopicSubscribe);
    UNIT_TEST_RUN(unittest_Robus_TopicUnsubscribe);

    UNITY_END();
}
