#include <stdio.h>
#include "main.h"
#include "robus.h"
#include "context.h"
#include "unit_test.h"
#include "default_scenario.h"
#include "filter.h"

extern default_scenario_t default_sc;

void unittest_Luos_Subscribe(void)
{
    NEW_TEST_CASE("Normal Add to node topic list");
    {
        //  Init default scenario context
        Init_Context();

        Luos_Subscribe(default_sc.App_1.app, 0);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(false, Filter_Topic(6));
        TEST_ASSERT_EQUAL(false, Filter_Topic(18));
        Luos_Subscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        TEST_ASSERT_EQUAL(false, Filter_Topic(6));
        TEST_ASSERT_EQUAL(false, Filter_Topic(18));
        Luos_Subscribe(default_sc.App_1.app, 6);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(false, Filter_Topic(18));
        Luos_Subscribe(default_sc.App_1.app, 18);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));
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
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        Luos_Subscribe(default_sc.App_2.app, 0);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        Luos_Subscribe(default_sc.App_3.app, 0);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));

        Luos_Subscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        Luos_Subscribe(default_sc.App_2.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        Luos_Subscribe(default_sc.App_3.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
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
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_1.app, 0);
        TEST_ASSERT_EQUAL(false, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_1.app, 6);
        TEST_ASSERT_EQUAL(false, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(false, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_1.app, 18);
        TEST_ASSERT_EQUAL(false, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(false, Filter_Topic(6));
        TEST_ASSERT_EQUAL(false, Filter_Topic(18));
    }
    NEW_TEST_CASE("Demand to remove last topic");
    {
        //  Init default scenario context
        Init_Context();
        Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC - 1);
        TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC - 1));
        Luos_Unsubscribe(default_sc.App_1.app, LAST_TOPIC - 1);
        TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC - 1));
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

        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_1.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_2.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(true, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_3.app, 4);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_2.app, 6);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(true, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_3.app, 6);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(false, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_1.app, 18);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(false, Filter_Topic(6));
        TEST_ASSERT_EQUAL(true, Filter_Topic(18));

        Luos_Unsubscribe(default_sc.App_2.app, 18);
        TEST_ASSERT_EQUAL(true, Filter_Topic(0));
        TEST_ASSERT_EQUAL(false, Filter_Topic(4));
        TEST_ASSERT_EQUAL(false, Filter_Topic(6));
        TEST_ASSERT_EQUAL(false, Filter_Topic(18));
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    UNIT_TEST_RUN(unittest_Luos_Subscribe);
    UNIT_TEST_RUN(unittest_Robus_TopicUnsubscribe);

    UNITY_END();
}
