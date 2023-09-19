#include <stdio.h>
#include "default_scenario.h"
#include "_timestamp.h"

extern volatile uint8_t msg_buffer[MSG_BUFFER_SIZE];
extern default_scenario_t default_sc;

void unittest_Luos_IsMsgTimstamped(void)
{
    NEW_TEST_CASE("Test Luos_IsMsgTimstamped assert conditions");
    {
        TRY
        {
            Luos_IsMsgTimstamped(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
}

void unittest_Luos_GetMsgTimestamp(void)
{
    NEW_TEST_CASE("Test Luos_GetMsgTimestamp assert conditions");
    {
        TRY
        {
            Luos_GetMsgTimestamp(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
}

void unittest_Timestamp_EncodeMsg(void)
{
    NEW_TEST_CASE("Test Timestamp_EncodeMsg assert conditions");
    {
        TRY
        {
            time_luos_t timestamp;
            Timestamp_EncodeMsg(NULL, timestamp);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
}

void unittest_Timestamp_ConvertToLatency(void)
{
    NEW_TEST_CASE("Test Timestamp_ConvertToLatency assert conditions");
    {
        TRY
        {
            Timestamp_ConvertToLatency(NULL);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
}

void unittest_Timestamp_ConvertToDate(void)
{
    NEW_TEST_CASE("Test Timestamp_ConvertToDate assert conditions");
    {
        TRY
        {
            uint64_t reception_date;
            Timestamp_ConvertToDate(NULL, reception_date);
        }
        TEST_ASSERT_TRUE(IS_ASSERT());
        END_TRY;
    }
}

void unittest_Timestamp()
{
    NEW_TEST_CASE("Timestamp measurement");
    {
        NEW_STEP("Save events");
        //  Init default scenario context
        Init_Context();
        Luos_Loop();
        //  Init variable
        time_luos_t event_a_timestamp;
        time_luos_t event_b_timestamp;

        // Save event A
        event_a_timestamp = Luos_Timestamp();
        // wait for 1.2 seconds
        uint32_t start_timer = Luos_GetSystick();
        while (Luos_GetSystick() - start_timer < 1200)
            ;
        // Save event B
        event_b_timestamp = Luos_Timestamp();
        // check the time elapsed between the two events
        time_luos_t time_elapsed = TimeOD_TimeFrom_s(TimeOD_TimeTo_s(event_b_timestamp) - TimeOD_TimeTo_s(event_a_timestamp));

        // Verify
        TEST_ASSERT_EQUAL(((TimeOD_TimeTo_ms(time_elapsed) > 1.15) && (TimeOD_TimeTo_ms(time_elapsed) < 1.25)), true);

        NEW_STEP("Transmit timestamps");
        // Init scenario context
        // Init default scenario context
        Init_Context();
        Luos_Loop();

        // Create message
        msg_t msg;
        msg.header.target      = 2;
        msg.header.target_mode = SERVICEIDACK;
        msg.header.size        = 1;
        msg.header.cmd         = IO_STATE;
        msg.data[0]            = true;

        // Send the 1st message to receiver with the 1st timestamp
        TEST_ASSERT_EQUAL(SUCCEED, Luos_SendTimestampMsg(default_sc.App_1.app, &msg, event_a_timestamp));
        Luos_Loop();
        // Get the message received
        msg_t *rx_msg;
        rx_msg = &default_sc.App_2.last_rx_msg;
        uint64_t low_level_rx_timestamp;
        // Check the message size
        TEST_ASSERT_EQUAL(msg.header.size, rx_msg->header.size);
        // Check that the message is timestamped
        TEST_ASSERT_EQUAL(true, Luos_IsMsgTimstamped(rx_msg));
        // Get back the 1st timestamp
        time_luos_t rx_event_a_timestamp = Luos_GetMsgTimestamp(rx_msg);
        TEST_ASSERT_FALSE(IS_ASSERT());

        // Send the 2nd message to receiver with the 2nd timestamp
        msg.header.size = 2;
        TEST_ASSERT_EQUAL(SUCCEED, Luos_SendTimestampMsg(default_sc.App_1.app, &msg, event_b_timestamp));
        Luos_Loop();
        // Get the message received
        rx_msg = &default_sc.App_2.last_rx_msg;
        // Check the message size
        TEST_ASSERT_EQUAL(msg.header.size, rx_msg->header.size);
        // Check that the message is timestamped
        TEST_ASSERT_EQUAL(true, Luos_IsMsgTimstamped(rx_msg));
        // Get back the 2nd timestamp
        time_luos_t rx_event_b_timestamp = Luos_GetMsgTimestamp(rx_msg);
        TEST_ASSERT_FALSE(IS_ASSERT());

        // check the time elapsed between the two events
        time_luos_t time_elapsed2 = TimeOD_TimeFrom_s(TimeOD_TimeTo_s(rx_event_b_timestamp) - TimeOD_TimeTo_s(rx_event_a_timestamp));
        // Verify
        TEST_ASSERT_EQUAL(TimeOD_TimeTo_ms(time_elapsed2), TimeOD_TimeTo_ms(time_elapsed));
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Timestamp function
    UNIT_TEST_RUN(unittest_Luos_IsMsgTimstamped);
    UNIT_TEST_RUN(unittest_Luos_GetMsgTimestamp);
    UNIT_TEST_RUN(unittest_Timestamp_EncodeMsg);
    UNIT_TEST_RUN(unittest_Timestamp_ConvertToLatency);
    UNIT_TEST_RUN(unittest_Timestamp_ConvertToDate);
    UNIT_TEST_RUN(unittest_Timestamp);

    UNITY_END();
}
