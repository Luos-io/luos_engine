#include "main.h"
#include <stdio.h>
#include <unistd.h>
#include <default_scenario.h>

extern default_scenario_t default_sc;

void unittest_Streaming_SendStreamingSize()
{
    NEW_TEST_CASE("Sample size sent to streaming < Available samples");
    {
        uint32_t ExpectedMaxSize;

        Reset_Context();
        //  Init default scenario context
        Init_Context();
        //  Init variable
        ExpectedMaxSize = 1;
        // Add samples
        Stream_AddAvailableSampleNB(default_sc.streamChannel1, 2);

        NEW_STEP("Verify 1 byte is received when stream channels size is 2");
        Luos_SendStreamingSize(default_sc.App_1.app, default_sc.App_1.tx_msg, default_sc.streamChannel1, 1);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg->header.size);
    }

    NEW_TEST_CASE("Sample size sent to streaming = available samples");
    {
        uint32_t ExpectedMaxSize;

        Reset_Context();
        //  Init default scenario context
        Init_Context();
        //  Init variable
        ExpectedMaxSize = 2;
        // Add samples
        Stream_AddAvailableSampleNB(default_sc.streamChannel1, 2);

        NEW_STEP("Verify 2 byte are received when stream channels size is 2");
        Luos_SendStreamingSize(default_sc.App_1.app, default_sc.App_1.tx_msg, default_sc.streamChannel1, 2);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg->header.size);
    }

    NEW_TEST_CASE("Sample size sent to streaming > available samples");
    {
        uint32_t ExpectedMaxSize;

        Reset_Context();
        //  Init default scenario context
        Init_Context();

        //  Init variable
        ExpectedMaxSize = 2;

        NEW_STEP("Verify 2 byte are received when we try to send 3 bytes");
        // Add samples
        Stream_AddAvailableSampleNB(default_sc.streamChannel1, 2);
        Luos_SendStreamingSize(default_sc.App_1.app, default_sc.App_1.tx_msg, default_sc.streamChannel1, 3);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg->header.size);
    }

    NEW_TEST_CASE("Limit test");
    {
        uint32_t ExpectedMaxSize;

        Reset_Context();
        //  Init default scenario context
        Init_Context();

        //  Init variable test case

        NEW_STEP("Try to send 0 byte : it does nothing");

        //  Init variable new step
        ExpectedMaxSize = 0;

        // Add samples
        Stream_AddAvailableSampleNB(default_sc.streamChannel1, 2);
        Luos_SendStreamingSize(default_sc.App_1.app, default_sc.App_1.tx_msg, default_sc.streamChannel1, 0);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg->header.size);

        NEW_STEP("try to send 128 byte the max size of a msg");

        //  Init variable
        ExpectedMaxSize = 128;

        // Add samples
        Stream_AddAvailableSampleNB(default_sc.streamChannel1, 128);
        Luos_SendStreamingSize(default_sc.App_1.app, default_sc.App_1.tx_msg, default_sc.streamChannel1, 128);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg->header.size);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    ASSERT_ACTIVATION(1);

    // Streaming functions
    UNIT_TEST_RUN(unittest_Streaming_SendStreamingSize);

    UNITY_END();
}
