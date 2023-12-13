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
bool ut_assert = false;

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint16_t test_case_number;
uint16_t step_number;

// external error context
jmp_buf err_ctx;
bool try_state = false;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Start a new test case
 * @param title (test description title)
 * @return None
 ******************************************************************************/
void NEW_TEST_CASE(char *title)
{
    step_number = 0;
    test_case_number++;

    printf("\n\nTest Case %d :\n", test_case_number);
    printf("------------------------------------------------------------------------\n");
    printf("%c", title[0]);

    for (uint16_t i = 1; i < strlen(title); i++)
    {
        printf("%c", title[i]);
    }
    printf("\n------------------------------------------------------------------------\n");
}

/******************************************************************************
 * @brief Start a new test case
 * @param title (step description title)
 * @return None
 ******************************************************************************/
void NEW_STEP(char *title)
{
    printf("\t---> Check step %d :     ", ++step_number);

    for (uint16_t i = 0; i < strlen(title); i++)
    {
        printf("%c", title[i]);
    }
    printf("\n");
}

/******************************************************************************
 * @brief Start a new test case in a loop
 * @param title (step description title)
 * @param index (index of the loop)
 * @return None
 ******************************************************************************/
void NEW_STEP_IN_LOOP(char title[], uint32_t index)
{
    char index2string[11];
    char step_title[128];

    memset((void *)step_title, 0, sizeof(step_title));

    sprintf(index2string, "%d", index + 1);
    strcat(step_title, "Loop ");
    strcat(step_title, index2string);
    strcat(step_title, ": ");
    strcat(step_title, title);

    NEW_STEP(step_title);
}

/******************************************************************************
 * @brief Check if an assert has occured
 * @param None
 * @return True boolean if an assert has occured
 ******************************************************************************/
bool IS_ASSERT(void)
{
    bool ret_val = ut_assert;
    ut_assert    = false;
    return ret_val;
}

/******************************************************************************
 * @brief Reset assert state to authorize a new assert
 * @param None
 * @return None
 ******************************************************************************/
void RESET_ASSERT(void)
{
    ut_assert = false;
}

/******************************************************************************
 * @brief Assertion management for unit testing
 * @param file name as a string
 * @param line number
 * @return None
 ******************************************************************************/
void Luos_assert(char *file, uint32_t line)
{
#ifdef UNIT_TEST_DEBUG
    printf("\n*\t[INFO] Assert message received\n");
    printf("\t- Line : %d\n\t- File : %s\n", line, file);
#endif
    ut_assert = true;
    // This is the THROW of the TRY CATCH
    if (try_state)
    {
        _LONGJMP(err_ctx, 1);
        try_state = false;
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}
