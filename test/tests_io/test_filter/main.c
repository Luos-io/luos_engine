#include "unit_test.h"
#include "filter.h"
#include "../src/service.c"

void unittest_Filter_ServiceId()
{
    uint16_t service_number = 10;
    NEW_TEST_CASE("Basic Service ID mask test");
    {
        Filter_IdInit();
        TRY
        {
            Filter_AddServiceId(7, service_number);
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(8, service_number);
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
            Filter_AddServiceId(9, service_number);
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
            Filter_AddServiceId(9, 0);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    service_number = MAX_SERVICE_NUMBER;
    NEW_TEST_CASE("Service ID mask test with max services");
    {
        Filter_IdInit();

        TRY
        {
            Filter_AddServiceId(7, service_number);
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7));
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(7 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(8, service_number);
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
            Filter_AddServiceId(9, service_number);
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
            Filter_AddServiceId(9, 0);
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(8));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9));
            TEST_ASSERT_EQUAL(false, Filter_ServiceID(9 + service_number - 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Overlap the MAX_SERVICE_NUMBER");
    {
        Filter_IdInit();
        TRY
        {
            Filter_AddServiceId(1, MAX_SERVICE_NUMBER + 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test the id 4096 Limit");
    {
        Filter_IdInit();
        TRY
        {
            Filter_AddServiceId(4096 - MAX_SERVICE_NUMBER, service_number);
            TEST_ASSERT_EQUAL(true, Filter_ServiceID(4096 - MAX_SERVICE_NUMBER));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(4096 - MAX_SERVICE_NUMBER + 1, service_number);
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
        Filter_IdInit();
        TRY
        {
            Filter_AddServiceId(0, service_number);
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
            Filter_AddTopic(LAST_TOPIC);
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        Filter_TopicInit();
        TRY
        {
            Filter_AddTopic(LAST_TOPIC + 1);
            TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC + 1));
        }
        TEST_ASSERT_TRUE(IS_ASSERT());

        Filter_TopicInit();
        TRY
        {
            Filter_AddTopic(LAST_TOPIC);
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
            Filter_RmTopic(LAST_TOPIC);
            TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        Filter_TopicInit();
        TRY
        {
            Filter_RmTopic(LAST_TOPIC + 1);
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
            TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC));
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
            TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC));
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
            TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddTopic(LAST_TOPIC);
            TEST_ASSERT_EQUAL(true, Filter_Topic(0));
            TEST_ASSERT_EQUAL(true, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(true, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
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
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
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
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
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
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
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
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
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
            TEST_ASSERT_EQUAL(true, Filter_Topic(LAST_TOPIC));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_RmTopic(LAST_TOPIC);
            TEST_ASSERT_EQUAL(false, Filter_Topic(0));
            TEST_ASSERT_EQUAL(false, Filter_Topic(1));
            TEST_ASSERT_EQUAL(false, Filter_Topic(2));
            TEST_ASSERT_EQUAL(false, Filter_Topic(3));
            TEST_ASSERT_EQUAL(false, Filter_Topic(4));
            TEST_ASSERT_EQUAL(false, Filter_Topic(LAST_TOPIC));
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
            Filter_Type(4096 + 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test the result if there is no service");
    {
        TRY
        {
            service_ctx.number = 0;
            TEST_ASSERT_EQUAL(false, Filter_Type(0));
            TEST_ASSERT_EQUAL(false, Filter_Type(1));
            TEST_ASSERT_EQUAL(false, Filter_Type(2));
            TEST_ASSERT_EQUAL(false, Filter_Type(12));
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
            TEST_ASSERT_EQUAL(true, Filter_Type(0));
            TEST_ASSERT_EQUAL(false, Filter_Type(1));
            TEST_ASSERT_EQUAL(false, Filter_Type(2));
            TEST_ASSERT_EQUAL(false, Filter_Type(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            service_ctx.number       = 1;
            service_ctx.list[0].type = 1;
            TEST_ASSERT_EQUAL(false, Filter_Type(0));
            TEST_ASSERT_EQUAL(true, Filter_Type(1));
            TEST_ASSERT_EQUAL(false, Filter_Type(2));
            TEST_ASSERT_EQUAL(false, Filter_Type(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            service_ctx.number       = 1;
            service_ctx.list[0].type = 12;
            TEST_ASSERT_EQUAL(false, Filter_Type(0));
            TEST_ASSERT_EQUAL(false, Filter_Type(1));
            TEST_ASSERT_EQUAL(false, Filter_Type(2));
            TEST_ASSERT_EQUAL(true, Filter_Type(12));
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
            TEST_ASSERT_EQUAL(false, Filter_Type(0));
            TEST_ASSERT_EQUAL(true, Filter_Type(1));
            TEST_ASSERT_EQUAL(false, Filter_Type(2));
            TEST_ASSERT_EQUAL(true, Filter_Type(12));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Filter_GetPhyTarget()
{
    header_t header;
    NEW_TEST_CASE("Test a wrong header target_mode");
    {
        TRY
        {
            header.target_mode = 0x0f;
            Filter_GetPhyTarget(&header);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test service ID");
    {
        TRY
        {
            Filter_AddServiceId(1, 1);
            header.target_mode = SERVICEIDACK;
            header.target      = 1;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Filter_GetPhyTarget(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(1, 1);
            header.target_mode = SERVICEIDACK;
            header.target      = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(1, 1);
            header.target_mode = SERVICEID;
            header.target      = 1;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Filter_GetPhyTarget(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(1, 1);
            header.target_mode = SERVICEID;
            header.target      = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(1, 2);
            header.target_mode = SERVICEIDACK;
            header.target      = 2;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Filter_GetPhyTarget(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(1, 2);
            header.target_mode = SERVICEIDACK;
            header.target      = 3;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(1, 2);
            header.target_mode = SERVICEID;
            header.target      = 2;
            // We should have Luos only
            TEST_ASSERT_EQUAL(0x01, Filter_GetPhyTarget(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }

        TRY
        {
            Filter_AddServiceId(1, 2);
            header.target_mode = SERVICEID;
            header.target      = 3;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x03, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x03, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x03, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x01, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x01, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
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
            TEST_ASSERT_EQUAL(0x03, Filter_GetPhyTarget(&header));
            header.target = 2;
            // We should have Robus only
            TEST_ASSERT_EQUAL(0x02, Filter_GetPhyTarget(&header));
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

    UNIT_TEST_RUN(unittest_Filter_ServiceId);
    UNIT_TEST_RUN(unittest_Filter_Topic);
    UNIT_TEST_RUN(unittest_Filter_Type);
    UNIT_TEST_RUN(unittest_Filter_GetPhyTarget);

    UNITY_END();
}
