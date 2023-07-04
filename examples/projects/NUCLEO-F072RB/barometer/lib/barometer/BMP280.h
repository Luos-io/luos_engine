#ifndef __BMP280_H__
#define __BMP280_H__
#include "SPI_Driver.h"
#include "luos_engine.h"
#include "robus_network.h"

void BMP280_Init();
void BMP280_Loop();

#endif
