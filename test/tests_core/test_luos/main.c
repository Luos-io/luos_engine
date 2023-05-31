#include "main.h"
#include <stdio.h>
#include <default_scenario.h>
#define Streaming_BUFFER_SIZE 1024

extern default_scenario_t default_sc;

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

    UNITY_END();
}
