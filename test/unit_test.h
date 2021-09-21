#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "unity.h"
#include "luos_hal.h"
#include "luos_hal_config.h"
#include "luos_utils.h"
#include "robus_struct.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef UNIT_TEST_RUN
#define UNIT_TEST_RUN(f) RUN(#f, f)
#endif

/* This structure is used for unit test assert
 */
typedef struct
{
    uint8_t enable; // enable = 0 to disable asserts
    uint8_t state;  // state = 1 when an assert has occured
    uint32_t file_size;
    uint32_t line_size;
    msg_t msg;
} ut_luos_assert_t;

/*******************************************************************************
 * Function
 ******************************************************************************/
void RESET_ASSERT(void);
void ASSERT_ACTIVATION(uint8_t activation);
bool IS_ASSERT(void);
void NEW_TEST_CASE(char *title);
void NEW_STEP(char *title);
void RUN(const char *name, void (*f)(void));
void NEW_STEP_IN_LOOP(char message[], uint32_t index);
void unittest_assert(char *file, uint32_t line);

#endif //UNIT_TEST_H
