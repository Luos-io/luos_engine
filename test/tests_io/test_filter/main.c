#include "unit_test.h"
#include "filter.h"
#include "../src/service.c"

void unittest_Filter_ServiceId()
{
    uint16_t service_number = 10;
    NEW_TEST_CASE("Basic Service ID mask test");
    {
        Phy_FiltersInit();
        TRY
        {
             Phy_AddLocalServices(7, service_number);
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(8, service_number);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(7));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(8 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(9, service_number);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(9));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(9 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(9, 0);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    service_number = MAX_LOCAL_SERVICE_NUMBER;
    NEW_TEST_CASE("Service ID mask test with max services");
    {
        Phy_FiltersInit();

        TRY
        {
             Phy_AddLocalServices(7, service_number);
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(8, service_number);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(7));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(8 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(9, service_number);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(9));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(9 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(9, 0);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Overlap the MAX_LOCAL_SERVICE_NUMBER");
    {
        Phy_FiltersInit();
        TRY
        {
             Phy_AddLocalServices(1, MAX_LOCAL_SERVICE_NUMBER + 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test the id 4096 Limit");
    {
        Phy_FiltersInit();
        TRY
        {
             Phy_AddLocalServices(4096 - MAX_LOCAL_SERVICE_NUMBER, service_number);
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(4096 - MAX_LOCAL_SERVICE_NUMBER));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(4096 - MAX_LOCAL_SERVICE_NUMBER + 1, service_number);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());

        TRY
        {
            Filter_ServiceID(4096 + 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test the id 0 Limit");
    {
        Phy_FiltersInit();
        TRY
        {
             Phy_AddLocalServices(0, service_number);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
}

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

void unittest_Phy_ComputeTargets()
{
    header_t header;
    NEW_TEST_CASE("Test a wrong header target_mode");
    {
        TRY
        {
            header.target_mode = 0x0f;
            Phy_ComputeTargets(&header);
        }
        TEST_ASSERT_TRUE(!IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test service ID");
    {
        TRY
        {
             Phy_AddLocalServices(1, 1);
            header.target_mode = SERVICEIDACK;
            header.target      = 1;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(1, 1);
            header.target_mode = SERVICEIDACK;
            header.target      = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(1, 1);
            header.target_mode = SERVICEID;
            header.target      = 1;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(1, 1);
            header.target_mode = SERVICEID;
            header.target      = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(1, 2);
            header.target_mode = SERVICEIDACK;
            header.target      = 2;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(1, 2);
            header.target_mode = SERVICEIDACK;
            header.target      = 3;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(1, 2);
            header.target_mode = SERVICEID;
            header.target      = 2;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
             Phy_AddLocalServices(1, 2);
            header.target_mode = SERVICEID;
            header.target      = 3;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Test Type");
    {
        TRY
        {
            service_ctx.number       = 1;
            service_ctx.list[0].type = 0;
            header.target_mode       = TYPE;
            header.target            = 0;
            // We should have Luos and Robus
            TEST_ASSERT_EQUAL(0x03, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            service_ctx.number       = 1;
            service_ctx.list[0].type = 0;
            header.target_mode       = TYPE;
            header.target            = 1;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Test Broadcast");
    {
        TRY
        {
            header.target_mode = BROADCAST;
            header.target      = BROADCAST_VAL;
            // We should have Luos and Robus
            TEST_ASSERT_EQUAL(0x03, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            header.target_mode = BROADCAST;
            header.target      = 1;
            // We should have Luos and Robus
            TEST_ASSERT_EQUAL(0x03, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Test Node ID");
    {
        TRY
        {
            Node_Get()->node_id = 1;
            header.target_mode  = NODEIDACK;
            header.target       = 1;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Node_Get()->node_id = 1;
            header.target_mode  = NODEIDACK;
            header.target       = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Node_Get()->node_id = 1;
            header.target_mode  = NODEID;
            header.target       = 1;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Node_Get()->node_id = 1;
            header.target_mode  = NODEID;
            header.target       = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Test Topic");
    {
        TRY
        {
            Filter_TopicInit();
            Filter_AddTopic(3);
            header.target_mode = TOPIC;
            header.target      = 3;
            // We should have Luos and Robus
            TEST_ASSERT_EQUAL(0x03, Phy_ComputeTargets(&header));
            header.target = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Phy_ComputeTargets(&header));
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

    RUN_TEST(unittest_Filter_ServiceId);
    RUN_TEST(unittest_Filter_Topic);
    RUN_TEST(unittest_Filter_Type);
    RUN_TEST(unittest_Phy_ComputeTargets);

    UNITY_END();
}
