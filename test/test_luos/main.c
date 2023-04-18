#include "main.h"
#include <stdio.h>
#include <default_scenario.h>
#define Streaming_BUFFER_SIZE 1024

extern default_scenario_t default_sc;

void unittest_Luos_SendStreamingStreamingSize()
{
    NEW_TEST_CASE("Sample size sent to streaming < Available samples");
    {
        uint32_t ExpectedMaxSize;
        msg_t tx_msg;
        tx_msg.header.target                            = 2;
        tx_msg.header.target_mode                       = SERVICEIDACK;
        tx_msg.header.cmd                               = DEFAULT_CMD;
        uint8_t Streaming_Buffer[Streaming_BUFFER_SIZE] = {0};
        streaming_channel_t streamChannel               = Streaming_CreateChannel(Streaming_Buffer, Streaming_BUFFER_SIZE, 1);

        //  Init default scenario context
        Init_Context();
        //  Init variable
        ExpectedMaxSize = 1;
        // Add samples
        Streaming_AddAvailableSampleNB(&streamChannel, 2);

        NEW_STEP("Verify 1 byte is received when stream channels size is 2");
        Luos_SendStreamingSize(default_sc.App_1.app, &tx_msg, &streamChannel, 1);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg.header.size);
    }

    NEW_TEST_CASE("Sample size sent to streaming = available samples");
    {
        uint32_t ExpectedMaxSize;
        msg_t tx_msg;
        tx_msg.header.target                            = 2;
        tx_msg.header.target_mode                       = SERVICEIDACK;
        tx_msg.header.cmd                               = DEFAULT_CMD;
        uint8_t Streaming_Buffer[Streaming_BUFFER_SIZE] = {0};
        streaming_channel_t streamChannel               = Streaming_CreateChannel(Streaming_Buffer, Streaming_BUFFER_SIZE, 1);

        //  Init default scenario context
        Init_Context();
        //  Init variable
        ExpectedMaxSize = 2;
        // Add samples
        Streaming_AddAvailableSampleNB(&streamChannel, 2);

        NEW_STEP("Verify 2 byte are received when stream channels size is 2");
        Luos_SendStreamingSize(default_sc.App_1.app, &tx_msg, &streamChannel, 2);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg.header.size);
    }

    NEW_TEST_CASE("Sample size sent to streaming > available samples");
    {
        uint32_t ExpectedMaxSize;
        msg_t tx_msg;
        tx_msg.header.target                            = 2;
        tx_msg.header.target_mode                       = SERVICEIDACK;
        tx_msg.header.cmd                               = DEFAULT_CMD;
        uint8_t Streaming_Buffer[Streaming_BUFFER_SIZE] = {0};
        streaming_channel_t streamChannel               = Streaming_CreateChannel(Streaming_Buffer, Streaming_BUFFER_SIZE, 1);

        //  Init default scenario context
        Init_Context();

        //  Init variable
        ExpectedMaxSize = 2;

        NEW_STEP("Verify 2 byte are received when we try to send 3 bytes");
        // Add samples
        Streaming_AddAvailableSampleNB(&streamChannel, 2);
        Luos_SendStreamingSize(default_sc.App_1.app, &tx_msg, &streamChannel, 3);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg.header.size);
    }

    NEW_TEST_CASE("Limit test");
    {
        uint32_t ExpectedMaxSize;
        msg_t tx_msg;
        tx_msg.header.target                            = 2;
        tx_msg.header.target_mode                       = SERVICEIDACK;
        tx_msg.header.cmd                               = DEFAULT_CMD;
        uint8_t Streaming_Buffer[Streaming_BUFFER_SIZE] = {0};
        streaming_channel_t streamChannel               = Streaming_CreateChannel(Streaming_Buffer, Streaming_BUFFER_SIZE, 1);

        //  Init default scenario context
        Init_Context();

        //  Init variable test case

        NEW_STEP("Try to send 0 byte : it does nothing");

        //  Init variable new step
        ExpectedMaxSize      = 0;
        tx_msg.header.target = 3;

        // Add samples
        Streaming_AddAvailableSampleNB(&streamChannel, 2);
        Luos_SendStreamingSize(default_sc.App_1.app, &tx_msg, &streamChannel, 0);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_3.last_rx_msg.header.size);

        NEW_STEP("try to send 128 byte the max size of a msg");

        //  Init variable
        ExpectedMaxSize      = 128;
        tx_msg.header.target = 2;

        // Add samples
        Streaming_AddAvailableSampleNB(&streamChannel, 128);
        Luos_SendStreamingSize(default_sc.App_1.app, &tx_msg, &streamChannel, 128);
        Luos_Loop();
        // Verify
        TEST_ASSERT_EQUAL(ExpectedMaxSize, default_sc.App_2.last_rx_msg.header.size);
    }
}

void unittest_Luos_ReceiveData()
{
    NEW_TEST_CASE("Try to send a void message argument");
    {
        //  Init default scenario context
        Init_Context();
        revision_t revision   = {.major = 1, .minor = 0, .build = 0};
        service_t *service    = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
        uint32_t bin_data[64] = {0xDEADBEEF};

        NEW_STEP("Verify if we assert");
        RESET_ASSERT();
        TEST_ASSERT_EQUAL(Luos_ReceiveData(NULL, 0, bin_data), -1);
        RESET_ASSERT();
    }

    NEW_TEST_CASE("Try to send a void table argument");
    {
        //  Init default scenario context
        Init_Context();
        revision_t revision = {.major = 1, .minor = 0, .build = 0};
        service_t *service  = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
        msg_t msg;
        NEW_STEP("Verify function returns -1");
        Luos_ReceiveData(service, &msg, 0);
        TEST_ASSERT_TRUE(IS_ASSERT());
        RESET_ASSERT();
    }

    NEW_TEST_CASE("Try to send a shity service argument");
    {
        //  Init default scenario context
        Init_Context();
        msg_t msg;
        uint32_t bin_data[64] = {0xDEADBEEF};
        TRY
        {
            NEW_STEP("Verify if we Assert");
            Luos_ReceiveData((service_t *)10, &msg, bin_data);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
    }

    NEW_TEST_CASE("Test the regular usage");
    {
        //  Init default scenario context
        Init_Context();
        revision_t revision = {.major = 1, .minor = 0, .build = 0};
        service_t *service  = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
        msg_t msg;
        uint8_t bin_data[256] = {0};

        NEW_STEP("Verify that the first message return 0 meaning message is not completely received");
        // Set first message
        msg.header.size = 256;
        memset(msg.data, 0xAA, 128);
        TEST_ASSERT_EQUAL(Luos_ReceiveData(service, &msg, bin_data), 0);

        NEW_STEP("Verify that the second message return 256 byte received");
        msg.header.size = 128;
        TEST_ASSERT_EQUAL(Luos_ReceiveData(service, &msg, bin_data), 256);

        NEW_STEP("Check if the data is OK");
        for (int i = 0; i < 256; i++)
        {
            TEST_ASSERT_EQUAL(bin_data[i], 0xAA);
        }
    }

    NEW_TEST_CASE("Try to send a void service argument to reset the data reception");
    {
        //  Init default scenario context
        Init_Context();
        revision_t revision = {.major = 1, .minor = 0, .build = 0};
        service_t *service  = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
        msg_t msg;
        uint8_t bin_data[256] = {0};

        NEW_STEP("Verify that the first message return 0 meaning message is not completely received");
        // Set first message
        msg.header.size = 256;
        memset(msg.data, 0xAA, 128);
        TEST_ASSERT_EQUAL(Luos_ReceiveData(service, &msg, bin_data), 0);

        NEW_STEP("Verify if we return an error which mean the data reception have been reseted");
        TEST_ASSERT_EQUAL(Luos_ReceiveData(0, &msg, bin_data), -1);

        NEW_STEP("Verify that the second message return 128 byte received half of the transmitted data because we reset it in the middle");
        msg.header.size = 128;
        TEST_ASSERT_EQUAL(Luos_ReceiveData(service, &msg, bin_data), 128);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Big data reception
    UNIT_TEST_RUN(unittest_Luos_ReceiveData);
    // Streaming functions
    UNIT_TEST_RUN(unittest_Luos_SendStreamingStreamingSize);

    UNITY_END();
}
