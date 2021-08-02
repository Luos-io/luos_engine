/******************************************************************************
 * @file unit_test.c
 * @brief toolbox for unit testing
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "unit_test.h"
#include <string.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern ut_luos_assert_t ut_assert;
uint16_t test_case_number;
uint16_t step_number;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Launch a unit test
 * @param None
 * @return None
 ******************************************************************************/
void UNIT_TEST_RUN(void (*function)())
{
    test_case_number = 0;
    RUN_TEST(function);
}

/******************************************************************************
 * @brief Start a new test case
 * @param None
 * @return None
 ******************************************************************************/
void NEW_TEST_CASE(char *title)
{
    step_number = 0;
    test_case_number++;

    if (test_case_number == 1)
    {
        printf("\n");
    }
    printf("------------------------------------------------------------------------\n");
    printf("Test Case %d :\n", test_case_number);
    printf("------------------------------------------------------------------------\n");

    for (uint16_t i = 0; i < strlen(title); i++)
    {
        printf("%c", title[i]);
    }
    printf("\n");
}

/******************************************************************************
 * @brief Print current step to verify
 * @param None
 * @return None
 ******************************************************************************/
void NEW_STEP()
{
    printf("\t---> Check step %d\n", ++step_number);
}

/******************************************************************************
 * @brief Check if an assert has occured
 * @param None
 * @return True boolean if an assert has occured
 ******************************************************************************/
bool IS_ASSERT(void)
{
    if ((ut_assert.enable == 1) && (ut_assert.state == 1))
    {
        ut_assert.state = 0;

#ifdef UNIT_TEST_DEBUG
        uint32_t line = (ut_assert.msg.data[0]);
        if (ut_assert.line_size > 3)
        {
            line += (ut_assert.msg.data[2] << 4) + (ut_assert.msg.data[1] << 8) + (ut_assert.msg.data[3] << 12);
        }
        else if (ut_assert.line_size > 2)
        {
            line += (ut_assert.msg.data[2] << 4) + (ut_assert.msg.data[1] << 8);
        }
        else if (ut_assert.line_size > 1)
        {
            line += (ut_assert.msg.data[2] << 4);
        }

        printf("\n*\t[INFO] Assert message received\t");
        printf("- Line : ");
        printf("%d - ", line);
        printf("\t- File : ");
        for (uint32_t i = 0; i < ut_assert.file_size; i++)
        {
            printf("%c", ut_assert.msg.data[i + ut_assert.line_size]);
        }
        printf("\n");
#endif

        return true;
    }
    else
    {
        ut_assert.state = 0;
        return false;
    }
}

/******************************************************************************
 * @brief Reset assert state to authorize a new assert
 * @param None
 * @return None
 ******************************************************************************/
void RESET_ASSERT(void)
{
    ut_assert.state = 0;
}

/******************************************************************************
 * @brief Assert activation or desactivation
 * @param activation : Set to 1 to activate asserts
 * @return None
 ******************************************************************************/
void ASSERT_ACTIVATION(uint8_t activation)
{
    if (activation)
    {
        ut_assert.enable = 1;
    }
    else
    {
        ut_assert.enable = 0;
    }
    RESET_ASSERT();
}
