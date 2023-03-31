#include "main.h"
#include <stdio.h>
#include <default_scenario.h>

extern default_scenario_t default_sc;

void unittest_RTFilter_Reset(void)
{
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

void unittest_RTFilter_InitCheck(void)
{
    NEW_TEST_CASE("Test the result initialization check function");
    {
        //  Init default scenario context
        Init_Context();
        //  Init variables
        search_result_t result;
        uint8_t value;

        NEW_STEP("Verify that we have not initialized the result");
        value = RTFilter_InitCheck(&result);
        //  Verify
        TEST_ASSERT_EQUAL(FAILED, value);

        NEW_STEP("Verify that we have initialized the result");
        RTFilter_Reset(&result);
        value = RTFilter_InitCheck(&result);
        //  Verify
        TEST_ASSERT_EQUAL(SUCCEED, value);
    }
}

void unittest_RTFilter_Type(void)
{
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

void unittest_RTFilter_ID(void)
{
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

void unittest_RTFilter_Service(void)
{
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

void unittest_RTFilter_Alias()
{
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
int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Streaming functions
    UNIT_TEST_RUN(unittest_RTFilter_Reset);
    UNIT_TEST_RUN(unittest_RTFilter_InitCheck);
    UNIT_TEST_RUN(unittest_RTFilter_Type);
    UNIT_TEST_RUN(unittest_RTFilter_ID);
    UNIT_TEST_RUN(unittest_RTFilter_Service);
    UNIT_TEST_RUN(unittest_RTFilter_Node);
    UNIT_TEST_RUN(unittest_RTFilter_Alias);

    UNITY_END();
}
