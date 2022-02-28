#include <stdio.h>
#include "main.h"
#include "unit_test.h"
#include "scenario/scenario_template.h"

void unittest_TEMPLATE(void)
{
    NEW_TEST_CASE("TEST SOMETHING");
    {

        // Init default dummy context
        //----------------------------
        Scenario_template_Context_Init(DETECTION);

        search_result_t result;
        RTFilter_Reset(&result);
        printf("Number of services after init :  %d\n", result.result_nbr);

        // Test step
        // **************************************************************
        NEW_STEP("Explain the step : this step checks something");

        // Init context
        // -------------
        // eg : uint8_t a,b = 0;

        // Call the function to test
        // --------------------------
        // eg : b = my function(a);

        // If needed, call  "Luos_loop();""
        // -------------------------------

        // Verify
        // --------
        // eg : TEST_ASSERT_EQUAL(10, b);

        // Another test steps
        // **************************************************************
        NEW_STEP("Explain the step : this step checks something else");
        // etc...
    }

    NEW_TEST_CASE("ANOTHER TEST CASE");
    {
        // etc...
    }
}
