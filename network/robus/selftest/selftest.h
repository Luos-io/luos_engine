/******************************************************************************
 * @file luos selftest
 * @brief function to test luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef SELFTEST_H
#define SELFTEST_H

#include "luos_engine.h"
#include "robus.h"

void selftest_SetRxFlag(void);
void selftest_run(void (*ok_callback)(void), void (*ko_callback)(void));

#endif /* LUOS_H */
