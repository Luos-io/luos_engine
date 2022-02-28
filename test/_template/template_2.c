#include <stdio.h>
#include "main.h"
#include "unit_test.h"
#include "scenario/scenario_template.h"

#define TEST_TIMEOUT 10000 //ms

void unittest_TEMPLATE_2(void)
{
    NEW_TEST_CASE("Template example");
    {
        uint32_t start_time;
        int is_finished = 0;

        // Init default dummy context : no detection is asked
        //---------------------------------------------------
        Scenario_template_Context_Init(NO_DETECTION);

        // Template scenario : check services are detected
        //---------------------------------------------------
        NEW_STEP("Check there are exactly 3 services detected\n");
        start_time = Luos_GetSystick();
        while (Luos_GetSystick() - start_time < TEST_TIMEOUT)
        {
            //printf("\n\n[Current Time]     %lu ms\n", Luos_GetSystick());
            Luos_Loop();
            Scenario_template_Loop(&is_finished);

            //
            // Here you should add the function you have to test
            //

            // In this example, test is stopped after TEST_TIMEOUT
            // or when App_Detection_Loop has finished its job
            if (is_finished)
            {
                break;
            }
        }

        //--------------------------------------------------------------------
        // Verify
        //--------------------------------------------------------------------
        search_result_t result;
        RTFilter_Reset(&result);
        printf("Number of services :  %d\n", result.result_nbr);
        // Test is OK if there are 3 activated services.
        TEST_ASSERT_EQUAL(DUMMY_SERVICE_NUMBER, result.result_nbr);
    }
}
