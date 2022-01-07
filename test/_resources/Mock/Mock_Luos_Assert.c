/******************************************************************************
 * @file Mock_Luos_Assert.c
 * @brief Mock function Luos_Assert()
 * @author Luos
 * @version 1.0.0
 ******************************************************************************/
#include "unit_test.h"

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
    unittest_assert(file, line);
}