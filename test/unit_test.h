#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "unity.h"
#include "luos_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef UNIT_TEST_RUN
#define UNIT_TEST_RUN(f) RUN(#f, f)
#endif

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

#endif //UNIT_TEST_H
