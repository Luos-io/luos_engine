#include "main.h"
#include <stdio.h>
#include <default_scenario.h>

extern volatile uint8_t msg_buffer[MSG_BUFFER_SIZE];
extern default_scenario_t default_sc;

static void MessageHandler(service_t *service, msg_t *msg)
{
    default_sc.App_1.last_rx_msg.header.config = msg->header.config;
    default_sc.App_1.last_rx_msg.header.size   = msg->header.size;
    for (uint16_t i = 0; i < default_sc.App_1.last_rx_msg.header.size; i++)
    {
        default_sc.App_1.last_rx_msg.data[i] = msg->data[i];
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
        //  Init default scenario context
        Init_Context();
        Luos_Loop();

        // Create message
        msg_t msg;
        msg.header.target      = 2;
        msg.header.target_mode = SERVICEIDACK;
        msg.header.size        = 1;
        msg.header.cmd         = IO_STATE;
        msg.data[0]            = true;

        // Send the 1st message to receiver with both the 1st timestamp
        TEST_ASSERT_EQUAL(SUCCEED, Luos_SendTimestampMsg(default_sc.App_1.app, &msg, event_a_timestamp));
        Luos_Loop();

        // Get the message received
        msg_t *rx_msg;
        rx_msg = &default_sc.App_2.last_rx_msg;
        uint64_t low_level_rx_timestamp;
        // Get back the 1st timestamp
        time_luos_t rx_event_a_timestamp = Luos_GetMsgTimestamp(rx_msg);
        // Send the 2nd message to receiver with both the 2nd timestamp
        msg.header.size = 1;
        TEST_ASSERT_EQUAL(SUCCEED, Luos_SendTimestampMsg(default_sc.App_1.app, &msg, event_b_timestamp));
        Luos_Loop();

        // Get the message received
        rx_msg = &default_sc.App_2.last_rx_msg;
        // Get back the 2nd timestamp
        time_luos_t rx_event_b_timestamp = Luos_GetMsgTimestamp(rx_msg);

        // check the time elapsed between the two events
        time_elapsed = TimeOD_TimeFrom_s(TimeOD_TimeTo_s(rx_event_b_timestamp) - TimeOD_TimeTo_s(rx_event_a_timestamp));
        // Verify
        TEST_ASSERT_EQUAL(((TimeOD_TimeTo_ms(time_elapsed) > 1.15) && (TimeOD_TimeTo_ms(time_elapsed) < 1.25)), true);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Timestamp function
    UNIT_TEST_RUN(unittest_Timestamp);

    UNITY_END();
}
