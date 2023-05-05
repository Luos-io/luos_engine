#include "main.h"
#include "robus.h"
#include "context.h"
#include "pub_sub.h"
#include <stdio.h>
#include <default_scenario.h>

extern default_scenario_t default_sc;

void unittest_PubSub_IsTopicSubscribed(void)
{
    NEW_TEST_CASE("Search Topics");
    {
        //  Init default scenario context
        Init_Context();
        Luos_Subscribe(default_sc.App_1.app, 1);
        Luos_Subscribe(default_sc.App_2.app, 18);

        TEST_ASSERT_TRUE(PubSub_IsTopicSubscribed(default_sc.App_1.app, 1));
        TEST_ASSERT_FALSE(PubSub_IsTopicSubscribed(default_sc.App_2.app, 1));
        TEST_ASSERT_FALSE(PubSub_IsTopicSubscribed(default_sc.App_1.app, 18));
        TEST_ASSERT_TRUE(PubSub_IsTopicSubscribed(default_sc.App_2.app, 18));
    }
}

void unittest_Luos_Subscribe(void)
{
    NEW_TEST_CASE("Add random Topics");
    {
        //  Init default scenario context
        Init_Context();

        TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, 1));

        TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, 18));
        TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, 27));
        TEST_ASSERT_EQUAL(3, default_sc.App_1.app->last_topic_position);
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->topic_list[0]);
        TEST_ASSERT_EQUAL(18, default_sc.App_1.app->topic_list[1]);
        TEST_ASSERT_EQUAL(27, default_sc.App_1.app->topic_list[2]);
    }
    NEW_TEST_CASE("Add max topics number");
    {
        //  Init default scenario context
        Init_Context();

        for (uint8_t i = 0; i < LAST_TOPIC; i++)
        {
            TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, i));
            TEST_ASSERT_EQUAL(i + 1, default_sc.App_1.app->last_topic_position);
        }

        TEST_ASSERT_EQUAL(FAILED, Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC));
        TEST_ASSERT_EQUAL(LAST_TOPIC, default_sc.App_1.app->last_topic_position);

        TEST_ASSERT_FALSE(PubSub_IsTopicSubscribed(default_sc.App_1.app, LAST_TOPIC));
    }
}

void unittest_Luos_Unsubscribe(void)
{
    NEW_TEST_CASE("Remove random topics");
    {
        //  Init default scenario context
        Init_Context();

        Luos_Subscribe(default_sc.App_1.app, 2);
        Luos_Subscribe(default_sc.App_1.app, 7);
        Luos_Subscribe(default_sc.App_1.app, 17);
        TEST_ASSERT_EQUAL(3, default_sc.App_1.app->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->topic_list[0]);
        TEST_ASSERT_EQUAL(7, default_sc.App_1.app->topic_list[1]);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->topic_list[2]);

        TEST_ASSERT_EQUAL(SUCCEED, Luos_Unsubscribe(default_sc.App_1.app, 7));
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->topic_list[0]);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->topic_list[1]);

        TEST_ASSERT_EQUAL(FAILED, Luos_Unsubscribe(default_sc.App_1.app, 18));
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->topic_list[0]);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->topic_list[1]);

        TEST_ASSERT_EQUAL(SUCCEED, Luos_Unsubscribe(default_sc.App_1.app, 17));
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->topic_list[0]);

        TEST_ASSERT_EQUAL(SUCCEED, Luos_Unsubscribe(default_sc.App_1.app, 2));
        TEST_ASSERT_EQUAL(0, default_sc.App_1.app->last_topic_position);
    }
    NEW_TEST_CASE("Remove same topic");
    {
        //  Init default scenario context
        Init_Context();

        Luos_Subscribe(default_sc.App_1.app, 2);
        Luos_Subscribe(default_sc.App_1.app, 17);

        TEST_ASSERT_EQUAL(SUCCEED, Luos_Unsubscribe(default_sc.App_1.app, 2));
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->last_topic_position);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->topic_list[0]);

        TEST_ASSERT_EQUAL(FAILED, Luos_Unsubscribe(default_sc.App_1.app, 2));
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->last_topic_position);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->topic_list[0]);
    }

    NEW_TEST_CASE("Last topic position is corrupted");
    {
        //  Init default scenario context
        Init_Context();

        Luos_Subscribe(default_sc.App_1.app, 2);
        default_sc.App_1.app->last_topic_position = LAST_TOPIC - 1;
        TEST_ASSERT_EQUAL(SUCCEED, Luos_Unsubscribe(default_sc.App_1.app, 2));

        Luos_Subscribe(default_sc.App_1.app, 2);
        default_sc.App_1.app->last_topic_position = LAST_TOPIC;
        TEST_ASSERT_EQUAL(FAILED, Luos_Unsubscribe(default_sc.App_1.app, 2));
    }
}
int main(int argc, char **argv)
{
    UNITY_BEGIN();

    UNIT_TEST_RUN(unittest_PubSub_IsTopicSubscribed);
    UNIT_TEST_RUN(unittest_Luos_Subscribe);
    UNIT_TEST_RUN(unittest_Luos_Unsubscribe);

    UNITY_END();
}
