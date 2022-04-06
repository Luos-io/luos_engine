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
SOURCEFILES_QUOTED_IF_SPACED=../src/config/luos_bootloader/peripheral/clock/plib_clock.c ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c ../src/config/luos_bootloader/peripheral/port/plib_port.c ../src/config/luos_bootloader/stdio/xc32_monitor.c ../src/config/luos_bootloader/initialization.c ../src/config/luos_bootloader/interrupts.c ../src/config/luos_bootloader/exceptions.c ../src/config/luos_bootloader/startup_xc32.c ../src/config/luos_bootloader/libc_syscalls.c ../src/main.c ../../../../../../Engine/bootloader/bootloader_core.c ../../../../../../Engine/Core/src/luos_engine.c ../../../../../../Engine/Core/src/luos_utils.c ../../../../../../Engine/Core/src/profile_core.c ../../../../../../Engine/Core/src/routing_table.c ../../../../../../Engine/Core/src/streaming.c ../../../../../../Engine/HAL/ATSAMD21/luos_hal.c ../../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c ../../../../../../Network/Robus/src/msg_alloc.c ../../../../../../Network/Robus/src/port_manager.c ../../../../../../Network/Robus/src/reception.c ../../../../../../Network/Robus/src/robus.c ../../../../../../Network/Robus/src/target.c ../../../../../../Network/Robus/src/timestamp.c ../../../../../../Network/Robus/src/transmission.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/903266522/plib_clock.o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ${OBJECTDIR}/_ext/1968410007/plib_port.o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ${OBJECTDIR}/_ext/1137593668/initialization.o ${OBJECTDIR}/_ext/1137593668/interrupts.o ${OBJECTDIR}/_ext/1137593668/exceptions.o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/1342350994/bootloader_core.o ${OBJECTDIR}/_ext/1555399743/luos_engine.o ${OBJECTDIR}/_ext/1555399743/luos_utils.o ${OBJECTDIR}/_ext/1555399743/profile_core.o ${OBJECTDIR}/_ext/1555399743/routing_table.o ${OBJECTDIR}/_ext/1555399743/streaming.o ${OBJECTDIR}/_ext/1234256160/luos_hal.o ${OBJECTDIR}/_ext/1857845632/robus_hal.o ${OBJECTDIR}/_ext/1000620745/msg_alloc.o ${OBJECTDIR}/_ext/1000620745/port_manager.o ${OBJECTDIR}/_ext/1000620745/reception.o ${OBJECTDIR}/_ext/1000620745/robus.o ${OBJECTDIR}/_ext/1000620745/target.o ${OBJECTDIR}/_ext/1000620745/timestamp.o ${OBJECTDIR}/_ext/1000620745/transmission.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/903266522/plib_clock.o.d ${OBJECTDIR}/_ext/901117036/plib_evsys.o.d ${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d ${OBJECTDIR}/_ext/1968410007/plib_port.o.d ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d ${OBJECTDIR}/_ext/1137593668/initialization.o.d ${OBJECTDIR}/_ext/1137593668/interrupts.o.d ${OBJECTDIR}/_ext/1137593668/exceptions.o.d ${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d ${OBJECTDIR}/_ext/1360937237/main.o.d ${OBJECTDIR}/_ext/1342350994/bootloader_core.o.d ${OBJECTDIR}/_ext/1555399743/luos_engine.o.d ${OBJECTDIR}/_ext/1555399743/luos_utils.o.d ${OBJECTDIR}/_ext/1555399743/profile_core.o.d ${OBJECTDIR}/_ext/1555399743/routing_table.o.d ${OBJECTDIR}/_ext/1555399743/streaming.o.d ${OBJECTDIR}/_ext/1234256160/luos_hal.o.d ${OBJECTDIR}/_ext/1857845632/robus_hal.o.d ${OBJECTDIR}/_ext/1000620745/msg_alloc.o.d ${OBJECTDIR}/_ext/1000620745/port_manager.o.d ${OBJECTDIR}/_ext/1000620745/reception.o.d ${OBJECTDIR}/_ext/1000620745/robus.o.d ${OBJECTDIR}/_ext/1000620745/target.o.d ${OBJECTDIR}/_ext/1000620745/timestamp.o.d ${OBJECTDIR}/_ext/1000620745/transmission.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/903266522/plib_clock.o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ${OBJECTDIR}/_ext/1968410007/plib_port.o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ${OBJECTDIR}/_ext/1137593668/initialization.o ${OBJECTDIR}/_ext/1137593668/interrupts.o ${OBJECTDIR}/_ext/1137593668/exceptions.o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/1342350994/bootloader_core.o ${OBJECTDIR}/_ext/1555399743/luos_engine.o ${OBJECTDIR}/_ext/1555399743/luos_utils.o ${OBJECTDIR}/_ext/1555399743/profile_core.o ${OBJECTDIR}/_ext/1555399743/routing_table.o ${OBJECTDIR}/_ext/1555399743/streaming.o ${OBJECTDIR}/_ext/1234256160/luos_hal.o ${OBJECTDIR}/_ext/1857845632/robus_hal.o ${OBJECTDIR}/_ext/1000620745/msg_alloc.o ${OBJECTDIR}/_ext/1000620745/port_manager.o ${OBJECTDIR}/_ext/1000620745/reception.o ${OBJECTDIR}/_ext/1000620745/robus.o ${OBJECTDIR}/_ext/1000620745/target.o ${OBJECTDIR}/_ext/1000620745/timestamp.o ${OBJECTDIR}/_ext/1000620745/transmission.o

# Source Files
SOURCEFILES=../src/config/luos_bootloader/peripheral/clock/plib_clock.c ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c ../src/config/luos_bootloader/peripheral/port/plib_port.c ../src/config/luos_bootloader/stdio/xc32_monitor.c ../src/config/luos_bootloader/initialization.c ../src/config/luos_bootloader/interrupts.c ../src/config/luos_bootloader/exceptions.c ../src/config/luos_bootloader/startup_xc32.c ../src/config/luos_bootloader/libc_syscalls.c ../src/main.c ../../../../../../Engine/bootloader/bootloader_core.c ../../../../../../Engine/Core/src/luos_engine.c ../../../../../../Engine/Core/src/luos_utils.c ../../../../../../Engine/Core/src/profile_core.c ../../../../../../Engine/Core/src/routing_table.c ../../../../../../Engine/Core/src/streaming.c ../../../../../../Engine/HAL/ATSAMD21/luos_hal.c ../../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c ../../../../../../Network/Robus/src/msg_alloc.c ../../../../../../Network/Robus/src/port_manager.c ../../../../../../Network/Robus/src/reception.c ../../../../../../Network/Robus/src/robus.c ../../../../../../Network/Robus/src/target.c ../../../../../../Network/Robus/src/timestamp.c ../../../../../../Network/Robus/src/transmission.c

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
${OBJECTDIR}/_ext/903266522/plib_clock.o: ../src/config/luos_bootloader/peripheral/clock/plib_clock.c  .generated_files/flags/luos_bootloader/d2cf92c10a682698eba1a1ea66abab1d406d013a .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/903266522" 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/903266522/plib_clock.o.d" -o ${OBJECTDIR}/_ext/903266522/plib_clock.o ../src/config/luos_bootloader/peripheral/clock/plib_clock.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/901117036/plib_evsys.o: ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c  .generated_files/flags/luos_bootloader/835b47cc70780525fa1d7dfb3cb98f7e71e13d6 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/901117036" 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o.d 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/901117036/plib_evsys.o.d" -o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968463158/plib_nvic.o: ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c  .generated_files/flags/luos_bootloader/272236f80c6409000a0838a3a25a4c15a218af3 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968463158" 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d" -o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o: ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c  .generated_files/flags/luos_bootloader/99a5d7d5428aaca68d01a38b8f937768ebd4890 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1001328808" 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d" -o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968410007/plib_port.o: ../src/config/luos_bootloader/peripheral/port/plib_port.c  .generated_files/flags/luos_bootloader/5afa9fe4cb4cfdcebf285139d12740a5d05a2f79 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968410007" 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968410007/plib_port.o.d" -o ${OBJECTDIR}/_ext/1968410007/plib_port.o ../src/config/luos_bootloader/peripheral/port/plib_port.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1426513406/xc32_monitor.o: ../src/config/luos_bootloader/stdio/xc32_monitor.c  .generated_files/flags/luos_bootloader/88cf65a2985d63d3700e38b554735f76dfc6610f .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1426513406" 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d" -o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ../src/config/luos_bootloader/stdio/xc32_monitor.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/initialization.o: ../src/config/luos_bootloader/initialization.c  .generated_files/flags/luos_bootloader/a317266a1b00e111262ec49002fb1e7a49d3d97e .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/initialization.o.d" -o ${OBJECTDIR}/_ext/1137593668/initialization.o ../src/config/luos_bootloader/initialization.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/interrupts.o: ../src/config/luos_bootloader/interrupts.c  .generated_files/flags/luos_bootloader/4fb595ee77880c2fb3d4e8245f4ee7e6d49b85c0 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/interrupts.o.d" -o ${OBJECTDIR}/_ext/1137593668/interrupts.o ../src/config/luos_bootloader/interrupts.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/exceptions.o: ../src/config/luos_bootloader/exceptions.c  .generated_files/flags/luos_bootloader/ba76cf8ce10694176b4d0a243b3a25848938ffd4 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/exceptions.o.d" -o ${OBJECTDIR}/_ext/1137593668/exceptions.o ../src/config/luos_bootloader/exceptions.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/startup_xc32.o: ../src/config/luos_bootloader/startup_xc32.c  .generated_files/flags/luos_bootloader/f9cea691af3359fcee512b2109b87812f1b320a6 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d" -o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ../src/config/luos_bootloader/startup_xc32.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/libc_syscalls.o: ../src/config/luos_bootloader/libc_syscalls.c  .generated_files/flags/luos_bootloader/38cbd821ea23339fe5f6e4ce2f8765fefcee5e9c .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d" -o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ../src/config/luos_bootloader/libc_syscalls.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/luos_bootloader/60e33b7d9e9134877b5a81aa7c10de85af5ccfea .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1342350994/bootloader_core.o: ../../../../../../Engine/bootloader/bootloader_core.c  .generated_files/flags/luos_bootloader/ecca2b0e924cc436dc7663084ac71a27756d7487 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1342350994" 
	@${RM} ${OBJECTDIR}/_ext/1342350994/bootloader_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1342350994/bootloader_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1342350994/bootloader_core.o.d" -o ${OBJECTDIR}/_ext/1342350994/bootloader_core.o ../../../../../../Engine/bootloader/bootloader_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/luos_engine.o: ../../../../../../Engine/Core/src/luos_engine.c  .generated_files/flags/luos_bootloader/999a0e5476981b5cf70b949d2c006e3643a6cb45 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_engine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/luos_engine.o.d" -o ${OBJECTDIR}/_ext/1555399743/luos_engine.o ../../../../../../Engine/Core/src/luos_engine.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/luos_utils.o: ../../../../../../Engine/Core/src/luos_utils.c  .generated_files/flags/luos_bootloader/422dd4db25c45e535cda3d6a816dbb974a56f064 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_utils.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_utils.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/luos_utils.o.d" -o ${OBJECTDIR}/_ext/1555399743/luos_utils.o ../../../../../../Engine/Core/src/luos_utils.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/profile_core.o: ../../../../../../Engine/Core/src/profile_core.c  .generated_files/flags/luos_bootloader/fd1a119274ae99c2166fc5a2d1ced2bb275830cb .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/profile_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/profile_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/profile_core.o.d" -o ${OBJECTDIR}/_ext/1555399743/profile_core.o ../../../../../../Engine/Core/src/profile_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/routing_table.o: ../../../../../../Engine/Core/src/routing_table.c  .generated_files/flags/luos_bootloader/daa7cf344b6ffc899d7d353c36768aa6909bee93 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/routing_table.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/routing_table.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/routing_table.o.d" -o ${OBJECTDIR}/_ext/1555399743/routing_table.o ../../../../../../Engine/Core/src/routing_table.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/streaming.o: ../../../../../../Engine/Core/src/streaming.c  .generated_files/flags/luos_bootloader/8d843a5013741132164d97274bdd4146ce9d68dd .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/streaming.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/streaming.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/streaming.o.d" -o ${OBJECTDIR}/_ext/1555399743/streaming.o ../../../../../../Engine/Core/src/streaming.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1234256160/luos_hal.o: ../../../../../../Engine/HAL/ATSAMD21/luos_hal.c  .generated_files/flags/luos_bootloader/b63dfe4cd2d06538a0d3898982edc47c1d203d06 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1234256160" 
	@${RM} ${OBJECTDIR}/_ext/1234256160/luos_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/1234256160/luos_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1234256160/luos_hal.o.d" -o ${OBJECTDIR}/_ext/1234256160/luos_hal.o ../../../../../../Engine/HAL/ATSAMD21/luos_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1857845632/robus_hal.o: ../../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c  .generated_files/flags/luos_bootloader/4dbd7808ae113b0b91818341c91c483f8101797e .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1857845632" 
	@${RM} ${OBJECTDIR}/_ext/1857845632/robus_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/1857845632/robus_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1857845632/robus_hal.o.d" -o ${OBJECTDIR}/_ext/1857845632/robus_hal.o ../../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/msg_alloc.o: ../../../../../../Network/Robus/src/msg_alloc.c  .generated_files/flags/luos_bootloader/32f23e7a871ea024bf560cc02d4b6cb4e6e47b1c .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/msg_alloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/msg_alloc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/msg_alloc.o.d" -o ${OBJECTDIR}/_ext/1000620745/msg_alloc.o ../../../../../../Network/Robus/src/msg_alloc.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/port_manager.o: ../../../../../../Network/Robus/src/port_manager.c  .generated_files/flags/luos_bootloader/5d3db4a32ba26136a3fde7c8278d04a481dde1de .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/port_manager.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/port_manager.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/port_manager.o.d" -o ${OBJECTDIR}/_ext/1000620745/port_manager.o ../../../../../../Network/Robus/src/port_manager.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/reception.o: ../../../../../../Network/Robus/src/reception.c  .generated_files/flags/luos_bootloader/64e36d944a0c588760a44e71a540e814d6ca3023 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/reception.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/reception.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/reception.o.d" -o ${OBJECTDIR}/_ext/1000620745/reception.o ../../../../../../Network/Robus/src/reception.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/robus.o: ../../../../../../Network/Robus/src/robus.c  .generated_files/flags/luos_bootloader/89d0727cd65fb1d0470155322bd20b8f293eab0c .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/robus.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/robus.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/robus.o.d" -o ${OBJECTDIR}/_ext/1000620745/robus.o ../../../../../../Network/Robus/src/robus.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/target.o: ../../../../../../Network/Robus/src/target.c  .generated_files/flags/luos_bootloader/c27d1e7cad831936101462671604d8a502507bf8 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/target.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/target.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/target.o.d" -o ${OBJECTDIR}/_ext/1000620745/target.o ../../../../../../Network/Robus/src/target.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/timestamp.o: ../../../../../../Network/Robus/src/timestamp.c  .generated_files/flags/luos_bootloader/96b7ca48b4449326e706f7a9c33ee1ca52c033f7 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/timestamp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/timestamp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/timestamp.o.d" -o ${OBJECTDIR}/_ext/1000620745/timestamp.o ../../../../../../Network/Robus/src/timestamp.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/transmission.o: ../../../../../../Network/Robus/src/transmission.c  .generated_files/flags/luos_bootloader/edcf0da67eb66710863d89df049f2e08a7e3a305 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/transmission.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/transmission.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/transmission.o.d" -o ${OBJECTDIR}/_ext/1000620745/transmission.o ../../../../../../Network/Robus/src/transmission.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
else
${OBJECTDIR}/_ext/903266522/plib_clock.o: ../src/config/luos_bootloader/peripheral/clock/plib_clock.c  .generated_files/flags/luos_bootloader/61c4e943e4932c8af5c10d8584cdeb6fd8564e3a .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/903266522" 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/903266522/plib_clock.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/903266522/plib_clock.o.d" -o ${OBJECTDIR}/_ext/903266522/plib_clock.o ../src/config/luos_bootloader/peripheral/clock/plib_clock.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/901117036/plib_evsys.o: ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c  .generated_files/flags/luos_bootloader/5236b9848097358e457449915a7288d4b78ccc25 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/901117036" 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o.d 
	@${RM} ${OBJECTDIR}/_ext/901117036/plib_evsys.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/901117036/plib_evsys.o.d" -o ${OBJECTDIR}/_ext/901117036/plib_evsys.o ../src/config/luos_bootloader/peripheral/evsys/plib_evsys.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968463158/plib_nvic.o: ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c  .generated_files/flags/luos_bootloader/39bd38fe191a5f4e43ecff826426342a07f58de0 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968463158" 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968463158/plib_nvic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968463158/plib_nvic.o.d" -o ${OBJECTDIR}/_ext/1968463158/plib_nvic.o ../src/config/luos_bootloader/peripheral/nvic/plib_nvic.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o: ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c  .generated_files/flags/luos_bootloader/8c081656113fe389f9a171974578584d3e79e25b .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1001328808" 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o.d" -o ${OBJECTDIR}/_ext/1001328808/plib_nvmctrl.o ../src/config/luos_bootloader/peripheral/nvmctrl/plib_nvmctrl.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1968410007/plib_port.o: ../src/config/luos_bootloader/peripheral/port/plib_port.c  .generated_files/flags/luos_bootloader/722aafdeec6eafefdca850a9b0d30bcbcc3f385 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1968410007" 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1968410007/plib_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1968410007/plib_port.o.d" -o ${OBJECTDIR}/_ext/1968410007/plib_port.o ../src/config/luos_bootloader/peripheral/port/plib_port.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1426513406/xc32_monitor.o: ../src/config/luos_bootloader/stdio/xc32_monitor.c  .generated_files/flags/luos_bootloader/c7df7aa4cd5aba1ac30d50a6d49275a08140fe62 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1426513406" 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1426513406/xc32_monitor.o.d" -o ${OBJECTDIR}/_ext/1426513406/xc32_monitor.o ../src/config/luos_bootloader/stdio/xc32_monitor.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/initialization.o: ../src/config/luos_bootloader/initialization.c  .generated_files/flags/luos_bootloader/6d3fae924724b8881d821bb06b26a4815989998a .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/initialization.o.d" -o ${OBJECTDIR}/_ext/1137593668/initialization.o ../src/config/luos_bootloader/initialization.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/interrupts.o: ../src/config/luos_bootloader/interrupts.c  .generated_files/flags/luos_bootloader/3ce38f7a9928f527843d71a5e3414bba9e92bb76 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/interrupts.o.d" -o ${OBJECTDIR}/_ext/1137593668/interrupts.o ../src/config/luos_bootloader/interrupts.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/exceptions.o: ../src/config/luos_bootloader/exceptions.c  .generated_files/flags/luos_bootloader/7a65fdb64e4c95a6b7353ff6b15a365bfcf0b3cd .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/exceptions.o.d" -o ${OBJECTDIR}/_ext/1137593668/exceptions.o ../src/config/luos_bootloader/exceptions.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/startup_xc32.o: ../src/config/luos_bootloader/startup_xc32.c  .generated_files/flags/luos_bootloader/db68c07d3b64690fef4a4e6af95220189e3a6f40 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/startup_xc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/startup_xc32.o.d" -o ${OBJECTDIR}/_ext/1137593668/startup_xc32.o ../src/config/luos_bootloader/startup_xc32.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1137593668/libc_syscalls.o: ../src/config/luos_bootloader/libc_syscalls.c  .generated_files/flags/luos_bootloader/7321c0c3c6962433bfe6abf09758b05800c63514 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1137593668" 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d 
	@${RM} ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1137593668/libc_syscalls.o.d" -o ${OBJECTDIR}/_ext/1137593668/libc_syscalls.o ../src/config/luos_bootloader/libc_syscalls.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/luos_bootloader/80bc35923e632d485a831e81a7a2eeb0b5cd8ae1 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1342350994/bootloader_core.o: ../../../../../../Engine/bootloader/bootloader_core.c  .generated_files/flags/luos_bootloader/70a2c80766411700a3ff3d6e21286e6437e6661b .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1342350994" 
	@${RM} ${OBJECTDIR}/_ext/1342350994/bootloader_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1342350994/bootloader_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1342350994/bootloader_core.o.d" -o ${OBJECTDIR}/_ext/1342350994/bootloader_core.o ../../../../../../Engine/bootloader/bootloader_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/luos_engine.o: ../../../../../../Engine/Core/src/luos_engine.c  .generated_files/flags/luos_bootloader/fc2ebb9c0fd7af3943fadbbb1d4f9a9e006de122 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_engine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/luos_engine.o.d" -o ${OBJECTDIR}/_ext/1555399743/luos_engine.o ../../../../../../Engine/Core/src/luos_engine.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/luos_utils.o: ../../../../../../Engine/Core/src/luos_utils.c  .generated_files/flags/luos_bootloader/535c7a1a4ad203a4b065730aee7928839abf805e .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_utils.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/luos_utils.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/luos_utils.o.d" -o ${OBJECTDIR}/_ext/1555399743/luos_utils.o ../../../../../../Engine/Core/src/luos_utils.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/profile_core.o: ../../../../../../Engine/Core/src/profile_core.c  .generated_files/flags/luos_bootloader/983fd606b45b685227a9425f3465ded81291f511 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/profile_core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/profile_core.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/profile_core.o.d" -o ${OBJECTDIR}/_ext/1555399743/profile_core.o ../../../../../../Engine/Core/src/profile_core.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/routing_table.o: ../../../../../../Engine/Core/src/routing_table.c  .generated_files/flags/luos_bootloader/68b27b893a024ac45331dc85f861bd550d2fdbc3 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/routing_table.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/routing_table.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/routing_table.o.d" -o ${OBJECTDIR}/_ext/1555399743/routing_table.o ../../../../../../Engine/Core/src/routing_table.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1555399743/streaming.o: ../../../../../../Engine/Core/src/streaming.c  .generated_files/flags/luos_bootloader/d91169aa7f3300d5681141f362825b3ec9b9f5a2 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1555399743" 
	@${RM} ${OBJECTDIR}/_ext/1555399743/streaming.o.d 
	@${RM} ${OBJECTDIR}/_ext/1555399743/streaming.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1555399743/streaming.o.d" -o ${OBJECTDIR}/_ext/1555399743/streaming.o ../../../../../../Engine/Core/src/streaming.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1234256160/luos_hal.o: ../../../../../../Engine/HAL/ATSAMD21/luos_hal.c  .generated_files/flags/luos_bootloader/8d7852cf5320dc5c48f9413b9cfeefd02b6e1007 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1234256160" 
	@${RM} ${OBJECTDIR}/_ext/1234256160/luos_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/1234256160/luos_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1234256160/luos_hal.o.d" -o ${OBJECTDIR}/_ext/1234256160/luos_hal.o ../../../../../../Engine/HAL/ATSAMD21/luos_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1857845632/robus_hal.o: ../../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c  .generated_files/flags/luos_bootloader/482cd64be0c6f105f8cb35b31d46be8282d865e9 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1857845632" 
	@${RM} ${OBJECTDIR}/_ext/1857845632/robus_hal.o.d 
	@${RM} ${OBJECTDIR}/_ext/1857845632/robus_hal.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1857845632/robus_hal.o.d" -o ${OBJECTDIR}/_ext/1857845632/robus_hal.o ../../../../../../Network/Robus/HAL/ATSAMD21/robus_hal.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/msg_alloc.o: ../../../../../../Network/Robus/src/msg_alloc.c  .generated_files/flags/luos_bootloader/b5b48f9894e9056139ee00b960d53ff1cb04d69d .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/msg_alloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/msg_alloc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/msg_alloc.o.d" -o ${OBJECTDIR}/_ext/1000620745/msg_alloc.o ../../../../../../Network/Robus/src/msg_alloc.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/port_manager.o: ../../../../../../Network/Robus/src/port_manager.c  .generated_files/flags/luos_bootloader/320e33e5b416e269f34f67b7a9b159c44ce96a37 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/port_manager.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/port_manager.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/port_manager.o.d" -o ${OBJECTDIR}/_ext/1000620745/port_manager.o ../../../../../../Network/Robus/src/port_manager.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/reception.o: ../../../../../../Network/Robus/src/reception.c  .generated_files/flags/luos_bootloader/8f3e6a8af32e82cfa762c0a26598b78866a43992 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/reception.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/reception.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/reception.o.d" -o ${OBJECTDIR}/_ext/1000620745/reception.o ../../../../../../Network/Robus/src/reception.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/robus.o: ../../../../../../Network/Robus/src/robus.c  .generated_files/flags/luos_bootloader/8e4395fdbed33f0b5e6d981dc5528facfde2492 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/robus.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/robus.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/robus.o.d" -o ${OBJECTDIR}/_ext/1000620745/robus.o ../../../../../../Network/Robus/src/robus.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/target.o: ../../../../../../Network/Robus/src/target.c  .generated_files/flags/luos_bootloader/c4f98c1a69d20f1ccb1367c00aae84aa9df34b19 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/target.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/target.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/target.o.d" -o ${OBJECTDIR}/_ext/1000620745/target.o ../../../../../../Network/Robus/src/target.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/timestamp.o: ../../../../../../Network/Robus/src/timestamp.c  .generated_files/flags/luos_bootloader/453446313f3326d069b6cded995bc208cf43b21a .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/timestamp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/timestamp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/timestamp.o.d" -o ${OBJECTDIR}/_ext/1000620745/timestamp.o ../../../../../../Network/Robus/src/timestamp.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1000620745/transmission.o: ../../../../../../Network/Robus/src/transmission.c  .generated_files/flags/luos_bootloader/b5a48407174f8f97c1fd79c3c69e836e67bcacb8 .generated_files/flags/luos_bootloader/4ed0d8af75e2e3882e901400885d4790cbe0e96e
	@${MKDIR} "${OBJECTDIR}/_ext/1000620745" 
	@${RM} ${OBJECTDIR}/_ext/1000620745/transmission.o.d 
	@${RM} ${OBJECTDIR}/_ext/1000620745/transmission.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -D__SAMD21J18A__ -DBOOTLOADER_CONFIG -I"../src" -I"../src/config/luos_bootloader" -I"../src/packs/ATSAMD21J18A_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../../../../../../Engine/OD" -I"../../../../../../Engine/Core/inc" -I"../../../../../../Engine/HAL/ATSAMD21" -I"../../../../../../Network/Robus/inc" -I"../../../../../../Network/Robus/HAL/ATSAMD21" -I"../../../../../../Engine/bootloader" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1000620745/transmission.o.d" -o ${OBJECTDIR}/_ext/1000620745/transmission.o ../../../../../../Network/Robus/src/transmission.c    -DXPRJ_luos_bootloader=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samd21a" ${PACK_COMMON_OPTIONS} 
	
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
