#include <stdio.h>
#include <default_scenario.h>
#include "luos_engine.c"

extern default_scenario_t default_sc;

// Init and Loop are used and tested in the default scenario

void unittest_Luos_GetVersion(void)
{
    NEW_TEST_CASE("Test Luos_GetVersion");
    {
        TRY
        {
            const revision_t *rev = Luos_GetVersion();
            TEST_ASSERT_EQUAL(&luos_version, rev);
        }
    }
}

void unittest_Luos_Send(void)
{
    NEW_TEST_CASE("Test Luos_Send assert conditions");
    {
        TRY
        {
            NEW_STEP("Try to send a void message argument");
            Luos_Send(NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Luos_Send normal conditions");
    {
        TRY
        {
            NEW_STEP("Try to send in a prohibited condition");
            //  Init default scenario context
            Init_Context();
            // fake detection condition
            default_sc.App_3.app->id = 0;
            msg_t msg;
            msg.header.target      = default_sc.App_2.app->id;
            msg.header.target_mode = SERVICEIDACK;
            msg.header.cmd         = LUOS_LAST_RESERVED_CMD + 1;
            msg.header.size        = 0;
            TEST_ASSERT_EQUAL(PROHIBITED, Luos_Send(default_sc.App_3.app, &msg));

            NEW_STEP("Try to send with no service and check if the first service is used");
            //  Init default scenario context
            Init_Context();
            TEST_ASSERT_EQUAL(SUCCEED, Luos_Send(NULL, &msg));
            TEST_ASSERT_EQUAL(default_sc.App_1.app->id, msg.header.source);
            Luos_Loop();
            TEST_ASSERT_EQUAL(default_sc.App_1.app->id, default_sc.App_2.last_rx_msg.header.source);

            NEW_STEP("Try to send with a not configured service");
            //  Init default scenario context
            Init_Context();
            default_sc.App_3.app->id = 0;
            msg.header.cmd           = LUOS_LAST_RESERVED_CMD - 1;
            TEST_ASSERT_EQUAL(SUCCEED, Luos_Send(default_sc.App_3.app, &msg));
            TEST_ASSERT_EQUAL(Node_Get()->node_id, msg.header.source);
            Luos_Loop();
            TEST_ASSERT_EQUAL(Node_Get()->node_id, default_sc.App_2.last_rx_msg.header.source);

            NEW_STEP("Try to do a normal send");
            //  Init default scenario context
            Init_Context();
            TEST_ASSERT_EQUAL(SUCCEED, Luos_Send(default_sc.App_3.app, &msg));
            TEST_ASSERT_EQUAL(default_sc.App_3.app->id, msg.header.source);
            Luos_Loop();
            TEST_ASSERT_EQUAL(default_sc.App_3.app->id, default_sc.App_2.last_rx_msg.header.source);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_SendMsg(void)
{
    NEW_TEST_CASE("Test Luos_SendMsg assert conditions");
    {
        TRY
        {
            Luos_SendMsg(NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Luos_SendMsg normal conditions");
    {

        TRY
        {
            //  Init default scenario context
            Init_Context();
            msg_t msg;
            msg.header.target      = default_sc.App_2.app->id;
            msg.header.target_mode = SERVICEIDACK;
            msg.header.cmd         = LUOS_LAST_RESERVED_CMD + 1;
            msg.header.size        = 0;
            TEST_ASSERT_EQUAL(SUCCEED, Luos_SendMsg(default_sc.App_3.app, &msg));
            TEST_ASSERT_EQUAL(default_sc.App_3.app->id, msg.header.source);
            TEST_ASSERT_EQUAL(BASE_PROTOCOL, msg.header.config);
            Luos_Loop();
            TEST_ASSERT_EQUAL(default_sc.App_3.app->id, default_sc.App_2.last_rx_msg.header.source);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_SendTimestampMsg(void)
{
    NEW_TEST_CASE("Test Luos_SendTimestampMsg assert conditions");
    {
        TRY
        {
            time_luos_t timestamp;
            Luos_SendTimestampMsg(NULL, 0, timestamp);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Luos_SendTimestampMsg normal conditions");
    {

        TRY
        {
            //  Init default scenario context
            Init_Context();
            msg_t msg;
            msg.header.target      = default_sc.App_2.app->id;
            msg.header.target_mode = SERVICEIDACK;
            msg.header.cmd         = LUOS_LAST_RESERVED_CMD + 1;
            msg.header.size        = 0;

            time_luos_t timestamp = TimeOD_TimeFrom_s(2);
            TEST_ASSERT_EQUAL(SUCCEED, Luos_SendTimestampMsg(default_sc.App_3.app, &msg, timestamp));
            TEST_ASSERT_EQUAL(default_sc.App_3.app->id, msg.header.source);
            TEST_ASSERT_EQUAL(TIMESTAMP_PROTOCOL, msg.header.config);
            Luos_Loop();
            TEST_ASSERT_EQUAL(default_sc.App_3.app->id, default_sc.App_2.last_rx_msg.header.source);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_ReadMsg(void)
{
    NEW_TEST_CASE("Test Luos_ReadMsg assert conditions");
    {
        TRY
        {
            NEW_STEP("Try to missspecify the service pointer");
            msg_t msg;
            Luos_ReadMsg(NULL, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            NEW_STEP("Try to miss-specify the service pointer");
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, STATE_TYPE, "mycustom_service", revision);
            Luos_ReadMsg(service, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Luos_ReadMsg normal conditions");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, STATE_TYPE, "mycustom_service", revision);
            Luos_Detect(default_sc.App_1.app);
            do
            {
                Luos_Loop();
            } while (!Luos_IsDetected());

            NEW_STEP("Check end detection reception");
            msg_t rx_msg;
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));

            NEW_STEP("Check normal reception");
            msg_t msg;
            msg.header.target      = service->id;
            msg.header.target_mode = SERVICEIDACK;
            msg.header.cmd         = LUOS_LAST_RESERVED_CMD + 1;
            msg.header.size        = 0;
            TEST_ASSERT_EQUAL(SUCCEED, Luos_SendMsg(default_sc.App_3.app, &msg));
            Luos_Loop();
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
            TEST_ASSERT_EQUAL(msg.header.source, rx_msg.header.source);
            TEST_ASSERT_EQUAL(rx_msg.header.config, msg.header.config);

            NEW_STEP("Try receive but no message available");
            TEST_ASSERT_EQUAL(FAILED, Luos_ReadMsg(service, &rx_msg));

            NEW_STEP("Check timestamped reception");
            time_luos_t timestamp = TimeOD_TimeFrom_s(2);
            TEST_ASSERT_EQUAL(SUCCEED, Luos_SendTimestampMsg(default_sc.App_3.app, &msg, timestamp));
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
            TEST_ASSERT_EQUAL(msg.header.source, rx_msg.header.source);
            TEST_ASSERT_EQUAL(rx_msg.header.config, msg.header.config);
            time_luos_t rx_timestamp = Luos_GetMsgTimestamp(&rx_msg);
            TEST_ASSERT_EQUAL(TimeOD_TimeTo_s(timestamp), TimeOD_TimeTo_s(rx_timestamp));

            NEW_STEP("Try receive but no message available");
            TEST_ASSERT_EQUAL(FAILED, Luos_ReadMsg(service, &rx_msg));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_ReadFromService(void)
{

    NEW_TEST_CASE("Test Luos_ReadFromService assert conditions");
    {
        TRY
        {
            NEW_STEP("Try to missspecify the service pointer");
            msg_t msg;
            Luos_ReadFromService(NULL, 1, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            NEW_STEP("Try to miss-specify the service pointer");
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, STATE_TYPE, "mycustom_service", revision);
            Luos_ReadFromService(service, 1, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            NEW_STEP("Try to miss-specify the targeted service id");
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, STATE_TYPE, "mycustom_service", revision);
            msg_t msg;
            Luos_ReadFromService(service, 0, &msg);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Luos_ReadFromService normal conditions");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, STATE_TYPE, "mycustom_service", revision);
            Luos_Detect(default_sc.App_1.app);
            do
            {
                Luos_Loop();
            } while (!Luos_IsDetected());

            NEW_STEP("Check end detection reception");
            msg_t rx_msg;
            TEST_ASSERT_EQUAL(FAILED, Luos_ReadFromService(service, 2, &rx_msg));
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadFromService(service, 1, &rx_msg));

            NEW_STEP("Check normal reception");
            msg_t msg;
            msg.header.target      = service->id;
            msg.header.target_mode = SERVICEIDACK;
            msg.header.cmd         = LUOS_LAST_RESERVED_CMD + 1;
            msg.header.size        = 0;
            TEST_ASSERT_EQUAL(SUCCEED, Luos_SendMsg(default_sc.App_3.app, &msg));
            Luos_Loop();
            TEST_ASSERT_EQUAL(FAILED, Luos_ReadFromService(service, 1, &rx_msg));
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadFromService(service, 3, &rx_msg));
            TEST_ASSERT_EQUAL(msg.header.source, rx_msg.header.source);
            TEST_ASSERT_EQUAL(rx_msg.header.config, msg.header.config);

            NEW_STEP("Try receive but no message available");
            TEST_ASSERT_EQUAL(FAILED, Luos_ReadFromService(service, 3, &rx_msg));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_Send_ReceiveData()
{
    NEW_TEST_CASE("Test Luos_SendData assert condition");
    {
        revision_t revision   = {.major = 1, .minor = 0, .build = 0};
        service_t *service    = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
        uint32_t bin_data[64] = {0xDEADBEEF};
        msg_t msg;

        NEW_STEP("Try to send a void message argument");
        {
            TRY
            {
                Luos_SendData(service, 0, bin_data, 10);
            }
            TEST_ASSERT_TRUE(IS_ASSERT());
            END_TRY;
        }

        NEW_STEP("Try to send a void table argument");
        {
            TRY
            {
                Luos_SendData(service, &msg, NULL, 10);
            }
            TEST_ASSERT_TRUE(IS_ASSERT());
            END_TRY;
        }

        NEW_STEP("Try to send a shity size");
        {
            TRY
            {
                Luos_SendData(service, &msg, bin_data, 0);
            }
            TEST_ASSERT_TRUE(IS_ASSERT());
            END_TRY;
        }
    }
    NEW_TEST_CASE("Test Luos_ReceiveData assert condition");
    {
        revision_t revision   = {.major = 1, .minor = 0, .build = 0};
        service_t *service    = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
        uint32_t bin_data[64] = {0xDEADBEEF};
        msg_t msg;

        NEW_STEP("Try to send a void message argument");
        {
            TRY
            {
                Luos_ReceiveData(service, 0, bin_data);
            }
            TEST_ASSERT_TRUE(IS_ASSERT());
            END_TRY;
        }

        NEW_STEP("Try to send a void table argument");
        {
            TRY
            {
                Luos_ReceiveData(service, &msg, NULL);
            }
            TEST_ASSERT_TRUE(IS_ASSERT());
            END_TRY;
        }

        NEW_STEP("Try to send a shity service argument");
        {
            TRY
            {
                Luos_ReceiveData((service_t *)10, &msg, bin_data);
            }
            TEST_ASSERT_TRUE(IS_ASSERT());
            END_TRY;
        }

        NEW_STEP("Try to send a shity service pointer");
        {
            TRY
            {
                TEST_ASSERT_EQUAL(-1, Luos_ReceiveData(NULL, &msg, bin_data));
            }
            CATCH
            {
                TEST_ASSERT_TRUE(false);
            }
            END_TRY;
        }
    }

    NEW_TEST_CASE("Test the regular usage");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
            Luos_Detect(default_sc.App_1.app);
            do
            {
                Luos_Loop();
            } while (!Luos_IsDetected());
            msg_t msg;
            uint8_t tx_data[256] = {0};
            uint8_t rx_data[256] = {0};
            msg_t rx_msg;
            // Catch the end of detection
            Luos_ReadMsg(service, &rx_msg);

            NEW_STEP("Verify that the first message return 0 meaning message is not completely received");
            {
                // Set first message
                memset(tx_data, 0xAA, 256);
                msg.header.target      = service->id;
                msg.header.target_mode = SERVICEIDACK;
                msg.header.cmd         = LUOS_LAST_RESERVED_CMD + 1;
                msg.header.size        = 0;
                Luos_SendData(default_sc.App_1.app, &msg, tx_data, 256);
                TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
                TEST_ASSERT_EQUAL(256, rx_msg.header.size);
                TEST_ASSERT_EQUAL(0, Luos_ReceiveData(service, &rx_msg, rx_data));
            }

            NEW_STEP("Verify that the second message return 256 byte received");
            {
                TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
                TEST_ASSERT_EQUAL(128, rx_msg.header.size);
                TEST_ASSERT_EQUAL(256, Luos_ReceiveData(service, &rx_msg, rx_data));
            }

            NEW_STEP("Check if the data is OK");
            {
                for (int i = 0; i < 256; i++)
                {
                    TEST_ASSERT_EQUAL(rx_data[i], 0xAA);
                }
            }
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Try to break the message");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
            Luos_Detect(default_sc.App_1.app);
            do
            {
                Luos_Loop();
            } while (!Luos_IsDetected());
            msg_t msg;
            uint8_t tx_data[380] = {0};
            uint8_t rx_data[380] = {0};
            msg_t rx_msg;
            // Catch the end of detection
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));

            NEW_STEP("Check that the first message return 0 meaning message is not completely received");
            {
                // Set first message
                msg.header.target      = service->id;
                msg.header.target_mode = SERVICEIDACK;
                msg.header.cmd         = LUOS_LAST_RESERVED_CMD + 1;
                msg.header.size        = 0;
                Luos_SendData(default_sc.App_1.app, &msg, tx_data, 380);
                TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
                TEST_ASSERT_EQUAL(380, rx_msg.header.size);
                TEST_ASSERT_EQUAL(0, Luos_ReceiveData(service, &rx_msg, rx_data));
            }

            NEW_STEP("Remove the second message but receive the second one and check if the data manager reset");
            {
                TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
                TEST_ASSERT_EQUAL(252, rx_msg.header.size);
                TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
                TEST_ASSERT_EQUAL(124, rx_msg.header.size);
                TEST_ASSERT_EQUAL(-1, Luos_ReceiveData(service, &rx_msg, rx_data));
            }
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }

    NEW_TEST_CASE("Try to send a void service argument to reset the data reception");
    {
        TRY
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
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        END_TRY;
    }
}

void unittest_Luos_NbrAvailableMsg(void)
{

    NEW_TEST_CASE("Test the regular usage");
    {
        TRY
        {
            //  Init default scenario context
            Init_Context();
            revision_t revision = {.major = 1, .minor = 0, .build = 0};
            service_t *service  = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
            Luos_Detect(default_sc.App_1.app);
            do
            {
                Luos_Loop();
            } while (!Luos_IsDetected());
            msg_t msg;
            uint8_t tx_data[256] = {0};
            uint8_t rx_data[256] = {0};
            msg_t rx_msg;
            // Catch the end of detection
            TEST_ASSERT_EQUAL(1, Luos_NbrAvailableMsg());
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReadMsg(service, &rx_msg));
            TEST_ASSERT_EQUAL(0, Luos_NbrAvailableMsg());
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

    // Big data reception
    UNIT_TEST_RUN(unittest_Luos_GetVersion);
    UNIT_TEST_RUN(unittest_Luos_Send);
    UNIT_TEST_RUN(unittest_Luos_SendMsg);
    UNIT_TEST_RUN(unittest_Luos_SendTimestampMsg);
    UNIT_TEST_RUN(unittest_Luos_ReadMsg);
    UNIT_TEST_RUN(unittest_Luos_ReadFromService);
    UNIT_TEST_RUN(unittest_Luos_Send_ReceiveData);
    UNIT_TEST_RUN(unittest_Luos_NbrAvailableMsg);

    UNITY_END();
}
