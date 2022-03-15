#include "main.h"
#include <stdio.h>
#include <unistd.h>
#include <default_scenario.h>

#include "../Robus/src/timestamp.c"

extern default_scenario_t default_sc;

void unittest_Timestamp()
{
    NEW_TEST_CASE("Timestamp unit test");
    {
        Reset_Context();
        //  Init default scenario context
        Init_Context();
        //  Init variable
        timestamp_token_t event_a_timestamp;
        timestamp_token_t event_b_timestamp;
        event_a_timestamp.next = 0;
        event_b_timestamp.next = 0;

        bool dummy_event_a = true;
        bool dummy_event_b = true;
        bool test_result   = false;

        NEW_STEP("Tag events");
        // tag event A
        Timestamp_Tag(&event_a_timestamp, &dummy_event_a);
        // wait for 1.2 seconds
        uint32_t start_timer = Luos_GetSystick();
        while (Luos_GetSystick() - start_timer < 1200)
            ;
        // tag event B
        Timestamp_Tag(&event_b_timestamp, &dummy_event_b);

        // check the time elapsed between the two events
        int64_t event_a_time = Timestamp_GetTimeFromToken(&dummy_event_a);
        int64_t event_b_time = Timestamp_GetTimeFromToken(&dummy_event_b);
        int64_t time_elapsed = event_b_time - event_a_time;

        if ((time_elapsed > 1150000) && (time_elapsed < 1250000))
        {
            test_result = true;
        }
        // Verify
        TEST_ASSERT_EQUAL(test_result, true);
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
