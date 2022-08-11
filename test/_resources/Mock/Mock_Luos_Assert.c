/******************************************************************************
 * @file Mock_Luos_Assert.c
 * @brief Mock function Luos_Assert()
 * @author Luos
 * @version 1.0.0
 ******************************************************************************/
#ifdef UNIT_TEST
#include "unit_test.h"
#else
#include "luos_hal.h"
#endif
/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Mock : Redefine function Luos_assert
 * @param file name as a string
 * @param line number
 * @return None
 ******************************************************************************/
void Luos_assert(char *file, uint32_t line)
{
#ifdef UNIT_TEST
    UNIT_TEST_ASSERT(file, line);
#endif
}
