#include "main.h"
#include "robus.h"
#include "context.h"
#include "topic.h"
#include <stdio.h>
#include <default_scenario.h>

extern default_scenario_t default_sc;

void unittest_Topic_IsTopicSubscribed(void)
{
    NEW_TEST_CASE("Search Topics");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();
        Robus_TopicSubscribe(default_sc.App_1.app->ll_service, 1);
        Robus_TopicSubscribe(default_sc.App_2.app->ll_service, 18);

        TEST_ASSERT_TRUE(Topic_IsTopicSubscribed(default_sc.App_1.app->ll_service, 1));
        TEST_ASSERT_FALSE(Topic_IsTopicSubscribed(default_sc.App_2.app->ll_service, 1));
        TEST_ASSERT_FALSE(Topic_IsTopicSubscribed(default_sc.App_1.app->ll_service, 18));
        TEST_ASSERT_TRUE(Topic_IsTopicSubscribed(default_sc.App_2.app->ll_service, 18));
    }
}

void unittest_Topic_Subscribe(void)
{
    NEW_TEST_CASE("Add random Topics");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        TEST_ASSERT_EQUAL(SUCCEED, Topic_Subscribe(default_sc.App_1.app->ll_service, 1));

        TEST_ASSERT_EQUAL(SUCCEED, Topic_Subscribe(default_sc.App_1.app->ll_service, 18));
        TEST_ASSERT_EQUAL(SUCCEED, Topic_Subscribe(default_sc.App_1.app->ll_service, 27));
        TEST_ASSERT_EQUAL(3, default_sc.App_1.app->ll_service->last_topic_position);
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->ll_service->topic_list[0]);
        TEST_ASSERT_EQUAL(18, default_sc.App_1.app->ll_service->topic_list[1]);
        TEST_ASSERT_EQUAL(27, default_sc.App_1.app->ll_service->topic_list[2]);
    }
    NEW_TEST_CASE("Add max topics number");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        for (uint8_t i = 0; i < LAST_TOPIC; i++)
        {
            TEST_ASSERT_EQUAL(SUCCEED, Topic_Subscribe(default_sc.App_1.app->ll_service, i));
            TEST_ASSERT_EQUAL(i + 1, default_sc.App_1.app->ll_service->last_topic_position);
        }

        TEST_ASSERT_EQUAL(FAILED, Topic_Subscribe(default_sc.App_1.app->ll_service, LAST_TOPIC));
        TEST_ASSERT_EQUAL(LAST_TOPIC, default_sc.App_1.app->ll_service->last_topic_position);

        TEST_ASSERT_FALSE(Topic_IsTopicSubscribed(default_sc.App_1.app->ll_service, LAST_TOPIC));
    }
}

void unittest_Topic_Unsubscribe(void)
{
    NEW_TEST_CASE("Remove random topics");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        Topic_Subscribe(default_sc.App_1.app->ll_service, 2);
        Topic_Subscribe(default_sc.App_1.app->ll_service, 7);
        Topic_Subscribe(default_sc.App_1.app->ll_service, 17);
        TEST_ASSERT_EQUAL(3, default_sc.App_1.app->ll_service->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->ll_service->topic_list[0]);
        TEST_ASSERT_EQUAL(7, default_sc.App_1.app->ll_service->topic_list[1]);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->ll_service->topic_list[2]);

        TEST_ASSERT_EQUAL(SUCCEED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 7));
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->ll_service->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->ll_service->topic_list[0]);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->ll_service->topic_list[1]);

        TEST_ASSERT_EQUAL(FAILED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 18));
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->ll_service->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->ll_service->topic_list[0]);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->ll_service->topic_list[1]);

        TEST_ASSERT_EQUAL(SUCCEED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 17));
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->ll_service->last_topic_position);
        TEST_ASSERT_EQUAL(2, default_sc.App_1.app->ll_service->topic_list[0]);

        TEST_ASSERT_EQUAL(SUCCEED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 2));
        TEST_ASSERT_EQUAL(0, default_sc.App_1.app->ll_service->last_topic_position);
    }
    NEW_TEST_CASE("Remove same topic");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        Topic_Subscribe(default_sc.App_1.app->ll_service, 2);
        Topic_Subscribe(default_sc.App_1.app->ll_service, 17);

        TEST_ASSERT_EQUAL(SUCCEED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 2));
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->ll_service->last_topic_position);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->ll_service->topic_list[0]);

        TEST_ASSERT_EQUAL(FAILED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 2));
        TEST_ASSERT_EQUAL(1, default_sc.App_1.app->ll_service->last_topic_position);
        TEST_ASSERT_EQUAL(17, default_sc.App_1.app->ll_service->topic_list[0]);
    }

    NEW_TEST_CASE("Last topic position is corrupted");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();

        Topic_Subscribe(default_sc.App_1.app->ll_service, 2);
        default_sc.App_1.app->ll_service->last_topic_position = LAST_TOPIC - 1;
        TEST_ASSERT_EQUAL(SUCCEED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 2));

        Topic_Subscribe(default_sc.App_1.app->ll_service, 2);
        default_sc.App_1.app->ll_service->last_topic_position = LAST_TOPIC;
        TEST_ASSERT_EQUAL(FAILED, Topic_Unsubscribe(default_sc.App_1.app->ll_service, 2));
    }
}
int main(int argc, char **argv)
{
    UNITY_BEGIN();
    ASSERT_ACTIVATION(1);

    UNIT_TEST_RUN(unittest_Topic_IsTopicSubscribed);
    UNIT_TEST_RUN(unittest_Topic_Subscribe);
    UNIT_TEST_RUN(unittest_Topic_Unsubscribe);

    UNITY_END();
}