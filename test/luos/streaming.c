#include <stdio.h>
#include <unistd.h>
#include "unit_test.h"
#include "default_scenario.h"

// **********************************************************************
// TEMPLATE FOR TESTING "Streaming_SendStreamingSize"
// TO BE COMPLETED
// **********************************************************************

extern default_scenario_t default_sc;

void unittest_Streaming_SendStreamingSize()
{
    NEW_TEST_CASE("ADD TEST CASE TITLE");
    {
        uint32_t max_size;

        // Init default scenario context
        Init_Context();

        // Init variable
        max_size = 1;

        // Add samples
        //Stream_AddAvailableSampleNB(default_sc.App_1.streamChannel, max_size);

        // Unit test : Call function Luos_SendStreamingSize
        NEW_STEP("ADD STEP TITLE");
        //Luos_SendStreamingSize(ADD PARAMETERS);
        //Luos_Loop();

        //--------------------------------------------------------------------
        // Verify
        //--------------------------------------------------------------------
        //TEST_ASSERT_EQUAL(max_size, default_sc.App_2.last_rx_msg->header.size);
    }
}
