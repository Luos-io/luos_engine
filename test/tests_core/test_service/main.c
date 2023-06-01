#include "service.h"
#include "service.c"
#include <stdio.h>
#include "unit_test.h"
#include <default_scenario.h>

extern default_scenario_t default_sc;

void unittest_Service_Init(void)
{
    uint8_t buffer[100];
    NEW_TEST_CASE("Test Service_Init");
    {
        TRY
        {
            service_ctx.number = 10;
            Service_Init();
            TEST_ASSERT_EQUAL(0, service_ctx.number);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_GetTable(void)
{
    NEW_TEST_CASE("Test Service_GetTable");
    {
        TRY
        {
            TEST_ASSERT_EQUAL(service_ctx.list, Service_GetTable());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_GetNumber(void)
{
    NEW_TEST_CASE("Test Service_GetNumber");
    {
        TRY
        {
            TEST_ASSERT_EQUAL(service_ctx.number, Service_GetNumber());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_ResetStatistics(void)
{
    NEW_TEST_CASE("Test Service_ResetStatistics");
    {
        TRY
        {
            service_ctx.number                       = 10;
            service_ctx.list[0].statistics.max_retry = 5;
            service_ctx.list[9].statistics.max_retry = 9;
            Service_ResetStatistics();
            TEST_ASSERT_EQUAL(0, service_ctx.list[0].statistics.max_retry);
            TEST_ASSERT_EQUAL(0, service_ctx.list[9].statistics.max_retry);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_GenerateId(void)
{
    NEW_TEST_CASE("Test Service_GenerateId assert conditions");
    TRY
    {
        Service_GenerateId(0);
    }
    TEST_ASSERT_TRUE(IS_ASSERT());
    END_TRY;

    NEW_TEST_CASE("Test Service_GenerateId");
    {
        TRY
        {
            service_ctx.number = 10;
            Service_GenerateId(1); // This is a special case, because detector already saved his ID so the first id will be 2
            for (uint16_t i = 0; i < service_ctx.number; i++)
            {
                if (service_ctx.list[i].id != 1)
                {
                    TEST_ASSERT_EQUAL(i + 2, service_ctx.list[i].id);
                }
            }

            // Force the first one to have id 1
            service_ctx.list[0].id = 1;
            Service_GenerateId(1); // This is a special case, because detector already saved his ID so the first id will be 2
            for (uint16_t i = 0; i < service_ctx.number; i++)
            {
                if (service_ctx.list[i].id != 1)
                {
                    TEST_ASSERT_EQUAL(i + 1, service_ctx.list[i].id);
                }
            }

            service_ctx.list[0].id = 0;
            Service_GenerateId(5);
            for (uint16_t i = 0; i < service_ctx.number; i++)
            {
                if (service_ctx.list[i].id != 1)
                {
                    TEST_ASSERT_EQUAL(i + 5, service_ctx.list[i].id);
                }
            }
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_ClearId(void)
{
    NEW_TEST_CASE("Test Service_ClearId");
    {
        TRY
        {
            service_ctx.number = 10;
            for (uint16_t i = 0; i < service_ctx.number; i++)
            {
                service_ctx.list[i].id = i + 1;
            }
            Service_ClearId();
            for (uint16_t i = 0; i < service_ctx.number; i++)
            {
                TEST_ASSERT_EQUAL(0, service_ctx.list[i].id);
            }
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_GetIndex(void)
{

    NEW_TEST_CASE("Test Service_GetIndex assert conditions");
    {
        TRY
        {

            service_ctx.number = 10;
            Service_GetIndex(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {

            service_ctx.number = 10;
            Service_GetIndex(service_ctx.list - 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            service_ctx.number = 10;
            Service_GetIndex(&service_ctx.list[11]);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Service_GetIndex");
    {
        TRY
        {
            service_ctx.number = 10;
            TEST_ASSERT_EQUAL(0, Service_GetIndex(&service_ctx.list[0]));
            TEST_ASSERT_EQUAL(1, Service_GetIndex(&service_ctx.list[1]));
            TEST_ASSERT_EQUAL(2, Service_GetIndex(&service_ctx.list[2]));
            TEST_ASSERT_EQUAL(3, Service_GetIndex(&service_ctx.list[3]));
            TEST_ASSERT_EQUAL(4, Service_GetIndex(&service_ctx.list[4]));
            TEST_ASSERT_EQUAL(5, Service_GetIndex(&service_ctx.list[5]));
            TEST_ASSERT_EQUAL(6, Service_GetIndex(&service_ctx.list[6]));
            TEST_ASSERT_EQUAL(7, Service_GetIndex(&service_ctx.list[7]));
            TEST_ASSERT_EQUAL(8, Service_GetIndex(&service_ctx.list[8]));
            TEST_ASSERT_EQUAL(9, Service_GetIndex(&service_ctx.list[9]));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_RmAutoUpdateTarget(void)
{
    NEW_TEST_CASE("Test Service_RmAutoUpdateTarget");
    {
        TRY
        {
            service_ctx.number                           = 10;
            service_ctx.list[2].auto_refresh.target      = 2;
            service_ctx.list[2].auto_refresh.time_ms     = 20;
            service_ctx.list[2].auto_refresh.last_update = 30;
            Service_RmAutoUpdateTarget(2);
            TEST_ASSERT_EQUAL(0, service_ctx.list[2].auto_refresh.target);
            TEST_ASSERT_EQUAL(0, service_ctx.list[2].auto_refresh.time_ms);
            TEST_ASSERT_EQUAL(0, service_ctx.list[2].auto_refresh.last_update);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_AutoUpdateManager(void)
{
    NEW_TEST_CASE("Test Service_AutoUpdateManager");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            Luos_Loop();
            service_ctx.list[2].auto_refresh.target      = 1;
            service_ctx.list[2].auto_refresh.time_ms     = 10;
            service_ctx.list[2].auto_refresh.last_update = 30;
            Service_AutoUpdateManager();
            TEST_ASSERT_NOT_EQUAL(30, service_ctx.list[2].auto_refresh.last_update);
            TEST_ASSERT_EQUAL(GET_CMD, default_sc.App_3.last_rx_msg.header.cmd);
            TEST_ASSERT_EQUAL(1, default_sc.App_3.last_rx_msg.header.source);
            TEST_ASSERT_EQUAL(3, default_sc.App_3.last_rx_msg.header.target);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_GetConcerned(void)
{
    NEW_TEST_CASE("Test Service_GetConcerned assert conditions");
    {
        TRY
        {
            Service_GetConcerned(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Service_GetConcerned");
    {
        TRY
        {
            header_t header;
            header.target_mode = SERVICEIDACK;
            header.target      = default_sc.App_3.app->id;
            header.source      = default_sc.App_2.app->id;
            TEST_ASSERT_EQUAL(default_sc.App_3.app, Service_GetConcerned(&header));

            header.target_mode = SERVICEID;
            header.target      = default_sc.App_3.app->id;
            header.source      = default_sc.App_2.app->id;
            TEST_ASSERT_EQUAL(default_sc.App_3.app, Service_GetConcerned(&header));

            header.target_mode = TYPE;
            header.target      = default_sc.App_3.app->type;
            header.source      = default_sc.App_2.app->id;
            // It will be the first service because all type are the same...
            TEST_ASSERT_EQUAL(default_sc.App_1.app, Service_GetConcerned(&header));

            header.target_mode = BROADCAST;
            header.target      = default_sc.App_3.app->id;
            header.source      = default_sc.App_2.app->id;
            TEST_ASSERT_EQUAL(default_sc.App_1.app, Service_GetConcerned(&header));

            header.target_mode = NODEIDACK;
            header.target      = default_sc.App_3.app->id;
            header.source      = default_sc.App_2.app->id;
            TEST_ASSERT_EQUAL(default_sc.App_1.app, Service_GetConcerned(&header));

            header.target_mode = NODEID;
            header.target      = default_sc.App_3.app->id;
            header.source      = default_sc.App_2.app->id;
            TEST_ASSERT_EQUAL(default_sc.App_1.app, Service_GetConcerned(&header));

            header.target_mode = TOPIC;
            header.target      = default_sc.App_3.app->id;
            header.source      = default_sc.App_2.app->id;
            TEST_ASSERT_EQUAL(NULL, Service_GetConcerned(&header));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_GetFilter(void)
{
    NEW_TEST_CASE("Test Service_GetFilter assert conditions");
    {
        TRY
        {
            Service_GetFilter(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Service_GetFilter");
    {
        TRY
        {
            msg_t msg;
            msg.header.target_mode = SERVICEIDACK;
            msg.header.target      = default_sc.App_3.app->id;
            TEST_ASSERT_EQUAL(0b00000100, Service_GetFilter(&msg));

            msg.header.target_mode = SERVICEID;
            msg.header.target      = default_sc.App_3.app->id;
            TEST_ASSERT_EQUAL(0b00000100, Service_GetFilter(&msg));

            msg.header.target_mode = TYPE;
            msg.header.target      = default_sc.App_3.app->type;
            // It will be the first service because all type are the same...
            TEST_ASSERT_EQUAL(0b00000111, Service_GetFilter(&msg));

            msg.header.target_mode = BROADCAST;
            msg.header.target      = default_sc.App_3.app->id;
            TEST_ASSERT_EQUAL(0b00000111, Service_GetFilter(&msg));

            msg.header.target_mode = NODEIDACK;
            msg.header.target      = 1;
            TEST_ASSERT_EQUAL(0b00000111, Service_GetFilter(&msg));

            msg.header.target_mode = NODEID;
            msg.header.target      = 1;
            TEST_ASSERT_EQUAL(0b00000111, Service_GetFilter(&msg));

            msg.header.target_mode = TOPIC;
            msg.header.target      = default_sc.App_3.app->id;
            TEST_ASSERT_EQUAL(0b00000000, Service_GetFilter(&msg));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Service_Deliver(void)
{
    NEW_TEST_CASE("Test Service_Deliver assert conditions");
    {
        TRY
        {
            Service_Deliver(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Service_Deliver");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            Luos_Loop();
            //  Send a message to the service
            phy_job_t job;
            msg_t msg;
            msg.header.target_mode = NODEID;
            msg.header.target      = 1;
            msg.header.source      = 4;
            msg.header.cmd         = GET_CMD;
            msg.header.size        = 0;
            job.msg_pt             = &msg;

            service_filter_t filter = Service_GetFilter(&msg);
            TEST_ASSERT_EQUAL(0b00000111, filter);
            job.phy_data = &filter;
            TEST_ASSERT_EQUAL(SUCCEED, Service_Deliver(&job));
            // All services consume this message
            TEST_ASSERT_EQUAL(0b00000000, filter);
            // Check that all services received the message
            TEST_ASSERT_EQUAL(4, default_sc.App_1.last_rx_msg.header.source);
            TEST_ASSERT_EQUAL(4, default_sc.App_2.last_rx_msg.header.source);
            TEST_ASSERT_EQUAL(4, default_sc.App_3.last_rx_msg.header.source);

            // Add a new service with no callback
            revision_t revision  = {.major = 1, .minor = 0, .build = 0};
            default_sc.App_1.app = Luos_CreateService(NULL, VOID_TYPE, "Test_App", revision);
            Luos_Detect(default_sc.App_1.app);
            do
            {
                Luos_Loop();
            } while (!Luos_IsDetected());

            filter = Service_GetFilter(&msg);
            TEST_ASSERT_EQUAL(0b00001111, filter);
            job.phy_data = &filter;
            TEST_ASSERT_EQUAL(FAILED, Service_Deliver(&job));
            // All services consume this message
            TEST_ASSERT_EQUAL(0b00001000, filter);
            // Check that all services received the message
            TEST_ASSERT_EQUAL(4, default_sc.App_1.last_rx_msg.header.source);
            TEST_ASSERT_EQUAL(4, default_sc.App_2.last_rx_msg.header.source);
            TEST_ASSERT_EQUAL(4, default_sc.App_3.last_rx_msg.header.source);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_UpdateAlias(void)
{
    NEW_TEST_CASE("Test Luos_UpdateAlias assert conditions");
    {
        Init_Context();
        Luos_Loop();
        char new_alias[16] = "new_alias";
        TRY
        {
            Luos_UpdateAlias(NULL, new_alias, 16);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            Luos_UpdateAlias(default_sc.App_1.app, NULL, 16);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Luos_UpdateAlias");
    {
        Init_Context();
        Luos_Loop();
        char new_alias[40] = "new_alias1";
        TRY
        {
            TEST_ASSERT_EQUAL(SUCCEED, Luos_UpdateAlias(default_sc.App_1.app, new_alias, 16));
            TEST_ASSERT_EQUAL_STRING(new_alias, default_sc.App_1.app->alias);

            strcpy(new_alias, "new alias");
            TEST_ASSERT_EQUAL(SUCCEED, Luos_UpdateAlias(default_sc.App_1.app, new_alias, strlen(new_alias)));
            TEST_ASSERT_EQUAL_STRING("new_alias", default_sc.App_1.app->alias);

            strcpy(new_alias, "new_alias!");
            TEST_ASSERT_EQUAL(FAILED, Luos_UpdateAlias(default_sc.App_1.app, new_alias, strlen(new_alias)));
            TEST_ASSERT_EQUAL_STRING("new_alias", default_sc.App_1.app->alias);

            strcpy(new_alias, "new_alias_with_a_very_long_size");
            TEST_ASSERT_EQUAL(SUCCEED, Luos_UpdateAlias(default_sc.App_1.app, new_alias, strlen(new_alias)));
            TEST_ASSERT_EQUAL_STRING("new_alias_with_", default_sc.App_1.app->alias);

            strcpy(new_alias, "new_alias");
            TEST_ASSERT_EQUAL(SUCCEED, Luos_UpdateAlias(default_sc.App_1.app, new_alias, 0));
            TEST_ASSERT_EQUAL_STRING(default_sc.App_1.app->default_alias, default_sc.App_1.app->alias);

            strcpy(new_alias, "new_alias");
            TEST_ASSERT_EQUAL(SUCCEED, Luos_UpdateAlias(default_sc.App_1.app, new_alias, strlen(new_alias)));
            TEST_ASSERT_EQUAL_STRING("new_alias", default_sc.App_1.app->alias);

            new_alias[0] = '\0';
            TEST_ASSERT_EQUAL(SUCCEED, Luos_UpdateAlias(default_sc.App_1.app, new_alias, 16));
            TEST_ASSERT_EQUAL_STRING(default_sc.App_1.app->default_alias, default_sc.App_1.app->alias);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_ServicesClear(void)
{
    NEW_TEST_CASE("Test Luos_ServicesClear");
    {
        Init_Context();
        Luos_Loop();
        TRY
        {
            Luos_ServicesClear();
            TEST_ASSERT_EQUAL(0, service_ctx.number);
            TEST_ASSERT_EQUAL(0, service_ctx.list[0].id);
            TEST_ASSERT_EQUAL(0, service_ctx.list[3].id);
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

    UNIT_TEST_RUN(unittest_Service_Init);
    UNIT_TEST_RUN(unittest_Service_GetTable);
    UNIT_TEST_RUN(unittest_Service_GetNumber);
    UNIT_TEST_RUN(unittest_Service_ResetStatistics);
    UNIT_TEST_RUN(unittest_Service_GenerateId);
    UNIT_TEST_RUN(unittest_Service_ClearId);
    UNIT_TEST_RUN(unittest_Service_GetIndex);
    UNIT_TEST_RUN(unittest_Service_RmAutoUpdateTarget);
    UNIT_TEST_RUN(unittest_Service_AutoUpdateManager);
    UNIT_TEST_RUN(unittest_Service_GetConcerned);
    UNIT_TEST_RUN(unittest_Service_GetFilter);
    UNIT_TEST_RUN(unittest_Service_Deliver);
    UNIT_TEST_RUN(unittest_Luos_UpdateAlias);
    UNIT_TEST_RUN(unittest_Luos_ServicesClear);

    UNITY_END();
}
