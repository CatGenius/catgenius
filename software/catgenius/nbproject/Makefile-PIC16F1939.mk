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
ifeq "$(wildcard nbproject/Makefile-local-PIC16F1939.mk)" "nbproject/Makefile-local-PIC16F1939.mk"
include nbproject/Makefile-local-PIC16F1939.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=PIC16F1939
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=cof
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../common/catgenie120.c ../common/catsensor.c ../common/cmdline.c ../common/rtc.c ../common/serial.c ../common/timer.c ../common/water.c ../common/bluetooth.c ../common/cmdline_box.c ../common/cmdline_gpio.c ../common/cmdline_tag.c ../common/cr14.c ../common/i2c.c ../common/eventlog.c catgenius.c litterlanguage.c romwashprogram.c userinterface.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1270477542/catgenie120.p1 ${OBJECTDIR}/_ext/1270477542/catsensor.p1 ${OBJECTDIR}/_ext/1270477542/cmdline.p1 ${OBJECTDIR}/_ext/1270477542/rtc.p1 ${OBJECTDIR}/_ext/1270477542/serial.p1 ${OBJECTDIR}/_ext/1270477542/timer.p1 ${OBJECTDIR}/_ext/1270477542/water.p1 ${OBJECTDIR}/_ext/1270477542/bluetooth.p1 ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1 ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1 ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1 ${OBJECTDIR}/_ext/1270477542/cr14.p1 ${OBJECTDIR}/_ext/1270477542/i2c.p1 ${OBJECTDIR}/_ext/1270477542/eventlog.p1 ${OBJECTDIR}/catgenius.p1 ${OBJECTDIR}/litterlanguage.p1 ${OBJECTDIR}/romwashprogram.p1 ${OBJECTDIR}/userinterface.p1
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1270477542/catgenie120.p1.d ${OBJECTDIR}/_ext/1270477542/catsensor.p1.d ${OBJECTDIR}/_ext/1270477542/cmdline.p1.d ${OBJECTDIR}/_ext/1270477542/rtc.p1.d ${OBJECTDIR}/_ext/1270477542/serial.p1.d ${OBJECTDIR}/_ext/1270477542/timer.p1.d ${OBJECTDIR}/_ext/1270477542/water.p1.d ${OBJECTDIR}/_ext/1270477542/bluetooth.p1.d ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1.d ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1.d ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1.d ${OBJECTDIR}/_ext/1270477542/cr14.p1.d ${OBJECTDIR}/_ext/1270477542/i2c.p1.d ${OBJECTDIR}/_ext/1270477542/eventlog.p1.d ${OBJECTDIR}/catgenius.p1.d ${OBJECTDIR}/litterlanguage.p1.d ${OBJECTDIR}/romwashprogram.p1.d ${OBJECTDIR}/userinterface.p1.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1270477542/catgenie120.p1 ${OBJECTDIR}/_ext/1270477542/catsensor.p1 ${OBJECTDIR}/_ext/1270477542/cmdline.p1 ${OBJECTDIR}/_ext/1270477542/rtc.p1 ${OBJECTDIR}/_ext/1270477542/serial.p1 ${OBJECTDIR}/_ext/1270477542/timer.p1 ${OBJECTDIR}/_ext/1270477542/water.p1 ${OBJECTDIR}/_ext/1270477542/bluetooth.p1 ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1 ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1 ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1 ${OBJECTDIR}/_ext/1270477542/cr14.p1 ${OBJECTDIR}/_ext/1270477542/i2c.p1 ${OBJECTDIR}/_ext/1270477542/eventlog.p1 ${OBJECTDIR}/catgenius.p1 ${OBJECTDIR}/litterlanguage.p1 ${OBJECTDIR}/romwashprogram.p1 ${OBJECTDIR}/userinterface.p1

# Source Files
SOURCEFILES=../common/catgenie120.c ../common/catsensor.c ../common/cmdline.c ../common/rtc.c ../common/serial.c ../common/timer.c ../common/water.c ../common/bluetooth.c ../common/cmdline_box.c ../common/cmdline_gpio.c ../common/cmdline_tag.c ../common/cr14.c ../common/i2c.c ../common/eventlog.c catgenius.c litterlanguage.c romwashprogram.c userinterface.c


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
	${MAKE} ${MAKE_OPTIONS} -f nbproject/Makefile-PIC16F1939.mk dist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=16F1939
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1270477542/catgenie120.p1: ../common/catgenie120.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/catgenie120.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/catgenie120.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/catgenie120.p1: > ${OBJECTDIR}/_ext/1270477542/catgenie120.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/catgenie120.dep >> ${OBJECTDIR}/_ext/1270477542/catgenie120.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/catgenie120.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/catsensor.p1: ../common/catsensor.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/catsensor.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/catsensor.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/catsensor.p1: > ${OBJECTDIR}/_ext/1270477542/catsensor.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/catsensor.dep >> ${OBJECTDIR}/_ext/1270477542/catsensor.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/catsensor.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline.p1: ../common/cmdline.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/rtc.p1: ../common/rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/rtc.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/rtc.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/rtc.p1: > ${OBJECTDIR}/_ext/1270477542/rtc.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/rtc.dep >> ${OBJECTDIR}/_ext/1270477542/rtc.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/rtc.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/serial.p1: ../common/serial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/serial.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/serial.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/serial.p1: > ${OBJECTDIR}/_ext/1270477542/serial.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/serial.dep >> ${OBJECTDIR}/_ext/1270477542/serial.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/serial.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/timer.p1: ../common/timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/timer.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/timer.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/timer.p1: > ${OBJECTDIR}/_ext/1270477542/timer.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/timer.dep >> ${OBJECTDIR}/_ext/1270477542/timer.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/timer.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/water.p1: ../common/water.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/water.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/water.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/water.p1: > ${OBJECTDIR}/_ext/1270477542/water.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/water.dep >> ${OBJECTDIR}/_ext/1270477542/water.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/water.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/bluetooth.p1: ../common/bluetooth.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/bluetooth.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/bluetooth.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/bluetooth.p1: > ${OBJECTDIR}/_ext/1270477542/bluetooth.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/bluetooth.dep >> ${OBJECTDIR}/_ext/1270477542/bluetooth.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/bluetooth.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline_box.p1: ../common/cmdline_box.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline_box.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline_box.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline_box.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline_box.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1: ../common/cmdline_gpio.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline_gpio.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline_gpio.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1: ../common/cmdline_tag.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline_tag.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline_tag.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline_tag.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cr14.p1: ../common/cr14.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cr14.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cr14.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cr14.p1: > ${OBJECTDIR}/_ext/1270477542/cr14.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cr14.dep >> ${OBJECTDIR}/_ext/1270477542/cr14.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cr14.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/i2c.p1: ../common/i2c.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/i2c.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/i2c.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/i2c.p1: > ${OBJECTDIR}/_ext/1270477542/i2c.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/i2c.dep >> ${OBJECTDIR}/_ext/1270477542/i2c.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/i2c.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/eventlog.p1: ../common/eventlog.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/eventlog.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/eventlog.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/eventlog.p1: > ${OBJECTDIR}/_ext/1270477542/eventlog.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/eventlog.dep >> ${OBJECTDIR}/_ext/1270477542/eventlog.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/eventlog.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/catgenius.p1: catgenius.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 catgenius.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  catgenius.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/catgenius.p1: > ${OBJECTDIR}/catgenius.p1.d
	@cat ${OBJECTDIR}/catgenius.dep >> ${OBJECTDIR}/catgenius.p1.d
	@${FIXDEPS} "${OBJECTDIR}/catgenius.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/litterlanguage.p1: litterlanguage.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 litterlanguage.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  litterlanguage.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/litterlanguage.p1: > ${OBJECTDIR}/litterlanguage.p1.d
	@cat ${OBJECTDIR}/litterlanguage.dep >> ${OBJECTDIR}/litterlanguage.p1.d
	@${FIXDEPS} "${OBJECTDIR}/litterlanguage.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/romwashprogram.p1: romwashprogram.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 romwashprogram.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  romwashprogram.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/romwashprogram.p1: > ${OBJECTDIR}/romwashprogram.p1.d
	@cat ${OBJECTDIR}/romwashprogram.dep >> ${OBJECTDIR}/romwashprogram.p1.d
	@${FIXDEPS} "${OBJECTDIR}/romwashprogram.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/userinterface.p1: userinterface.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 userinterface.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  userinterface.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/userinterface.p1: > ${OBJECTDIR}/userinterface.p1.d
	@cat ${OBJECTDIR}/userinterface.dep >> ${OBJECTDIR}/userinterface.p1.d
	@${FIXDEPS} "${OBJECTDIR}/userinterface.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/_ext/1270477542/catgenie120.p1: ../common/catgenie120.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/catgenie120.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/catgenie120.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/catgenie120.p1: > ${OBJECTDIR}/_ext/1270477542/catgenie120.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/catgenie120.dep >> ${OBJECTDIR}/_ext/1270477542/catgenie120.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/catgenie120.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/catsensor.p1: ../common/catsensor.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/catsensor.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/catsensor.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/catsensor.p1: > ${OBJECTDIR}/_ext/1270477542/catsensor.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/catsensor.dep >> ${OBJECTDIR}/_ext/1270477542/catsensor.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/catsensor.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline.p1: ../common/cmdline.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/rtc.p1: ../common/rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/rtc.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/rtc.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/rtc.p1: > ${OBJECTDIR}/_ext/1270477542/rtc.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/rtc.dep >> ${OBJECTDIR}/_ext/1270477542/rtc.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/rtc.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/serial.p1: ../common/serial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/serial.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/serial.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/serial.p1: > ${OBJECTDIR}/_ext/1270477542/serial.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/serial.dep >> ${OBJECTDIR}/_ext/1270477542/serial.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/serial.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/timer.p1: ../common/timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/timer.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/timer.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/timer.p1: > ${OBJECTDIR}/_ext/1270477542/timer.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/timer.dep >> ${OBJECTDIR}/_ext/1270477542/timer.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/timer.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/water.p1: ../common/water.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/water.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/water.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/water.p1: > ${OBJECTDIR}/_ext/1270477542/water.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/water.dep >> ${OBJECTDIR}/_ext/1270477542/water.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/water.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/bluetooth.p1: ../common/bluetooth.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/bluetooth.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/bluetooth.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/bluetooth.p1: > ${OBJECTDIR}/_ext/1270477542/bluetooth.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/bluetooth.dep >> ${OBJECTDIR}/_ext/1270477542/bluetooth.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/bluetooth.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline_box.p1: ../common/cmdline_box.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline_box.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline_box.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline_box.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline_box.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline_box.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1: ../common/cmdline_gpio.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline_gpio.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline_gpio.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline_gpio.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1: ../common/cmdline_tag.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cmdline_tag.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cmdline_tag.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1: > ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cmdline_tag.dep >> ${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cmdline_tag.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/cr14.p1: ../common/cr14.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/cr14.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/cr14.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/cr14.p1: > ${OBJECTDIR}/_ext/1270477542/cr14.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/cr14.dep >> ${OBJECTDIR}/_ext/1270477542/cr14.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/cr14.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/i2c.p1: ../common/i2c.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/i2c.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/i2c.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/i2c.p1: > ${OBJECTDIR}/_ext/1270477542/i2c.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/i2c.dep >> ${OBJECTDIR}/_ext/1270477542/i2c.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/i2c.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1270477542/eventlog.p1: ../common/eventlog.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1270477542 
	${MP_CC} --pass1 ../common/eventlog.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  ../common/eventlog.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}\_ext\1270477542" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/_ext/1270477542/eventlog.p1: > ${OBJECTDIR}/_ext/1270477542/eventlog.p1.d
	@cat ${OBJECTDIR}/_ext/1270477542/eventlog.dep >> ${OBJECTDIR}/_ext/1270477542/eventlog.p1.d
	@${FIXDEPS} "${OBJECTDIR}/_ext/1270477542/eventlog.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/catgenius.p1: catgenius.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 catgenius.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  catgenius.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/catgenius.p1: > ${OBJECTDIR}/catgenius.p1.d
	@cat ${OBJECTDIR}/catgenius.dep >> ${OBJECTDIR}/catgenius.p1.d
	@${FIXDEPS} "${OBJECTDIR}/catgenius.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/litterlanguage.p1: litterlanguage.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 litterlanguage.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  litterlanguage.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/litterlanguage.p1: > ${OBJECTDIR}/litterlanguage.p1.d
	@cat ${OBJECTDIR}/litterlanguage.dep >> ${OBJECTDIR}/litterlanguage.p1.d
	@${FIXDEPS} "${OBJECTDIR}/litterlanguage.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/romwashprogram.p1: romwashprogram.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 romwashprogram.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  romwashprogram.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/romwashprogram.p1: > ${OBJECTDIR}/romwashprogram.p1.d
	@cat ${OBJECTDIR}/romwashprogram.dep >> ${OBJECTDIR}/romwashprogram.p1.d
	@${FIXDEPS} "${OBJECTDIR}/romwashprogram.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/userinterface.p1: userinterface.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	${MP_CC} --pass1 userinterface.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@${MP_CC} --scandep  userinterface.c $(MP_EXTRA_CC_PRE) -q --chip=$(MP_PROCESSOR_OPTION) -P  --outdir="${OBJECTDIR}" -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0 --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --opt=default,+asm,-asmfile,-speed,+space,-debug,9  --double=24 --float=24 --addrqual=ignore --mode=pro -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s"
	@echo ${OBJECTDIR}/userinterface.p1: > ${OBJECTDIR}/userinterface.p1.d
	@cat ${OBJECTDIR}/userinterface.dep >> ${OBJECTDIR}/userinterface.p1.d
	@${FIXDEPS} "${OBJECTDIR}/userinterface.p1.d" $(SILENT) -ht  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) -odist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  -mdist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.map --summary=default,-psect,-class,+mem,-hex --chip=$(MP_PROCESSOR_OPTION) -P --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -D__DEBUG --debugger=pickit3 -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0  --double=24 --float=24 --addrqual=ignore --mode=pro --output=default,-inhx032 -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s" ${OBJECTFILES_QUOTED_IF_SPACED}  
	@${RM} dist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.hex
else
dist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) -odist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  -mdist/${CND_CONF}/${IMAGE_TYPE}/catgenius.${IMAGE_TYPE}.map --summary=default,-psect,-class,+mem,-hex --chip=$(MP_PROCESSOR_OPTION) -P --runtime=default,+clear,+init,-keep,+osccal,+resetbits,-download,+stackcall,+clib --summary=default,-psect,-class,+mem,-hex --opt=default,+asm,-asmfile,-speed,+space,-debug,9 -DHW_CATGENIE120PLUS -DAPP_CATGENIUS -P -N31 -I"C:/Program Files (x86)/HI-TECH Software/PICC/9.83/include" -I"." -I"../common" --warn=0  --double=24 --float=24 --addrqual=ignore --mode=pro --output=default,-inhx032 -g --asmlist "--errformat=%%f:%%l: error: %%s" "--msgformat=%%f:%%l: advisory: %%s" "--warnformat=%%f:%%l warning: %%s" ${OBJECTFILES_QUOTED_IF_SPACED}  
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/PIC16F1939
	${RM} -r dist/PIC16F1939

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
