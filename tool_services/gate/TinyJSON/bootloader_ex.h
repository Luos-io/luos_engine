/******************************************************************************
 * @file Bootloader extension
 * @brief Bootloader functionnalities for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef BOOTLOADER_EX_H
#define BOOTLOADER_EX_H

#include "io_struct.h"
#include "luos_engine.h"
#include "tiny-json.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
uint16_t Bootloader_LuosToJson(msg_t *, char *);
void Bootloader_JsonToLuos(service_t *, char *, json_t const *);
uint16_t Bootloader_StartData(char *);
void Bootloader_EndData(service_t *, char *, char *);

#endif /* BOOTLOADER_H */