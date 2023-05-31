#include "context.h"
#include "pub_sub.h"
#include <stdio.h>
#include "unit_test.h"
#include <default_scenario.h>
#include "filter.h"

extern default_scenario_t default_sc;
streaming_channel_t rxchannel = {0};

static void MessageHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == IO_STATE)
    {
        Luos_ReceiveStreaming(service, msg, &rxchannel);
    }
}

void unittest_Streaming_CreateChannel(void)
{
    uint8_t buffer[100];
    NEW_TEST_CASE("Test Streaming_CreateChannel assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_CreateChannel(NULL, 100, 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            //  Test assert conditions
            Streaming_CreateChannel(buffer, 0, 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        TRY
        {
            //  Test assert conditions
            Streaming_CreateChannel(buffer, 100, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test Streaming_CreateChannel");
    {
        TRY
        {
            streaming_channel_t channel = {0};
            channel                     = Streaming_CreateChannel(buffer, 100, 1);
            TEST_ASSERT_EQUAL(buffer, channel.data_ptr);
            TEST_ASSERT_EQUAL(buffer, channel.ring_buffer);
            TEST_ASSERT_EQUAL(1, channel.data_size);
            TEST_ASSERT_EQUAL(100, channel.end_ring_buffer - channel.ring_buffer);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Streaming_ResetChannel(void)
{
    uint8_t buffer[100];
    NEW_TEST_CASE("Test Streaming_ResetChannel assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_ResetChannel(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test Streaming_ResetChannel");
    {
        TRY
        {
            streaming_channel_t channel = {0};
            channel                     = Streaming_CreateChannel(buffer, 100, 1);
            channel.data_ptr            = buffer + 10;
            channel.sample_ptr          = buffer + 30;
            Streaming_ResetChannel(&channel);
            TEST_ASSERT_EQUAL(buffer, channel.data_ptr);
            TEST_ASSERT_EQUAL(buffer, channel.ring_buffer);
            TEST_ASSERT_EQUAL(buffer, channel.sample_ptr);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Streaming_PutSample(void)
{
    uint8_t buffer[100];
    streaming_channel_t channel = {0};
    channel                     = Streaming_CreateChannel(buffer, 100, 1);
    NEW_TEST_CASE("Test Streaming_PutSample assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_PutSample(NULL, buffer, 10);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_PutSample(&channel, NULL, 10);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_PutSample(&channel, buffer, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_PutSample(&channel, buffer, 101);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_PutSample(&channel, buffer, 100);
        }
        TEST_ASSERT_FALSE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test Streaming_PutSample simple case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            TEST_ASSERT_EQUAL(1, Streaming_PutSample(&channel, data, 1));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.sample_ptr);
            TEST_ASSERT_EQUAL(10, *(uint8_t *)(channel.sample_ptr));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(2, Streaming_PutSample(&channel, data + 1, 1));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 2, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.sample_ptr);
            TEST_ASSERT_EQUAL(10, *(uint8_t *)(channel.sample_ptr));
            TEST_ASSERT_EQUAL(20, *(uint8_t *)(channel.sample_ptr + 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(4, Streaming_PutSample(&channel, data + 1, 2));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 4, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.sample_ptr);
            TEST_ASSERT_EQUAL(10, *(uint8_t *)(channel.sample_ptr));
            TEST_ASSERT_EQUAL(20, *(uint8_t *)(channel.sample_ptr + 1));
            TEST_ASSERT_EQUAL(20, *(uint8_t *)(channel.sample_ptr + 2));
            TEST_ASSERT_EQUAL(30, *(uint8_t *)(channel.sample_ptr + 3));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Test Streaming_PutSample buffer loop case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(3, Streaming_PutSample(&channel, data, 3));
            TEST_ASSERT_EQUAL(30, buffer[0]);
            TEST_ASSERT_EQUAL(10, buffer[98]);
            TEST_ASSERT_EQUAL(20, buffer[99]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Streaming_GetSample(void)
{
    uint8_t buffer[100];
    streaming_channel_t channel = {0};
    channel                     = Streaming_CreateChannel(buffer, 100, 1);
    NEW_TEST_CASE("Test Streaming_GetSample assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_GetSample(NULL, buffer, 10);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_GetSample(&channel, NULL, 10);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_GetSample(&channel, buffer, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Streaming_GetSample simple case");
    {
        uint8_t data[3] = {10, 20, 30};
        uint8_t result[3];
        TRY
        {
            TEST_ASSERT_EQUAL(1, Streaming_PutSample(&channel, data, 1));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.sample_ptr);
            TEST_ASSERT_EQUAL(10, *(uint8_t *)(channel.sample_ptr));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_GetSample(&channel, result, 1));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.sample_ptr);
            TEST_ASSERT_EQUAL(10, result[0]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(1, Streaming_PutSample(&channel, data + 1, 1));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 2, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.sample_ptr);
            TEST_ASSERT_EQUAL(20, *(uint8_t *)(channel.sample_ptr));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_GetSample(&channel, result, 1));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 2, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 2, channel.sample_ptr);
            TEST_ASSERT_EQUAL(20, result[0]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(2, Streaming_PutSample(&channel, data + 1, 2));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 4, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 2, channel.sample_ptr);
            TEST_ASSERT_EQUAL(20, *(uint8_t *)(channel.sample_ptr));
            TEST_ASSERT_EQUAL(30, *(uint8_t *)(channel.sample_ptr + 1));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_GetSample(&channel, result, 2));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 4, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 4, channel.sample_ptr);
            TEST_ASSERT_EQUAL(20, result[0]);
            TEST_ASSERT_EQUAL(30, result[1]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
    NEW_TEST_CASE("Test Streaming_GetSample buffer loop case");
    {
        uint8_t data[3] = {10, 20, 30};
        uint8_t result[2];
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(3, Streaming_PutSample(&channel, data, 3));
            TEST_ASSERT_EQUAL(30, buffer[0]);
            TEST_ASSERT_EQUAL(10, buffer[98]);
            TEST_ASSERT_EQUAL(20, buffer[99]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_GetSample(&channel, result, 3));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.sample_ptr);
            TEST_ASSERT_EQUAL(10, result[0]);
            TEST_ASSERT_EQUAL(20, result[1]);
            TEST_ASSERT_EQUAL(30, result[2]);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
    NEW_TEST_CASE("Test Streaming_GetSample no sample available");
    {
        uint8_t data[3] = {10, 20, 30};
        uint8_t result[2];
        TRY
        {
            Streaming_ResetChannel(&channel);
            TEST_ASSERT_EQUAL(0, Streaming_GetSample(&channel, result, 3));
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.data_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.sample_ptr);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Streaming_GetAvailableSampleNB(void)
{
    uint8_t buffer[100];
    streaming_channel_t channel = {0};
    channel                     = Streaming_CreateChannel(buffer, 100, 1);
    NEW_TEST_CASE("Test Streaming_GetAvailableSampleNB assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_GetAvailableSampleNB(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Streaming_GetAvailableSampleNB simple case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_GetAvailableSampleNB(&channel));
            TEST_ASSERT_EQUAL(1, Streaming_PutSample(&channel, data, 1));
            TEST_ASSERT_EQUAL(1, Streaming_GetAvailableSampleNB(&channel));
            TEST_ASSERT_EQUAL(2, Streaming_PutSample(&channel, data + 1, 1));
            TEST_ASSERT_EQUAL(2, Streaming_GetAvailableSampleNB(&channel));
            TEST_ASSERT_EQUAL(4, Streaming_PutSample(&channel, data + 1, 2));
            TEST_ASSERT_EQUAL(4, Streaming_GetAvailableSampleNB(&channel));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
    NEW_TEST_CASE("Test Streaming_GetAvailableSampleNB buffer loop case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(3, Streaming_PutSample(&channel, data, 3));
            TEST_ASSERT_EQUAL(3, Streaming_GetAvailableSampleNB(&channel));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Streaming_GetAvailableSampleNBUntilEndBuffer(void)
{
    uint8_t buffer[100];
    streaming_channel_t channel = {0};
    channel                     = Streaming_CreateChannel(buffer, 100, 1);
    NEW_TEST_CASE("Test Streaming_GetAvailableSampleNBUntilEndBuffer assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_GetAvailableSampleNBUntilEndBuffer(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Streaming_GetAvailableSampleNBUntilEndBuffer simple case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_GetAvailableSampleNBUntilEndBuffer(&channel));
            TEST_ASSERT_EQUAL(1, Streaming_PutSample(&channel, data, 1));
            TEST_ASSERT_EQUAL(1, Streaming_GetAvailableSampleNBUntilEndBuffer(&channel));
            TEST_ASSERT_EQUAL(2, Streaming_PutSample(&channel, data + 1, 1));
            TEST_ASSERT_EQUAL(2, Streaming_GetAvailableSampleNBUntilEndBuffer(&channel));
            TEST_ASSERT_EQUAL(4, Streaming_PutSample(&channel, data + 1, 2));
            TEST_ASSERT_EQUAL(4, Streaming_GetAvailableSampleNBUntilEndBuffer(&channel));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
    NEW_TEST_CASE("Test Streaming_GetAvailableSampleNBUntilEndBuffer buffer loop case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(3, Streaming_PutSample(&channel, data, 3));
            TEST_ASSERT_EQUAL(2, Streaming_GetAvailableSampleNBUntilEndBuffer(&channel));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Streaming_AddAvailableSampleNB(void)
{
    uint8_t buffer[100];
    streaming_channel_t channel = {0};
    channel                     = Streaming_CreateChannel(buffer, 100, 1);
    NEW_TEST_CASE("Test Streaming_AddAvailableSampleNB assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_AddAvailableSampleNB(NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_AddAvailableSampleNB(&channel, 101);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Streaming_AddAvailableSampleNB simple case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_AddAvailableSampleNB(&channel, 0));
            TEST_ASSERT_EQUAL(1, Streaming_AddAvailableSampleNB(&channel, 1));
            TEST_ASSERT_EQUAL(2, Streaming_AddAvailableSampleNB(&channel, 1));
            TEST_ASSERT_EQUAL(4, Streaming_AddAvailableSampleNB(&channel, 2));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
    NEW_TEST_CASE("Test Streaming_AddAvailableSampleNB buffer loop case");
    {
        uint8_t buffer[100];
        streaming_channel_t channel = {0};
        channel                     = Streaming_CreateChannel(buffer, 100, 1);
        uint8_t data[3]             = {10, 20, 30};
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(2, Streaming_AddAvailableSampleNB(&channel, 2));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 98, channel.sample_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.data_ptr);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(3, Streaming_AddAvailableSampleNB(&channel, 3));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 98, channel.sample_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.data_ptr);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Streaming_RmvAvailableSampleNB(void)
{
    uint8_t buffer[100];
    streaming_channel_t channel = {0};
    channel                     = Streaming_CreateChannel(buffer, 100, 1);
    NEW_TEST_CASE("Test Streaming_RmvAvailableSampleNB assert conditions");
    {
        TRY
        {
            //  Test assert conditions
            Streaming_RmvAvailableSampleNB(NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Streaming_RmvAvailableSampleNB(&channel, 1);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test Streaming_RmvAvailableSampleNB simple case");
    {
        uint8_t data[3] = {10, 20, 30};
        TRY
        {
            TEST_ASSERT_EQUAL(0, Streaming_RmvAvailableSampleNB(&channel, 0));
            TEST_ASSERT_EQUAL(1, Streaming_AddAvailableSampleNB(&channel, 1));
            TEST_ASSERT_EQUAL(0, Streaming_RmvAvailableSampleNB(&channel, 1));
            TEST_ASSERT_EQUAL(2, Streaming_AddAvailableSampleNB(&channel, 2));
            TEST_ASSERT_EQUAL(0, Streaming_RmvAvailableSampleNB(&channel, 2));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
    NEW_TEST_CASE("Test Streaming_RmvAvailableSampleNB buffer loop case");
    {
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(3, Streaming_AddAvailableSampleNB(&channel, 3));
            TEST_ASSERT_EQUAL(0, Streaming_RmvAvailableSampleNB(&channel, 3));
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.sample_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer + 1, channel.data_ptr);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            Streaming_ResetChannel(&channel);
            channel.sample_ptr = channel.ring_buffer + 98;
            channel.data_ptr   = channel.ring_buffer + 98;
            TEST_ASSERT_EQUAL(2, Streaming_AddAvailableSampleNB(&channel, 2));
            TEST_ASSERT_EQUAL(0, Streaming_RmvAvailableSampleNB(&channel, 2));
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.sample_ptr);
            TEST_ASSERT_EQUAL(channel.ring_buffer, channel.data_ptr);
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Luos_Send_and_receive_Streaming(void)
{
    uint8_t buffer[200];
    uint8_t rxbuffer[200];
    streaming_channel_t channel = {0};
    channel                     = Streaming_CreateChannel(buffer, 200, 1);
    rxchannel                   = Streaming_CreateChannel(buffer, 200, 1);
    Init_Context();
    NEW_TEST_CASE("Test Luos_SendStreamingSize assert conditions");
    {
        msg_t msg;
        TRY
        {
            //  Test assert conditions
            Luos_SendStreamingSize(NULL, &msg, &channel, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Luos_SendStreamingSize(default_sc.App_1.app, NULL, &channel, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Luos_SendStreamingSize(default_sc.App_1.app, &msg, NULL, 0);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;

        TRY
        {
            //  Test assert conditions
            Luos_ReceiveStreaming(NULL, &msg, &channel);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Luos_ReceiveStreaming(default_sc.App_1.app, NULL, &channel);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        TRY
        {
            //  Test assert conditions
            Luos_ReceiveStreaming(default_sc.App_1.app, &msg, NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
    NEW_TEST_CASE("Test send receive streaming");
    {
        revision_t revision = {.major = 1, .minor = 0, .build = 0};
        service_t *service  = Luos_CreateService(MessageHandler, VOID_TYPE, "Test_App", revision);
        // Detection
        Luos_Detect(service);
        do
        {
            Luos_Loop();
        } while (!Luos_IsDetected());
        msg_t msg;
        msg.header.target      = service->id;
        msg.header.target_mode = SERVICEIDACK;
        msg.header.cmd         = IO_STATE;
        TRY
        {
            TEST_ASSERT_EQUAL(1, Streaming_AddAvailableSampleNB(&channel, 1));
            Luos_SendStreamingSize(default_sc.App_1.app, &msg, &channel, 1);
            Luos_Loop();
            TEST_ASSERT_EQUAL(1, Streaming_GetAvailableSampleNB(&rxchannel));

            TEST_ASSERT_EQUAL(2, Streaming_AddAvailableSampleNB(&channel, 2));
            Luos_SendStreamingSize(default_sc.App_1.app, &msg, &channel, 2);
            Luos_Loop();
            TEST_ASSERT_EQUAL(3, Streaming_GetAvailableSampleNB(&rxchannel));

            TEST_ASSERT_EQUAL(150, Streaming_AddAvailableSampleNB(&channel, 150));
            Luos_SendStreamingSize(default_sc.App_1.app, &msg, &channel, 150);
            Luos_Loop();
            TEST_ASSERT_EQUAL(153, Streaming_GetAvailableSampleNB(&rxchannel));

            // Check buffer loop
            Streaming_RmvAvailableSampleNB(&rxchannel, 153);
            TEST_ASSERT_EQUAL(150, Streaming_AddAvailableSampleNB(&channel, 150));
            Luos_SendStreamingSize(default_sc.App_1.app, &msg, &channel, 150);
            Luos_Loop();
            TEST_ASSERT_EQUAL(150, Streaming_GetAvailableSampleNB(&rxchannel));
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

    UNIT_TEST_RUN(unittest_Streaming_CreateChannel);
    UNIT_TEST_RUN(unittest_Streaming_ResetChannel);
    UNIT_TEST_RUN(unittest_Streaming_PutSample);
    UNIT_TEST_RUN(unittest_Streaming_GetSample);
    UNIT_TEST_RUN(unittest_Streaming_GetAvailableSampleNB);
    UNIT_TEST_RUN(unittest_Streaming_GetAvailableSampleNBUntilEndBuffer);
    UNIT_TEST_RUN(unittest_Streaming_AddAvailableSampleNB);
    UNIT_TEST_RUN(unittest_Streaming_RmvAvailableSampleNB);
    UNIT_TEST_RUN(unittest_Luos_Send_and_receive_Streaming);

    UNITY_END();
}
