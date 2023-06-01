#include "node.c"
#include <stdio.h>
#include "unit_test.h"
#include <default_scenario.h>

extern default_scenario_t default_sc;

void unittest_Node_Get(void)
{
    NEW_TEST_CASE("Test Node_Get");
    {
        TRY
        {
            TEST_ASSERT_EQUAL(&node_ctx.info, Node_Get());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Node_GetState(void)
{
    NEW_TEST_CASE("Test Node_GetState");
    {
        TRY
        {
            TEST_ASSERT_EQUAL(node_ctx.state, Node_GetState());
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Node_Init(void)
{
    NEW_TEST_CASE("Test Node_Init");
    {
        TRY
        {
            node_ctx.info.node_id   = 10;
            node_ctx.info.certified = true;
            node_ctx.info.node_info = 1;
            node_ctx.timeout_run    = true;
            node_ctx.timeout        = 1;
            Node_Init();
            TEST_ASSERT_EQUAL(DEFAULTID, node_ctx.info.node_id);
            TEST_ASSERT_EQUAL(false, node_ctx.info.certified);
            TEST_ASSERT_EQUAL(0, node_ctx.info.node_info);
            TEST_ASSERT_EQUAL(false, node_ctx.timeout_run);
            TEST_ASSERT_EQUAL(0, node_ctx.timeout);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Node_SetState(void)
{
    NEW_TEST_CASE("Test Node_SetState assert condition");
    {
        TRY
        {
            Node_SetState(EXTERNAL_DETECTION + 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Node_SetState");
    {
        TRY
        {

            node_ctx.state       = LOCAL_DETECTION;
            node_ctx.timeout_run = false;
            node_ctx.timeout     = 1;
            Node_SetState(NO_DETECTION);
            TEST_ASSERT_EQUAL(0, node_ctx.timeout);
            TEST_ASSERT_EQUAL(false, node_ctx.timeout_run);
            TEST_ASSERT_EQUAL(NO_DETECTION, node_ctx.state);

            node_ctx.state       = LOCAL_DETECTION;
            node_ctx.timeout_run = false;
            node_ctx.timeout     = 1;
            Node_SetState(DETECTION_OK);
            TEST_ASSERT_EQUAL(0, node_ctx.timeout);
            TEST_ASSERT_EQUAL(false, node_ctx.timeout_run);
            TEST_ASSERT_EQUAL(DETECTION_OK, node_ctx.state);

            node_ctx.state       = NO_DETECTION;
            node_ctx.timeout_run = true;
            node_ctx.timeout     = 1;
            Node_SetState(LOCAL_DETECTION);
            TEST_ASSERT_NOT_EQUAL(0, node_ctx.timeout);
            TEST_ASSERT_EQUAL(true, node_ctx.timeout_run);
            TEST_ASSERT_EQUAL(LOCAL_DETECTION, node_ctx.state);

            node_ctx.state       = NO_DETECTION;
            node_ctx.timeout_run = true;
            node_ctx.timeout     = 1;
            Node_SetState(EXTERNAL_DETECTION);
            TEST_ASSERT_NOT_EQUAL(0, node_ctx.timeout);
            TEST_ASSERT_EQUAL(true, node_ctx.timeout_run);
            TEST_ASSERT_EQUAL(EXTERNAL_DETECTION, node_ctx.state);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Node_Loop(void)
{
    NEW_TEST_CASE("Test Node_Loop");
    {
        TRY
        {
            node_ctx.state       = LOCAL_DETECTION;
            node_ctx.timeout_run = true;
            node_ctx.timeout     = 1;
            // wait some time to trigger the timeout condition
            while (Luos_GetSystick() - node_ctx.timeout <= DETECTION_TIMEOUT_MS)
                ;
            Node_Loop();
            TEST_ASSERT_EQUAL(0, node_ctx.timeout);
            TEST_ASSERT_EQUAL(false, node_ctx.timeout_run);
            TEST_ASSERT_EQUAL(NO_DETECTION, node_ctx.state);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_IsDetected(void)
{
    NEW_TEST_CASE("Test Luos_IsDetected");
    TRY
    {
        node_ctx.state = NO_DETECTION;
        TEST_ASSERT_EQUAL(false, Luos_IsDetected());
        node_ctx.state = LOCAL_DETECTION;
        TEST_ASSERT_EQUAL(false, Luos_IsDetected());
        node_ctx.state = EXTERNAL_DETECTION;
        TEST_ASSERT_EQUAL(false, Luos_IsDetected());
        node_ctx.state = DETECTION_OK;
        TEST_ASSERT_EQUAL(true, Luos_IsDetected());
    }
    CATCH
    {
        TEST_ASSERT_TRUE(false);
    }
    END_TRY;
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    UNIT_TEST_RUN(unittest_Node_Get);
    UNIT_TEST_RUN(unittest_Node_GetState);
    UNIT_TEST_RUN(unittest_Node_Init);
    UNIT_TEST_RUN(unittest_Node_SetState);
    UNIT_TEST_RUN(unittest_Node_Loop);
    UNIT_TEST_RUN(unittest_Luos_IsDetected);

    UNITY_END();
}
