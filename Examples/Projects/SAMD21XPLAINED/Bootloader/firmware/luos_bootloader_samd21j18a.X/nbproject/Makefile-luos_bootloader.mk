#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-luos_bootloader.mk)" "nbproject/Makefile-local-luos_bootloader.mk"
include nbproject/Makefile-local-luos_bootloader.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=luos_bootloader
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../src/config/luos_bootloader/peripheral/clock/plib_clock.c ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c ../src/config/luos_bootloader/peripheral/port/plib_port.c ../src/config/luos_bootloader/stdio/xc32_monitor.c ../src/config/luos_bootloader/initialization.c ../src/config/luos_bootloader/interrupts.c ../src/config/luos_bootloader/exceptions.c ../src/config/luos_bootloader/startup_xc32.c ../src/config/luos_bootloader/libc_syscalls.c ../src/main.c ../../../../../Luos_engine/Bootloader/bootloader_core.c ../../../../../Luos_engine/Core/src/luos_engine.c ../../../../../Luos_engine/Core/src/luos_utils.c ../../../../../Luos_engine/Core/src/profile_core.c ../../../../../Luos_engine/Core/src/routing_table.c ../../../../../Luos_engine/Core/src/streaming.c ../../../../../Luos_engine/HAL/ATSAMD21/luos_hal.c ../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c ../../../../../Network/Robus/src/msg_alloc.c ../../../../../Network/Robus/src/port_manager.c ../../../../../Network/Robus/src/reception.c ../../../../../Network/Robus/src/robus.c ../../../../../Network/Robus/src/target.c ../../../../../Network/Robus/src/timestamp.c ../../../../../Network/Robus/src/transmission.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/903266522/plib_clock.o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ${OBJECTDIR}/_ext/1968410007/plib_port.o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ${OBJECTDIR}/_ext/1137593668/initialization.o ${OBJECTDIR}/_ext/1137593668/interrupts.o ${OBJECTDIR}/_ext/1137593668/exceptions.o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/785613041/bootloader_core.o ${OBJECTDIR}/_ext/1494896510/luos_engine.o ${OBJECTDIR}/_ext/1494896510/luos_utils.o ${OBJECTDIR}/_ext/1494896510/profile_core.o ${OBJECTDIR}/_ext/1494896510/routing_table.o ${OBJECTDIR}/_ext/1494896510/streaming.o ${OBJECTDIR}/_ext/665251421/luos_hal.o ${OBJECTDIR}/_ext/622140657/robus_hal.o ${OBJECTDIR}/_ext/857245384/msg_alloc.o ${OBJECTDIR}/_ext/857245384/port_manager.o ${OBJECTDIR}/_ext/857245384/reception.o ${OBJECTDIR}/_ext/857245384/robus.o ${OBJECTDIR}/_ext/857245384/target.o ${OBJECTDIR}/_ext/857245384/timestamp.o ${OBJECTDIR}/_ext/857245384/transmission.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/903266522/plib_clock.o.d ${OBJECTDIR}/_ext/901117036/plib_evsys.o.d ${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d ${OBJECTDIR}/_ext/1968410007/plib_port.o.d ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d ${OBJECTDIR}/_ext/1137593668/initialization.o.d ${OBJECTDIR}/_ext/1137593668/interrupts.o.d ${OBJECTDIR}/_ext/1137593668/exceptions.o.d ${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d ${OBJECTDIR}/_ext/1360937237/main.o.d ${OBJECTDIR}/_ext/785613041/bootloader_core.o.d ${OBJECTDIR}/_ext/1494896510/luos_engine.o.d ${OBJECTDIR}/_ext/1494896510/luos_utils.o.d ${OBJECTDIR}/_ext/1494896510/profile_core.o.d ${OBJECTDIR}/_ext/1494896510/routing_table.o.d ${OBJECTDIR}/_ext/1494896510/streaming.o.d ${OBJECTDIR}/_ext/665251421/luos_hal.o.d ${OBJECTDIR}/_ext/622140657/robus_hal.o.d ${OBJECTDIR}/_ext/857245384/msg_alloc.o.d ${OBJECTDIR}/_ext/857245384/port_manager.o.d ${OBJECTDIR}/_ext/857245384/reception.o.d ${OBJECTDIR}/_ext/857245384/robus.o.d ${OBJECTDIR}/_ext/857245384/target.o.d ${OBJECTDIR}/_ext/857245384/timestamp.o.d ${OBJECTDIR}/_ext/857245384/transmission.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/903266522/plib_clock.o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ${OBJECTDIR}/_ext/1968410007/plib_port.o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ${OBJECTDIR}/_ext/1137593668/initialization.o ${OBJECTDIR}/_ext/1137593668/interrupts.o ${OBJECTDIR}/_ext/1137593668/exceptions.o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/785613041/bootloader_core.o ${OBJECTDIR}/_ext/1494896510/luos_engine.o ${OBJECTDIR}/_ext/1494896510/luos_utils.o ${OBJECTDIR}/_ext/1494896510/profile_core.o ${OBJECTDIR}/_ext/1494896510/routing_table.o ${OBJECTDIR}/_ext/1494896510/streaming.o ${OBJECTDIR}/_ext/665251421/luos_hal.o ${OBJECTDIR}/_ext/622140657/robus_hal.o ${OBJECTDIR}/_ext/857245384/msg_alloc.o ${OBJECTDIR}/_ext/857245384/port_manager.o ${OBJECTDIR}/_ext/857245384/reception.o ${OBJECTDIR}/_ext/857245384/robus.o ${OBJECTDIR}/_ext/857245384/target.o ${OBJECTDIR}/_ext/857245384/timestamp.o ${OBJECTDIR}/_ext/857245384/transmission.o

# Source Files
SOURCEFILES=../src/config/luos_bootloader/peripheral/clock/plib_clock.c ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c ../src/config/luos_bootloader/peripheral/port/plib_port.c ../src/config/luos_bootloader/stdio/xc32_monitor.c ../src/config/luos_bootloader/initialization.c ../src/config/luos_bootloader/interrupts.c ../src/config/luos_bootloader/exceptions.c ../src/config/luos_bootloader/startup_xc32.c ../src/config/luos_bootloader/libc_syscalls.c ../src/main.c ../../../../../Luos_engine/Bootloader/bootloader_core.c ../../../../../Luos_engine/Core/src/luos_engine.c ../../../../../Luos_engine/Core/src/luos_utils.c ../../../../../Luos_engine/Core/src/profile_core.c ../../../../../Luos_engine/Core/src/routing_table.c ../../../../../Luos_engine/Core/src/streaming.c ../../../../../Luos_engine/HAL/ATSAMD21/luos_hal.c ../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c ../../../../../Network/Robus/src/msg_alloc.c ../../../../../Network/Robus/src/port_manager.c ../../../../../Network/Robus/src/reception.c ../../../../../Network/Robus/src/robus.c ../../../../../Network/Robus/src/target.c ../../../../../Network/Robus/src/timestamp.c ../../../../../Network/Robus/src/transmission.c

# Pack Options 
PACK_COMMON_OPTIONS=-I "${CMSIS_DIR}/CMSIS/Core/Include"



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-luos_bootloader.mk dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=ATSAMD21J18A
MP_LINKER_FILE_OPTION=,--script="..\src\config\luos_bootloader\ATSAMD21J18A_bootloader.ld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/903266522/plib_clock.o: ../src/config/luos_bootloader/peripheral/clock/plib_clock.c  .generated_files/flags/luos_bootloader/c577427a701efbbe0fc291b2c961b82274552bf8 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/903266522" 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/903266522/plib_clock.o.d" -o ${OBJECTDIR}/_ext/903266522/plib_clock.o ../src/config/luos_bootloader/peripheral/clock/plib_clock.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/901117036/plib_evsys.o: ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c  .generated_files/flags/luos_bootloader/76037a050b38f2a60f7d0e3d17c72d22015fa2ca .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/901117036" 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o.d 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/901117036/plib_evsys.o.d" -o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968463158/plib_nvic.o: ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c  .generated_files/flags/luos_bootloader/9d4c0003dfe7299768acb82e8c8b19d5126cbe51 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968463158" 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d" -o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o: ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c  .generated_files/flags/luos_bootloader/c50c493305ca764543ee337b77a79ee9b87ed448 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1001328808" 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d" -o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968410007/plib_port.o: ../src/config/luos_bootloader/peripheral/port/plib_port.c  .generated_files/flags/luos_bootloader/18eaea17bddd8f7d3ab6e567cc43e22207e9a327 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968410007" 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968410007/plib_port.o.d" -o ${OBJECTDIR}/_ext/1968410007/plib_port.o ../src/config/luos_bootloader/peripheral/port/plib_port.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1426513406/xc32_monitor.o: ../src/config/luos_bootloader/stdio/xc32_monitor.c  .generated_files/flags/luos_bootloader/97281192591bc8691fd08441eec1c085bdd46788 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1426513406" 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d" -o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ../src/config/luos_bootloader/stdio/xc32_monitor.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/initialization.o: ../src/config/luos_bootloader/initialization.c  .generated_files/flags/luos_bootloader/bbbddd79003c3977f3ab81d3542d780e20d11913 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/initialization.o.d" -o ${OBJECTDIR}/_ext/1137593668/initialization.o ../src/config/luos_bootloader/initialization.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/interrupts.o: ../src/config/luos_bootloader/interrupts.c  .generated_files/flags/luos_bootloader/4399fdeb813344725d2ab10fbf1c62cb26b7531d .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/interrupts.o.d" -o ${OBJECTDIR}/_ext/1137593668/interrupts.o ../src/config/luos_bootloader/interrupts.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/exceptions.o: ../src/config/luos_bootloader/exceptions.c  .generated_files/flags/luos_bootloader/41cc924da90c1f6a95ef96529067e408f3854aa .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/exceptions.o.d" -o ${OBJECTDIR}/_ext/1137593668/exceptions.o ../src/config/luos_bootloader/exceptions.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/startup_xc32.o: ../src/config/luos_bootloader/startup_xc32.c  .generated_files/flags/luos_bootloader/d549fbd9c2d3d0381a4c3c5d6e31ae2294e5fa6a .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d" -o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ../src/config/luos_bootloader/startup_xc32.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/libc_syscalls.o: ../src/config/luos_bootloader/libc_syscalls.c  .generated_files/flags/luos_bootloader/5a783e49b0bd1fbd3ae975211103a13906425dfd .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d" -o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ../src/config/luos_bootloader/libc_syscalls.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/luos_bootloader/a49113937ffa31d9277f1bf4f360849f6129bc0d .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/785613041/bootloader_core.o: ../../../../../Luos_engine/Bootloader/bootloader_core.c  .generated_files/flags/luos_bootloader/2dcaf45850b8c4ea75f8616eae1762f086447837 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/785613041" 
	@${RM} ${OBJECTDIR}/_ext/785613041/bootloader_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/785613041/bootloader_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/785613041/bootloader_core.o.d" -o ${OBJECTDIR}/_ext/785613041/bootloader_core.o ../../../../../Luos_engine/Bootloader/bootloader_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/luos_engine.o: ../../../../../Luos_engine/Core/src/luos_engine.c  .generated_files/flags/luos_bootloader/b5bfcb4911d6c0be9221603ff7e945b5339759ef .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_engine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/luos_engine.o.d" -o ${OBJECTDIR}/_ext/1494896510/luos_engine.o ../../../../../Luos_engine/Core/src/luos_engine.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/luos_utils.o: ../../../../../Luos_engine/Core/src/luos_utils.c  .generated_files/flags/luos_bootloader/e4faa3d1832fc864376da81e4c864c74734c6ea1 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_utils.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_utils.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/luos_utils.o.d" -o ${OBJECTDIR}/_ext/1494896510/luos_utils.o ../../../../../Luos_engine/Core/src/luos_utils.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/profile_core.o: ../../../../../Luos_engine/Core/src/profile_core.c  .generated_files/flags/luos_bootloader/7352499f2fbef5489695df84cdc7cf4f4a176318 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/profile_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/profile_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/profile_core.o.d" -o ${OBJECTDIR}/_ext/1494896510/profile_core.o ../../../../../Luos_engine/Core/src/profile_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/routing_table.o: ../../../../../Luos_engine/Core/src/routing_table.c  .generated_files/flags/luos_bootloader/c39748d15b614a0974a4aa0859d384b4502b2f76 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/routing_table.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/routing_table.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/routing_table.o.d" -o ${OBJECTDIR}/_ext/1494896510/routing_table.o ../../../../../Luos_engine/Core/src/routing_table.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/streaming.o: ../../../../../Luos_engine/Core/src/streaming.c  .generated_files/flags/luos_bootloader/7d61a264177cf85773a6094a754a1e359cb597b2 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/streaming.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/streaming.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/streaming.o.d" -o ${OBJECTDIR}/_ext/1494896510/streaming.o ../../../../../Luos_engine/Core/src/streaming.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/665251421/luos_hal.o: ../../../../../Luos_engine/HAL/ATSAMD21/luos_hal.c  .generated_files/flags/luos_bootloader/8a5f5858e7b712423fe0de983e2be80c91010162 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/665251421" 
	@${RM} ${OBJECTDIR}/_ext/665251421/luos_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/665251421/luos_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/665251421/luos_hal.o.d" -o ${OBJECTDIR}/_ext/665251421/luos_hal.o ../../../../../Luos_engine/HAL/ATSAMD21/luos_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/622140657/robus_hal.o: ../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c  .generated_files/flags/luos_bootloader/d2dfa2e8bc6aabaa267a710d488bc2321fc14dc .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/622140657" 
	@${RM} ${OBJECTDIR}/_ext/622140657/robus_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/622140657/robus_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/622140657/robus_hal.o.d" -o ${OBJECTDIR}/_ext/622140657/robus_hal.o ../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/msg_alloc.o: ../../../../../Network/Robus/src/msg_alloc.c  .generated_files/flags/luos_bootloader/6f9804afdc015b4c49d0eaebac25810ddee2a037 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/msg_alloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/msg_alloc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/msg_alloc.o.d" -o ${OBJECTDIR}/_ext/857245384/msg_alloc.o ../../../../../Network/Robus/src/msg_alloc.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/port_manager.o: ../../../../../Network/Robus/src/port_manager.c  .generated_files/flags/luos_bootloader/b6ac12b05c8c28d3eaf1aa05a9e083b70a7ea514 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/port_manager.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/port_manager.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/port_manager.o.d" -o ${OBJECTDIR}/_ext/857245384/port_manager.o ../../../../../Network/Robus/src/port_manager.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/reception.o: ../../../../../Network/Robus/src/reception.c  .generated_files/flags/luos_bootloader/2d4ed8b83277ad5180f5f526888738a649344fb8 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/reception.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/reception.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/reception.o.d" -o ${OBJECTDIR}/_ext/857245384/reception.o ../../../../../Network/Robus/src/reception.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/robus.o: ../../../../../Network/Robus/src/robus.c  .generated_files/flags/luos_bootloader/34102e238c495586bde489888bfb27d55f5ccd60 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/robus.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/robus.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/robus.o.d" -o ${OBJECTDIR}/_ext/857245384/robus.o ../../../../../Network/Robus/src/robus.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/target.o: ../../../../../Network/Robus/src/target.c  .generated_files/flags/luos_bootloader/fb51ba6b59a6abb6bed7a0684dbae2845b27e20a .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/target.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/target.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/target.o.d" -o ${OBJECTDIR}/_ext/857245384/target.o ../../../../../Network/Robus/src/target.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/timestamp.o: ../../../../../Network/Robus/src/timestamp.c  .generated_files/flags/luos_bootloader/3988329195a952d3a73641914ed7977574b64a6f .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/timestamp.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/timestamp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/timestamp.o.d" -o ${OBJECTDIR}/_ext/857245384/timestamp.o ../../../../../Network/Robus/src/timestamp.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/transmission.o: ../../../../../Network/Robus/src/transmission.c  .generated_files/flags/luos_bootloader/86e2b1753c18378b1034a56a45492d3753e8b1eb .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/transmission.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/transmission.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/transmission.o.d" -o ${OBJECTDIR}/_ext/857245384/transmission.o ../../../../../Network/Robus/src/transmission.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
else
${OBJECTDIR}/_ext/903266522/plib_clock.o: ../src/config/luos_bootloader/peripheral/clock/plib_clock.c  .generated_files/flags/luos_bootloader/d0340862297c5222646337001672f13f077a021b .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/903266522" 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/903266522/plib_clock.o.d" -o ${OBJECTDIR}/_ext/903266522/plib_clock.o ../src/config/luos_bootloader/peripheral/clock/plib_clock.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/901117036/plib_evsys.o: ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c  .generated_files/flags/luos_bootloader/f4b06cc59a65eeb7684f3ce5de0c65d77c6ea7ac .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/901117036" 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o.d 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/901117036/plib_evsys.o.d" -o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968463158/plib_nvic.o: ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c  .generated_files/flags/luos_bootloader/bc843c5c86790caeb912eb3a4c39990750030b82 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968463158" 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d" -o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o: ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c  .generated_files/flags/luos_bootloader/3bd2bd1969e8552d1a823724271254083cf441f4 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1001328808" 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d" -o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968410007/plib_port.o: ../src/config/luos_bootloader/peripheral/port/plib_port.c  .generated_files/flags/luos_bootloader/31ec28b0a8f9b9080d1ca77f7bc8e3172da255f3 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968410007" 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968410007/plib_port.o.d" -o ${OBJECTDIR}/_ext/1968410007/plib_port.o ../src/config/luos_bootloader/peripheral/port/plib_port.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1426513406/xc32_monitor.o: ../src/config/luos_bootloader/stdio/xc32_monitor.c  .generated_files/flags/luos_bootloader/8f3e7a94c6f40c13052cb961e74a90749f3737f1 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1426513406" 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d" -o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ../src/config/luos_bootloader/stdio/xc32_monitor.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/initialization.o: ../src/config/luos_bootloader/initialization.c  .generated_files/flags/luos_bootloader/3248ee483443992abf5761684d314a7610461f0c .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/initialization.o.d" -o ${OBJECTDIR}/_ext/1137593668/initialization.o ../src/config/luos_bootloader/initialization.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/interrupts.o: ../src/config/luos_bootloader/interrupts.c  .generated_files/flags/luos_bootloader/bd47ebc9be5177a93fb09a8d6e688e621e3e5f64 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/interrupts.o.d" -o ${OBJECTDIR}/_ext/1137593668/interrupts.o ../src/config/luos_bootloader/interrupts.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/exceptions.o: ../src/config/luos_bootloader/exceptions.c  .generated_files/flags/luos_bootloader/2f9944142b3fb6e7190bd38f699a51f2b88ffa1 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/exceptions.o.d" -o ${OBJECTDIR}/_ext/1137593668/exceptions.o ../src/config/luos_bootloader/exceptions.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/startup_xc32.o: ../src/config/luos_bootloader/startup_xc32.c  .generated_files/flags/luos_bootloader/997d2fafe2a6048fa86829a6c36932de5e66d07a .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d" -o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ../src/config/luos_bootloader/startup_xc32.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/libc_syscalls.o: ../src/config/luos_bootloader/libc_syscalls.c  .generated_files/flags/luos_bootloader/afb1be341535cae406b40f499e962f56d6e3ffb7 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d" -o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ../src/config/luos_bootloader/libc_syscalls.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/luos_bootloader/3cfe5a934c66ce145b5033d99afbbcd41a01a880 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/785613041/bootloader_core.o: ../../../../../Luos_engine/Bootloader/bootloader_core.c  .generated_files/flags/luos_bootloader/31abb84bbc3054438a3485e33fb47060102dd439 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/785613041" 
	@${RM} ${OBJECTDIR}/_ext/785613041/bootloader_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/785613041/bootloader_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/785613041/bootloader_core.o.d" -o ${OBJECTDIR}/_ext/785613041/bootloader_core.o ../../../../../Luos_engine/Bootloader/bootloader_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/luos_engine.o: ../../../../../Luos_engine/Core/src/luos_engine.c  .generated_files/flags/luos_bootloader/159ae3259e51fbd24baf710cee8fb6b0d5c1f217 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_engine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/luos_engine.o.d" -o ${OBJECTDIR}/_ext/1494896510/luos_engine.o ../../../../../Luos_engine/Core/src/luos_engine.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/luos_utils.o: ../../../../../Luos_engine/Core/src/luos_utils.c  .generated_files/flags/luos_bootloader/9458c1948745de2b3f4fbdb5de46bd9cf856396f .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_utils.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/luos_utils.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/luos_utils.o.d" -o ${OBJECTDIR}/_ext/1494896510/luos_utils.o ../../../../../Luos_engine/Core/src/luos_utils.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/profile_core.o: ../../../../../Luos_engine/Core/src/profile_core.c  .generated_files/flags/luos_bootloader/206f4282c5f2d375bc06291e2b0212379d78230f .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/profile_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/profile_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/profile_core.o.d" -o ${OBJECTDIR}/_ext/1494896510/profile_core.o ../../../../../Luos_engine/Core/src/profile_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/routing_table.o: ../../../../../Luos_engine/Core/src/routing_table.c  .generated_files/flags/luos_bootloader/fc85cc26c7ad7e49c7e24cc29bd7038f5bd93662 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/routing_table.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/routing_table.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/routing_table.o.d" -o ${OBJECTDIR}/_ext/1494896510/routing_table.o ../../../../../Luos_engine/Core/src/routing_table.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1494896510/streaming.o: ../../../../../Luos_engine/Core/src/streaming.c  .generated_files/flags/luos_bootloader/70784b59b8fbb9f9686b96cf518677c6ef2dac55 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1494896510" 
	@${RM} ${OBJECTDIR}/_ext/1494896510/streaming.o.d 
	@${RM} ${OBJECTDIR}/_ext/1494896510/streaming.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1494896510/streaming.o.d" -o ${OBJECTDIR}/_ext/1494896510/streaming.o ../../../../../Luos_engine/Core/src/streaming.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/665251421/luos_hal.o: ../../../../../Luos_engine/HAL/ATSAMD21/luos_hal.c  .generated_files/flags/luos_bootloader/a01b96b0e649636e8f208e1d152f5e3d347229e6 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/665251421" 
	@${RM} ${OBJECTDIR}/_ext/665251421/luos_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/665251421/luos_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/665251421/luos_hal.o.d" -o ${OBJECTDIR}/_ext/665251421/luos_hal.o ../../../../../Luos_engine/HAL/ATSAMD21/luos_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/622140657/robus_hal.o: ../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c  .generated_files/flags/luos_bootloader/caff0aa9da1474a7a83c32c03f5ca52717f335e0 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/622140657" 
	@${RM} ${OBJECTDIR}/_ext/622140657/robus_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/622140657/robus_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/622140657/robus_hal.o.d" -o ${OBJECTDIR}/_ext/622140657/robus_hal.o ../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/msg_alloc.o: ../../../../../Network/Robus/src/msg_alloc.c  .generated_files/flags/luos_bootloader/c7aa16ad207b39b6ee4b7e5156637408e9610d17 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/msg_alloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/msg_alloc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/msg_alloc.o.d" -o ${OBJECTDIR}/_ext/857245384/msg_alloc.o ../../../../../Network/Robus/src/msg_alloc.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/port_manager.o: ../../../../../Network/Robus/src/port_manager.c  .generated_files/flags/luos_bootloader/4a3e016db868096eb858d3f3239d4a9a306bde5c .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/port_manager.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/port_manager.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/port_manager.o.d" -o ${OBJECTDIR}/_ext/857245384/port_manager.o ../../../../../Network/Robus/src/port_manager.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/reception.o: ../../../../../Network/Robus/src/reception.c  .generated_files/flags/luos_bootloader/d9c15cde01f8fba99e36f4cee52964515ab20d38 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/reception.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/reception.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/reception.o.d" -o ${OBJECTDIR}/_ext/857245384/reception.o ../../../../../Network/Robus/src/reception.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/robus.o: ../../../../../Network/Robus/src/robus.c  .generated_files/flags/luos_bootloader/5a529bb78f1cbef563cada1de9a3b0af47f523c .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/robus.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/robus.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/robus.o.d" -o ${OBJECTDIR}/_ext/857245384/robus.o ../../../../../Network/Robus/src/robus.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/target.o: ../../../../../Network/Robus/src/target.c  .generated_files/flags/luos_bootloader/4252a827af146e26dcbb89df64c3e82f9d536290 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/target.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/target.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/target.o.d" -o ${OBJECTDIR}/_ext/857245384/target.o ../../../../../Network/Robus/src/target.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/timestamp.o: ../../../../../Network/Robus/src/timestamp.c  .generated_files/flags/luos_bootloader/658dcbaa5203604529340fde2b60ffac269c9ff .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/timestamp.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/timestamp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/timestamp.o.d" -o ${OBJECTDIR}/_ext/857245384/timestamp.o ../../../../../Network/Robus/src/timestamp.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/857245384/transmission.o: ../../../../../Network/Robus/src/transmission.c  .generated_files/flags/luos_bootloader/3dc1e906ec45108ca49c6dc030a08688d16bab70 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/857245384" 
	@${RM} ${OBJECTDIR}/_ext/857245384/transmission.o.d 
	@${RM} ${OBJECTDIR}/_ext/857245384/transmission.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../Luos_engine/Core/inc" -I"../../../../../Luos_engine/Bootloader" -I"../../../../../Luos_engine/OD" -I"../../../../../Luos_engine/HAL/ATSAMD21" -I"../../../../../Network/Robus/inc" -I"../../../../../Network/Robus/HAL/ATSAMD21" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/857245384/transmission.o.d" -o ${OBJECTDIR}/_ext/857245384/transmission.o ../../../../../Network/Robus/src/transmission.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../src/config/luos_bootloader/ATSAMD21J18A_bootloader.ld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g   -mprocessor=$(MP_PROCESSOR_OPTION) -mno-device-startup-code -o dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=_min_heap_size=512,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -mdfp="${DFP_DIR}/samd21a"
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../src/config/luos_bootloader/ATSAMD21J18A_bootloader.ld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION) -mno-device-startup-code -o dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=512,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -mdfp="${DFP_DIR}/samd21a"
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/luos_bootloader_samd21j18a.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/luos_bootloader
	${RM} -r dist/luos_bootloader

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
