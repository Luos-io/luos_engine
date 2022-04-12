#include "main.h"
#include <stdio.h>
#include <default_scenario.h>

extern default_scenario_t default_sc;
extern volatile uint8_t msg_buffer[MSG_BUFFER_SIZE];

static void MessageHandler(service_t *service, msg_t *msg)
{
    default_sc.App_1.last_rx_msg->header.config = msg->header.config;
    default_sc.App_1.last_rx_msg->header.size   = msg->header.size;
    for (uint16_t i = 0; i < default_sc.App_1.last_rx_msg->header.size; i++)
    {
        default_sc.App_1.last_rx_msg->data[i] = msg->data[i];
    }
}

void unittest_Timestamp()
{
    NEW_TEST_CASE("Timestamp measurement");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();
        //  Init variable
        time_luos_t event_a_timestamp;
        time_luos_t event_b_timestamp;

        NEW_STEP("Save events");
        // Save event A
        event_a_timestamp = Timestamp_now();
        // wait for 1.2 seconds
        uint32_t start_timer = Luos_GetSystick();
        while (Luos_GetSystick() - start_timer < 1200)
            ;
        // Save event B
        event_b_timestamp = Timestamp_now();

        // check the time elapsed between the two events
        time_luos_t time_elapsed = event_b_timestamp - event_a_timestamp;

        // Verify
        TEST_ASSERT_EQUAL(((TimeOD_TimeTo_s(time_elapsed) > TimeOD_TimeFrom_ms(1.15)) && (TimeOD_TimeTo_s(time_elapsed) < TimeOD_TimeFrom_ms(1.25))), true);

        NEW_STEP("Transmit timestamps");

        // Init scenario context
        Luos_ServicesClear();
        RoutingTB_Erase();
        Luos_Init();
        msg_t receive_msg;
        default_sc.App_1.last_rx_msg = &receive_msg;
        default_sc.App_1.tx_msg      = (msg_t *)(&msg_buffer[0]);
        Luos_Init();

        // Create services
        revision_t revision    = {.major = 1, .minor = 0, .build = 0};
        service_t *transmitter = Luos_CreateService(0, VOID_TYPE, "Dummy_App", revision);
        service_t *receiver    = Luos_CreateService(MessageHandler, VOID_TYPE, "Dummy_App", revision);

        // Launch detection
        Luos_Detect(transmitter);
        Luos_Loop();

        // Create message
        msg_t msg;
        msg.header.target      = receiver->ll_service->id;
        msg.header.target_mode = IDACK;
        msg.header.size        = 1;
        msg.header.cmd         = STATE_TYPE;
        msg.data[0]            = true;

        // Send the 1st message to receiver with both the 1st timestamp
        Luos_SendTimestampMsg(receiver, &msg, event_a_timestamp);
        Luos_Loop();
        // Get the message received
        msg_t *rx_msg;
        rx_msg = default_sc.App_1.last_rx_msg;
        // Get back the 1st timestamp
        time_luos_t rx_event_a_timestamp = Timestamp_GetTimestamp(rx_msg);

        // Send the 2nd message to receiver with both the 2nd timestamp
        msg.header.size = 1;
        Luos_SendTimestampMsg(receiver, &msg, event_b_timestamp);
        Luos_Loop();
        // Get the message received
        rx_msg = default_sc.App_1.last_rx_msg;
        // Get back the 2nd timestamp
        time_luos_t rx_event_b_timestamp = Timestamp_GetTimestamp(rx_msg);

        // check the time elapsed between the two events
        time_elapsed = rx_event_b_timestamp - rx_event_a_timestamp;
        // Verify
        TEST_ASSERT_EQUAL(((TimeOD_TimeTo_s(time_elapsed) > TimeOD_TimeFrom_ms(1.15)) && (TimeOD_TimeTo_s(time_elapsed) < TimeOD_TimeFrom_ms(1.25))), true);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    ASSERT_ACTIVATION(1);

    // Timestamp function
    UNIT_TEST_RUN(unittest_Timestamp);

    UNITY_END();
}
