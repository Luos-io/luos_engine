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
ut_luos_assert_t ut_assert = {.state = 0, .enable = 1};

/*******************************************************************************
 * Variables
 ******************************************************************************/
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
void RUN(const char *name, void (*function)(void))
{
    printf("\n\n========================================================================================================================================\n");
    printf("Unit test function : %s\n", name);
    printf("========================================================================================================================================\n");

    test_case_number = 0;
    RUN_TEST(function);
}

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
    char index2string[5];
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

/******************************************************************************
 * @brief Assertion management for unit testing
 * @param file name as a string
 * @param line number
 * @return None
 ******************************************************************************/
void unittest_assert(char *file, uint32_t line)
{
    msg_t msg;

    ut_assert.state     = 1;
    ut_assert.line_size = sizeof(line);
    ut_assert.file_size = strlen(file);

    msg.header.target_mode = BROADCAST;
    msg.header.target      = BROADCAST_VAL;
    msg.header.cmd         = ASSERT;
    msg.header.size        = sizeof(line) + strlen(file);
    memcpy(msg.data, &line, sizeof(line));
    memcpy(&msg.data[sizeof(line)], file, strlen(file));
    ut_assert.msg = msg;
}
