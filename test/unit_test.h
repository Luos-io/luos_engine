#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdio.h>
#include <stdbool.h>
#include "unity.h"
#include "luos_utils.h"

/*******************************************************************************
 * Function
 ******************************************************************************/
void reset_assert(void);
void assert_activation(uint8_t activation);
bool is_assert(void);
void UNIT_TEST_RUN(void (*function)());
void NEW_TEST_CASE(char *title);
void NEW_STEP();

#endif //UNIT_TEST_H
