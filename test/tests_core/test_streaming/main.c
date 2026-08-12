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

void unittest_Streaming_PutSample_capacity(void)
{
    // Every other test uses a 1 byte data size, where a sample count and a byte
    // count are the same number. Use floats so the two units differ. The ring
    // holds 16 samples, so 64 bytes, and guard sits right behind it.
    struct
    {
        float ring[16];
        uint8_t guard[64];
    } mem;
    float data[32];
    streaming_channel_t channel = {0};
    for (uint32_t i = 0; i < 32; i++)
    {
        data[i] = (float)i;
    }

    NEW_TEST_CASE("Test Streaming_PutSample capacity is counted in samples");
    {
        TRY
        {
            channel = Streaming_CreateChannel(mem.ring, 16, sizeof(float));
            // 17 samples do not fit in a 16 sample channel, but they do fit in
            // its 64 byte span.
            Streaming_PutSample(&channel, data, 17);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }

    NEW_TEST_CASE("Test Streaming_PutSample never writes past the ring buffer");
    {
        memset(mem.guard, 0xAA, sizeof(mem.guard));
        TRY
        {
            channel = Streaming_CreateChannel(mem.ring, 16, sizeof(float));
            // Sit on the last sample slot, as a channel that has been running
            // for a while does. The wrap-around branch then has a single sample
            // of room before the end and dumps the whole rest at the start of
            // the ring. 32 samples is what Luos_ReceiveStreaming hands over for
            // a full MAX_DATA_MSG_SIZE message on a 4 byte channel.
            channel.data_ptr   = (void *)((uintptr_t)channel.ring_buffer + (15 * sizeof(float)));
            channel.sample_ptr = channel.data_ptr;
            Streaming_PutSample(&channel, data, 32);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
        for (uint32_t i = 0; i < sizeof(mem.guard); i++)
        {
            TEST_ASSERT_EQUAL_HEX8(0xAA, mem.guard[i]);
        }
    }

    NEW_TEST_CASE("Test Streaming_PutSample still accepts a full channel");
    {
        TRY
        {
            channel = Streaming_CreateChannel(mem.ring, 16, sizeof(float));
            TEST_ASSERT_EQUAL(15, Streaming_PutSample(&channel, data, 15));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        TRY
        {
            channel = Streaming_CreateChannel(mem.ring, 16, sizeof(float));
            Streaming_PutSample(&channel, data, 16);
        }
        TEST_ASSERT_FALSE(IS_ASSERT());
        END_TRY;
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

void unittest_Luos_ReceiveStreaming_oversized(void)
{
    // A message big enough to overflow a small channel, as a remote node can
    // send at any time. The channel holds 16 floats, a full MAX_DATA_MSG_SIZE
    // message carries 32 of them.
    struct
    {
        float ring[16];
        uint8_t guard[64];
    } mem;
    streaming_channel_t channel = {0};
    Init_Context();

    NEW_TEST_CASE("Test Luos_ReceiveStreaming drops a chunk the channel cannot hold");
    {
        msg_t msg;
        memset(mem.guard, 0xAA, sizeof(mem.guard));
        msg.header.cmd  = IO_STATE;
        msg.header.size = MAX_DATA_MSG_SIZE;
        memset(msg.data, 0x55, MAX_DATA_MSG_SIZE);
        TRY
        {
            channel = Streaming_CreateChannel(mem.ring, 16, sizeof(float));
            TEST_ASSERT_EQUAL(FAILED, Luos_ReceiveStreaming(default_sc.App_1.app, &msg, &channel));
            TEST_ASSERT_EQUAL(0, Streaming_GetAvailableSampleNB(&channel));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
        for (uint32_t i = 0; i < sizeof(mem.guard); i++)
        {
            TEST_ASSERT_EQUAL_HEX8(0xAA, mem.guard[i]);
        }
    }

    NEW_TEST_CASE("Test Luos_ReceiveStreaming drops a chunk smaller than one sample");
    {
        msg_t msg;
        msg.header.cmd  = IO_STATE;
        msg.header.size = 2;
        TRY
        {
            channel = Streaming_CreateChannel(mem.ring, 16, sizeof(float));
            TEST_ASSERT_EQUAL(FAILED, Luos_ReceiveStreaming(default_sc.App_1.app, &msg, &channel));
            TEST_ASSERT_EQUAL(0, Streaming_GetAvailableSampleNB(&channel));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }

    NEW_TEST_CASE("Test Luos_ReceiveStreaming still accepts a chunk that fits");
    {
        msg_t msg;
        msg.header.cmd  = IO_STATE;
        msg.header.size = 8 * sizeof(float);
        TRY
        {
            channel = Streaming_CreateChannel(mem.ring, 16, sizeof(float));
            TEST_ASSERT_EQUAL(SUCCEED, Luos_ReceiveStreaming(default_sc.App_1.app, &msg, &channel));
            TEST_ASSERT_EQUAL(8, Streaming_GetAvailableSampleNB(&channel));
        }
        CATCH
        {
            TEST_ASSERT_TRUE(false);
        }
    }
}

void unittest_Luos_Streaming_roundtrip_4byte_samples(void)
{
    // header.size is a byte count on the wire. Send a data_size > 1 channel
    // through it and check nothing is lost. 40 float samples are 160 bytes, more
    // than one MAX_DATA_MSG_SIZE message holds, so the multi chunk path is
    // covered as well.
    static float txring[64];
    static float rxring[64];
    streaming_channel_t tx = {0};
    Init_Context();

    NEW_TEST_CASE("Test streaming round trip on a 4 byte data size");
    {
        revision_t revision = {.major = 1, .minor = 0, .build = 0};
        service_t *service  = Luos_CreateService(MessageHandler, VOID_TYPE, "Float_App", revision);
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
            tx        = Streaming_CreateChannel(txring, 64, sizeof(float));
            rxchannel = Streaming_CreateChannel(rxring, 64, sizeof(float));
            for (uint32_t i = 0; i < 40; i++)
            {
                txring[i] = (float)(100 + i);
            }
            TEST_ASSERT_EQUAL(40, Streaming_AddAvailableSampleNB(&tx, 40));
            Luos_SendStreamingSize(service, &msg, &tx, 40);
            Luos_Loop();
            TEST_ASSERT_EQUAL(40, Streaming_GetAvailableSampleNB(&rxchannel));
            for (uint32_t i = 0; i < 40; i++)
            {
                TEST_ASSERT_EQUAL_FLOAT((float)(100 + i), rxring[i]);
            }
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
    UNIT_TEST_RUN(unittest_Streaming_PutSample_capacity);
    UNIT_TEST_RUN(unittest_Streaming_GetSample);
    UNIT_TEST_RUN(unittest_Streaming_GetAvailableSampleNB);
    UNIT_TEST_RUN(unittest_Streaming_GetAvailableSampleNBUntilEndBuffer);
    UNIT_TEST_RUN(unittest_Streaming_AddAvailableSampleNB);
    UNIT_TEST_RUN(unittest_Streaming_RmvAvailableSampleNB);
    UNIT_TEST_RUN(unittest_Luos_Send_and_receive_Streaming);
    UNIT_TEST_RUN(unittest_Luos_ReceiveStreaming_oversized);
    UNIT_TEST_RUN(unittest_Luos_Streaming_roundtrip_4byte_samples);

    UNITY_END();
}
