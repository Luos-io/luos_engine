#include "context.h"
#include "pub_sub.h"
#include <stdio.h>
#include "unit_test.h"
#include <default_scenario.h>
#include "filter.h"

extern default_scenario_t default_sc;

void unittest_PubSub_IsTopicSubscribed(void)
{
    NEW_TEST_CASE("Test IsTopicSubscribed assert conditions");
    {
        TRY
        {
            PubSub_IsTopicSubscribed(NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            //  Init default scenario context
            Init_Context();
            PubSub_IsTopicSubscribed(default_sc.App_1.app, LAST_TOPIC);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Search Topics");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Luos_Subscribe(void)
{

    NEW_TEST_CASE("Test Subscribe assert conditions");
    {
        TRY
        {
            Luos_Subscribe(NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            //  Init default scenario context
            Init_Context();
            Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Add random Topics");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, 1));
            TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, 2));
            TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC - 1));
            TEST_ASSERT_EQUAL(3, default_sc.App_1.app->last_topic_position);
            TEST_ASSERT_EQUAL(1, default_sc.App_1.app->topic_list[0]);
            TEST_ASSERT_EQUAL(2, default_sc.App_1.app->topic_list[1]);
            TEST_ASSERT_EQUAL(LAST_TOPIC - 1, default_sc.App_1.app->topic_list[2]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Add max topics number");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            for (uint8_t i = 0; i < LAST_TOPIC; i++)
            {
                TEST_ASSERT_EQUAL(SUCCEED, Luos_Subscribe(default_sc.App_1.app, i));
                TEST_ASSERT_EQUAL(i + 1, default_sc.App_1.app->last_topic_position);
            }

            TEST_ASSERT_EQUAL(LAST_TOPIC, default_sc.App_1.app->last_topic_position);

            TEST_ASSERT_TRUE(PubSub_IsTopicSubscribed(default_sc.App_1.app, LAST_TOPIC - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Normal Add to node topic list");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Assert when adding last topic");
    {
        //  Init default scenario context
        Init_Context();

        RESET_ASSERT();

        Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC - 1);
        TEST_ASSERT_FALSE(IS_ASSERT());

        TRY
        {
            Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }

    NEW_TEST_CASE("Add same topic multiple times");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_Unsubscribe(void)
{
    NEW_TEST_CASE("Remove from an empty list");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            error_return_t err = Luos_Unsubscribe(default_sc.App_1.app, 3);
            TEST_ASSERT_EQUAL(FAILED, err);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Normal Remove from topic list");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Demand to remove last topic");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            Luos_Subscribe(default_sc.App_1.app, LAST_TOPIC - 1);
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC - 1));
            Luos_Unsubscribe(default_sc.App_1.app, LAST_TOPIC - 1);
            TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC - 1));
            error_return_t err = Luos_Unsubscribe(default_sc.App_1.app, LAST_TOPIC - 1);
            TEST_ASSERT_EQUAL(err, FAILED);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Remove multiple times same topic");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Remove random topics");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Remove same topic");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Last topic position is already removed");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            Luos_Subscribe(default_sc.App_1.app, 2);
            default_sc.App_1.app->last_topic_position = LAST_TOPIC;
            TEST_ASSERT_EQUAL(SUCCEED, Luos_Unsubscribe(default_sc.App_1.app, 2));
            TEST_ASSERT_EQUAL(FAILED, Luos_Unsubscribe(default_sc.App_1.app, 2));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
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
