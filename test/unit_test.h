#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdio.h>
#include <stdbool.h>
#include "unity.h"
#include "luos_utils.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void RESET_ASSERT(void);
void ASSERT_ACTIVATION(uint8_t activation);
bool IS_ASSERT(void);
void UNIT_TEST_RUN(void (*function)());
void NEW_TEST_CASE(char *title);
void NEW_STEP();

#endif //UNIT_TEST_H
