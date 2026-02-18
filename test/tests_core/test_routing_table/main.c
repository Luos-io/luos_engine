#include <stdio.h>
#include <default_scenario.h>
#include "routing_table.c"

extern default_scenario_t default_sc;

void unittest_RoutingTB_IDFromAlias(void)
{
    NEW_TEST_CASE("Test RoutingTB_IDFromAlias assert conditions");
    {
        TRY
        {
            RoutingTB_IDFromAlias(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_IDFromAlias return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            TEST_ASSERT_EQUAL(1, RoutingTB_IDFromAlias("Dummy_App_1"));
            TEST_ASSERT_EQUAL(2, RoutingTB_IDFromAlias("Dummy_App_2"));
            TEST_ASSERT_EQUAL(3, RoutingTB_IDFromAlias("Dummy_App_3"));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_GetServiceIndex(void)
{
    NEW_TEST_CASE("Test RoutingTB_GetServiceIndex assert conditions");
    {
        TRY
        {
            RoutingTB_GetServiceIndex(0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_GetServiceIndex return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            TEST_ASSERT_EQUAL(1, RoutingTB_GetServiceIndex(1));
            TEST_ASSERT_EQUAL(2, RoutingTB_GetServiceIndex(2));
            TEST_ASSERT_EQUAL(3, RoutingTB_GetServiceIndex(3));
            TEST_ASSERT_EQUAL(0, RoutingTB_GetServiceIndex(4));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_NodeIDFromID(void)
{
    NEW_TEST_CASE("Test RoutingTB_NodeIDFromID assert conditions");
    {
        TRY
        {
            RoutingTB_NodeIDFromID(0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_NodeIDFromID return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            TEST_ASSERT_EQUAL(1, RoutingTB_NodeIDFromID(1));
            TEST_ASSERT_EQUAL(1, RoutingTB_NodeIDFromID(2));
            TEST_ASSERT_EQUAL(1, RoutingTB_NodeIDFromID(3));
            TEST_ASSERT_EQUAL(0, RoutingTB_NodeIDFromID(4));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_ComputeRoutingTableEntryNB(void)
{
    NEW_TEST_CASE("check RoutingTB_NodeIDFromID return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            TEST_ASSERT_EQUAL(4, last_routing_table_entry); // 3 services + 1 nodes
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_AliasFromId(void)
{
    NEW_TEST_CASE("Test RoutingTB_AliasFromId assert conditions");
    {
        TRY
        {
            RoutingTB_AliasFromId(0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_AliasFromId return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            TEST_ASSERT_EQUAL_STRING("Dummy_App_1", RoutingTB_AliasFromId(1));
            TEST_ASSERT_EQUAL_STRING("Dummy_App_2", RoutingTB_AliasFromId(2));
            TEST_ASSERT_EQUAL_STRING("Dummy_App_3", RoutingTB_AliasFromId(3));
            TEST_ASSERT_EQUAL_STRING(0, RoutingTB_AliasFromId(4));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_BigestID(void)
{
    NEW_TEST_CASE("check RoutingTB_BigestID return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            TEST_ASSERT_EQUAL(3, RoutingTB_BigestID());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_BigestNodeID(void)
{
    NEW_TEST_CASE("check RoutingTB_BigestNodeID return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            TEST_ASSERT_EQUAL(1, RoutingTB_BigestNodeID());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_AddNumToAlias(void)
{
    NEW_TEST_CASE("Test RoutingTB_AddNumToAlias assert conditions");
    {
        TRY
        {
            RoutingTB_AddNumToAlias(NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            char alias[MAX_ALIAS_SIZE] = "Dummy_App";
            RoutingTB_AddNumToAlias(alias, 100);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            char alias[MAX_ALIAS_SIZE] = "Dummy_App";
            RoutingTB_AddNumToAlias(alias, 101);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_AddNumToAlias return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            char alias[MAX_ALIAS_SIZE] = "Dummy_App";
            RoutingTB_AddNumToAlias(alias, 1);
            TEST_ASSERT_EQUAL_STRING("Dummy_App1", alias);
            RoutingTB_AddNumToAlias(alias, 2);
            TEST_ASSERT_EQUAL_STRING("Dummy_App12", alias);
            RoutingTB_AddNumToAlias(alias, 3);
            TEST_ASSERT_EQUAL_STRING("Dummy_App123", alias);

            char alias_big[MAX_ALIAS_SIZE] = "Dummy_App123456";
            RoutingTB_AddNumToAlias(alias_big, 1);
            TEST_ASSERT_EQUAL_STRING("Dummy_App123451", alias_big);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_ConvertNodeToRoutingTable(void)
{
    NEW_TEST_CASE("Test RoutingTB_ConvertNodeToRoutingTable assert conditions");
    {
        TRY
        {
            node_t node;
            RoutingTB_ConvertNodeToRoutingTable(NULL, &node);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            routing_table_t entry;
            RoutingTB_ConvertNodeToRoutingTable(&entry, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_ConvertNodeToRoutingTable return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            routing_table_t entry;
            node_t node;
            node.certified                 = false;
            node.node_id                   = 10;
            node.node_info                 = 20;
            node.connection.parent.node_id = 30;
            node.connection.child.node_id  = 40;

            RoutingTB_ConvertNodeToRoutingTable(&entry, &node);
            TEST_ASSERT_EQUAL(NODE, entry.mode);
            TEST_ASSERT_EQUAL(false, entry.certified);
            TEST_ASSERT_EQUAL(10, entry.node_id);
            TEST_ASSERT_EQUAL(20, entry.node_info);
            TEST_ASSERT_EQUAL(30, entry.connection.parent.node_id);
            TEST_ASSERT_EQUAL(40, entry.connection.child.node_id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_ConvertServiceToRoutingTable(void)
{
    NEW_TEST_CASE("Test RoutingTB_ConvertServiceToRoutingTable assert conditions");
    {
        TRY
        {
            service_t service;
            RoutingTB_ConvertServiceToRoutingTable(NULL, &service);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            routing_table_t entry;
            RoutingTB_ConvertServiceToRoutingTable(&entry, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_ConvertServiceToRoutingTable return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            routing_table_t entry;
            service_t service;
            service.id = 10;
            strcpy((char *)service.alias, "Dummy_App");
            service.type = DISTANCE_TYPE;

            RoutingTB_ConvertServiceToRoutingTable(&entry, &service);
            TEST_ASSERT_EQUAL(SERVICE, entry.mode);
            TEST_ASSERT_EQUAL(10, entry.id);
            TEST_ASSERT_EQUAL_STRING("Dummy_App", entry.alias);
            TEST_ASSERT_EQUAL(DISTANCE_TYPE, entry.type);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_RemoveService(void)
{
    NEW_TEST_CASE("Test RoutingTB_RemoveService assert conditions");
    {
        TRY
        {
            RoutingTB_RemoveService(0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("check RoutingTB_RemoveService return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();

            RoutingTB_RemoveService(2);
            TEST_ASSERT_EQUAL(3, last_routing_table_entry);
            TEST_ASSERT_EQUAL(1, routing_table[1].id);
            TEST_ASSERT_EQUAL(3, routing_table[2].id);
            TEST_ASSERT_EQUAL(0, routing_table[3].id);
            TEST_ASSERT_EQUAL(3, last_service);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;

        TRY
        {
            //  Init default scenario context
            Init_Context();

            RoutingTB_RemoveService(3);
            TEST_ASSERT_EQUAL(3, last_routing_table_entry);
            TEST_ASSERT_EQUAL(1, routing_table[1].id);
            TEST_ASSERT_EQUAL(2, routing_table[2].id);
            TEST_ASSERT_EQUAL(0, routing_table[3].id);
            TEST_ASSERT_EQUAL(2, last_service);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_RemoveNode(void)
{
    NEW_TEST_CASE("check RoutingTB_RemoveNode return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            RoutingTB_RemoveNode(1);
            TEST_ASSERT_EQUAL(1, last_routing_table_entry);
            TEST_ASSERT_EQUAL(0, routing_table[1].id);
            TEST_ASSERT_EQUAL(0, last_service);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_Erase(void)
{
    NEW_TEST_CASE("check RoutingTB_Erase return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            RoutingTB_Erase();
            TEST_ASSERT_EQUAL(0, last_routing_table_entry);
            TEST_ASSERT_EQUAL(0, last_service);
            TEST_ASSERT_EQUAL(0, routing_table[0].id);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_Get(void)
{
    NEW_TEST_CASE("check RoutingTB_Get return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            TEST_ASSERT_EQUAL(routing_table, RoutingTB_Get());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RoutingTB_GetLastEntry(void)
{
    NEW_TEST_CASE("check RoutingTB_GetLastEntry return value");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            TEST_ASSERT_EQUAL(last_routing_table_entry, RoutingTB_GetLastEntry());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_RTFilter_InitCheck(void)
{
    NEW_TEST_CASE("Test RTFilter_InitCheck assert conditions");
    {
        TRY
        {
            RTFilter_InitCheck(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test the result initialization check function");
    {
        //  Init default scenario context
        Init_Context();
        //  Init variables
        search_result_t result = {0};

        NEW_STEP("Verify that we have not initialized the result");
        TEST_ASSERT_EQUAL(FAILED, RTFilter_InitCheck(&result));

        NEW_STEP("Verify that we have initialized the result");
        RTFilter_Reset(&result);
        TEST_ASSERT_EQUAL(SUCCEED, RTFilter_InitCheck(&result));
    }
}

void unittest_RTFilter_Reset(void)
{
    NEW_TEST_CASE("Test RTFilter_Reset assert conditions");
    {
        TRY
        {
            RTFilter_Reset(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test the services in the result table");
    {
        uint32_t ExpectedServiceNB;

        //  Init default scenario context
        Init_Context();
        //  Init variables
        ExpectedServiceNB = 3;
        search_result_t result;
        char alias[MAX_ALIAS_SIZE] = {0};
        uint8_t alias_result;
        // Add samples
        RTFilter_Reset(&result);

        NEW_STEP("Verify that we have 3 services created");
        // Luos_Loop();
        //  Verify
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify the contents of the result table");

        for (uint8_t i = 0; i < result.result_nbr; i++)
        {
            TEST_ASSERT_EQUAL(i + 1, result.result_table[i]->id);
            TEST_ASSERT_EQUAL(VOID_TYPE, result.result_table[i]->type);

            sprintf(alias, "Dummy_App_%d", i + 1);
            alias_result = strcmp(alias, result.result_table[i]->alias);
            TEST_ASSERT_EQUAL(0, alias_result);
        }
    }
}

void unittest_RTFilter_ID(void)
{
    NEW_TEST_CASE("Test RTFilter_ID assert conditions");
    {
        TRY
        {
            RTFilter_ID(NULL, 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            search_result_t result;
            RTFilter_ID(&result, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test the id filtering result number");
    {
        uint32_t ExpectedServiceNB;

        //  Init default scenario context
        Init_Context();
        //  Init variables
        ExpectedServiceNB = 1;
        search_result_t result;
        // Add samples

        NEW_STEP("Verify that we have 1 service with this exact id");
        RTFilter_ID(RTFilter_Reset(&result), 2);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have the right id");
        TEST_ASSERT_EQUAL(2, result.result_table[0]->id);

        NEW_STEP("Verify that we have no service with id bigger than 3");
        ExpectedServiceNB = 0;
        RTFilter_ID(RTFilter_Reset(&result), 4);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have no service with id 0");
        ExpectedServiceNB = 0;
        RTFilter_ID(RTFilter_Reset(&result), 0);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);
    }
}

void unittest_RTFilter_Type(void)
{
    NEW_TEST_CASE("Test RTFilter_Type assert conditions");
    {
        TRY
        {
            RTFilter_Type(NULL, VOID_TYPE);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test the type filtering without initialization");
    {
        RESET_ASSERT();

        //  Init default scenario context
        Init_Context();
        //  Init variables
        search_result_t result;
        result.result_nbr = 0;
        RTFilter_Type(&result, VOID_TYPE);
        NEW_STEP("Test result_nbr is set to 0");
        TEST_ASSERT_EQUAL(0, result.result_nbr);
    }
    NEW_TEST_CASE("Test the type filtering result number");
    {
        RESET_ASSERT();
        uint32_t ExpectedServiceNB;

        //  Init default scenario context
        Init_Context();
        //  Init variables
        ExpectedServiceNB = 3;
        search_result_t result;
        // Add samples
        RTFilter_Type(RTFilter_Reset(&result), VOID_TYPE);

        NEW_STEP("Verify that we have all the 3 services that we initialized");
        // Luos_Loop();
        //  Verify
        TEST_ASSERT_FALSE(IS_ASSERT());
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);
    }
    NEW_TEST_CASE("Add new service and retest");
    {
        uint32_t ExpectedServiceNB;
        //  Init default scenario context
        Init_Context();
        revision_t revision = {.major = 1, .minor = 0, .build = 0};

        Luos_CreateService(0, STATE_TYPE, "mycustom_service", revision);

        Luos_Detect(default_sc.App_1.app);
        do
        {
            Luos_Loop();
        } while (!Luos_IsDetected());
        //  Init variables
        search_result_t result;

        NEW_STEP("Verify that we have the 4 services");
        RTFilter_Reset(&result);
        ExpectedServiceNB = 4;
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have the 3 VOID_TYPE services");
        RTFilter_Type(RTFilter_Reset(&result), VOID_TYPE);
        ExpectedServiceNB = 3;
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have the STATE_TYPE service");
        ExpectedServiceNB = 1;
        RTFilter_Type(RTFilter_Reset(&result), STATE_TYPE);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have no services in the result");
        ExpectedServiceNB = 0;
        RTFilter_Type(RTFilter_Reset(&result), COLOR_TYPE);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);
    }
}

void unittest_RTFilter_Node(void)
{
    NEW_TEST_CASE("Test RTFilter_Node assert conditions");
    {
        TRY
        {
            RTFilter_Node(NULL, 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            search_result_t result;
            RTFilter_Node(&result, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test the node filtering result number");
    {
        uint32_t ExpectedServiceNB;

        //  Init default scenario context
        Init_Context();
        //  Init variables
        ExpectedServiceNB = 3;
        search_result_t result;
        // Add samples

        NEW_STEP("Verify that we have all the 3 services");
        RTFilter_Node(RTFilter_Reset(&result), 1);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have all no service");
        ExpectedServiceNB = 0;
        RTFilter_Node(RTFilter_Reset(&result), 2);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);
    }
}

void unittest_RTFilter_Alias()
{
    NEW_TEST_CASE("Test RTFilter_Alias assert conditions");
    {
        TRY
        {
            RTFilter_Alias(NULL, "Dummy");
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            search_result_t result;
            RTFilter_Alias(&result, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test the alias filtering result number");
    {
        uint32_t ExpectedServiceNB;

        //  Init default scenario context
        Init_Context();
        revision_t revision = {.major = 1, .minor = 0, .build = 0};

        Luos_CreateService(0, STATE_TYPE, "Custom_App", revision);

        Luos_Detect(default_sc.App_1.app);
        do
        {
            Luos_Loop();
        } while (!Luos_IsDetected());
        //  Init variables
        ExpectedServiceNB = 3;
        search_result_t result;
        // Add samples
        RTFilter_Alias(RTFilter_Reset(&result), "Dummy");

        NEW_STEP("Verify that we have all the 3 Dummy services");
        // Luos_Loop();
        //  Verify
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have all the 3 Dummy_App_ services");
        RTFilter_Alias(RTFilter_Reset(&result), "Dummy_App_");
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        ExpectedServiceNB = 4;
        NEW_STEP("Verify that we have all the 4 services");
        RTFilter_Alias(RTFilter_Reset(&result), "App");
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        ExpectedServiceNB = 1;
        NEW_STEP("Verify that we have the 1 MyCustomApp service");
        RTFilter_Alias(RTFilter_Reset(&result), "Custom_App");
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have all the 1 Dummy_App_2");
        RTFilter_Alias(RTFilter_Reset(&result), "Dummy_App_2");
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have all the no Led alias");
        ExpectedServiceNB = 0;
        RTFilter_Alias(RTFilter_Reset(&result), "Led");
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);
    }
}

void unittest_RTFilter_Service(void)
{
    NEW_TEST_CASE("Test RTFilter_Service assert conditions");
    {
        TRY
        {
            service_t service;
            RTFilter_Service(NULL, &service);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            search_result_t result;
            RTFilter_Service(&result, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test the id filtering result number");
    {
        uint32_t ExpectedServiceNB;

        //  Init default scenario context
        Init_Context();
        ExpectedServiceNB = 1;
        search_result_t result;
        // Add samples

        NEW_STEP("Verify that we have 1 service found with this pointer");
        RTFilter_Service(RTFilter_Reset(&result), default_sc.App_3.app);
        TEST_ASSERT_EQUAL(ExpectedServiceNB, result.result_nbr);

        NEW_STEP("Verify that we have assert if we put 0 at service pointer");
        //  Init variables
        ExpectedServiceNB = 0;
        RTFilter_Reset(&result);
        result.result_nbr = 0;

        RTFilter_Service(&result, 0);
        TEST_ASSERT_TRUE(IS_ASSERT());
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    UNIT_TEST_RUN(unittest_RoutingTB_IDFromAlias);
    UNIT_TEST_RUN(unittest_RoutingTB_GetServiceIndex);
    UNIT_TEST_RUN(unittest_RoutingTB_NodeIDFromID);
    UNIT_TEST_RUN(unittest_RoutingTB_AliasFromId);
    UNIT_TEST_RUN(unittest_RoutingTB_BigestID);
    UNIT_TEST_RUN(unittest_RoutingTB_BigestNodeID);
    UNIT_TEST_RUN(unittest_RoutingTB_ComputeRoutingTableEntryNB);
    UNIT_TEST_RUN(unittest_RoutingTB_AddNumToAlias);
    UNIT_TEST_RUN(unittest_RoutingTB_ConvertNodeToRoutingTable);
    UNIT_TEST_RUN(unittest_RoutingTB_ConvertServiceToRoutingTable);
    UNIT_TEST_RUN(unittest_RoutingTB_RemoveService);
    UNIT_TEST_RUN(unittest_RoutingTB_RemoveNode);
    UNIT_TEST_RUN(unittest_RoutingTB_Erase);
    UNIT_TEST_RUN(unittest_RoutingTB_Get);
    UNIT_TEST_RUN(unittest_RoutingTB_GetLastEntry);
    UNIT_TEST_RUN(unittest_RTFilter_Reset);
    UNIT_TEST_RUN(unittest_RTFilter_InitCheck);
    UNIT_TEST_RUN(unittest_RTFilter_Type);
    UNIT_TEST_RUN(unittest_RTFilter_ID);
    UNIT_TEST_RUN(unittest_RTFilter_Service);
    UNIT_TEST_RUN(unittest_RTFilter_Node);
    UNIT_TEST_RUN(unittest_RTFilter_Alias);

    UNITY_END();
}
