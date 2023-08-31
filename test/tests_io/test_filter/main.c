#include "unit_test.h"
#include "filter.h"
#include "../src/service.c"

void unittest_Filter_Topic()
{
    NEW_TEST_CASE("Test the id max Limit");
    {
        Filter_TopicInit();
        TRY
        {
            Filter_AddTopic(MAX_LOCAL_TOPIC_NUMBER - 1);
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        Filter_TopicInit();
        TRY
        {
            Filter_AddTopic(MAX_LOCAL_TOPIC_NUMBER);
            TEST_ASSERT_EQUAL(false, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER));
        }
        TEST_ASSERT_TRUE(IS_ASSERT());

        Filter_TopicInit();
        TRY
        {
            Filter_AddTopic(MAX_LOCAL_TOPIC_NUMBER - 1);
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
            Filter_RmTopic(MAX_LOCAL_TOPIC_NUMBER - 1);
            TEST_ASSERT_EQUAL(false, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        Filter_TopicInit();
        TRY
        {
            Filter_RmTopic(MAX_LOCAL_TOPIC_NUMBER);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test normal topic filtering");
    {
        Filter_TopicInit();
        TRY
        {
            Filter_AddTopic(0);
            TEST_ASSERT_EQUAL(true, Filter_Topic(0));
            TEST_ASSERT_EQUAL(false, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(false, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(false, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddTopic(1);
            TEST_ASSERT_EQUAL(true, Filter_Topic(0));
            TEST_ASSERT_EQUAL(true, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(false, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(false, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddTopic(3);
            TEST_ASSERT_EQUAL(true, Filter_Topic(0));
            TEST_ASSERT_EQUAL(true, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(true, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(false, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddTopic(MAX_LOCAL_TOPIC_NUMBER - 1);
            TEST_ASSERT_EQUAL(true, Filter_Topic(0));
            TEST_ASSERT_EQUAL(true, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(true, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_RmTopic(0);
            TEST_ASSERT_EQUAL(false, Filter_Topic(0));
            TEST_ASSERT_EQUAL(true, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(true, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_RmTopic(0);
            TEST_ASSERT_EQUAL(false, Filter_Topic(0));
            TEST_ASSERT_EQUAL(true, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(true, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_RmTopic(3);
            TEST_ASSERT_EQUAL(false, Filter_Topic(0));
            TEST_ASSERT_EQUAL(true, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(false, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_RmTopic(1);
            TEST_ASSERT_EQUAL(false, Filter_Topic(0));
            TEST_ASSERT_EQUAL(false, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(false, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_RmTopic(1);
            TEST_ASSERT_EQUAL(false, Filter_Topic(0));
            TEST_ASSERT_EQUAL(false, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(false, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(true, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_RmTopic(MAX_LOCAL_TOPIC_NUMBER - 1);
            TEST_ASSERT_EQUAL(false, Filter_Topic(0));
            TEST_ASSERT_EQUAL(false, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(false, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(false, Filter_Topic(MAX_LOCAL_TOPIC_NUMBER - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Filter_Type()
{

    NEW_TEST_CASE("Test the id max Limit");
    {
        TRY
        {
            Phy_FilterType(4096 + 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test the result if there is no service");
    {
        TRY
        {
            service_ctx.number = 0;
            TEST_ASSERT_EQUAL(false, Phy_FilterType(0));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(1));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(2));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Test the result on normal condition");
    {
        TRY
        {
            service_ctx.number       = 1;
            service_ctx.list[0].type = 0;
            TEST_ASSERT_EQUAL(true, Phy_FilterType(0));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(1));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(2));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            service_ctx.number       = 1;
            service_ctx.list[0].type = 1;
            TEST_ASSERT_EQUAL(false, Phy_FilterType(0));
            TEST_ASSERT_EQUAL(true, Phy_FilterType(1));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(2));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            service_ctx.number       = 1;
            service_ctx.list[0].type = 12;
            TEST_ASSERT_EQUAL(false, Phy_FilterType(0));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(1));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(2));
            TEST_ASSERT_EQUAL(true, Phy_FilterType(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            service_ctx.number       = 2;
            service_ctx.list[0].type = 1;
            service_ctx.list[1].type = 12;
            TEST_ASSERT_EQUAL(false, Phy_FilterType(0));
            TEST_ASSERT_EQUAL(true, Phy_FilterType(1));
            TEST_ASSERT_EQUAL(false, Phy_FilterType(2));
            TEST_ASSERT_EQUAL(true, Phy_FilterType(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(unittest_Filter_Topic);
    RUN_TEST(unittest_Filter_Type);

    UNITY_END();
}
