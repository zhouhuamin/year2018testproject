#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CascadeCmdHandler.o \
	${OBJECTDIR}/CmdHandler_T.o \
	${OBJECTDIR}/Cmd_Acceptor.o \
	${OBJECTDIR}/ControlCmdHandler.o \
	${OBJECTDIR}/DeviceStatus.o \
	${OBJECTDIR}/Log/FreeLongLog.o \
	${OBJECTDIR}/MSG_Center.o \
	${OBJECTDIR}/MutexGuard.o \
	${OBJECTDIR}/MyLog.o \
	${OBJECTDIR}/SimpleConfig.o \
	${OBJECTDIR}/SysMutex.o \
	${OBJECTDIR}/SysUtil.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/tinyXML/tinystr.o \
	${OBJECTDIR}/tinyXML/tinyxml.o \
	${OBJECTDIR}/tinyXML/tinyxmlerror.o \
	${OBJECTDIR}/tinyXML/tinyxmlparser.o \
	${OBJECTDIR}/tinyXML/xml_config.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/channeltest

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/channeltest: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/channeltest ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CascadeCmdHandler.o: CascadeCmdHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CascadeCmdHandler.o CascadeCmdHandler.cpp

${OBJECTDIR}/CmdHandler_T.o: CmdHandler_T.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CmdHandler_T.o CmdHandler_T.cpp

${OBJECTDIR}/Cmd_Acceptor.o: Cmd_Acceptor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Cmd_Acceptor.o Cmd_Acceptor.cpp

${OBJECTDIR}/ControlCmdHandler.o: ControlCmdHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ControlCmdHandler.o ControlCmdHandler.cpp

${OBJECTDIR}/DeviceStatus.o: DeviceStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DeviceStatus.o DeviceStatus.cpp

${OBJECTDIR}/Log/FreeLongLog.o: Log/FreeLongLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/Log
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Log/FreeLongLog.o Log/FreeLongLog.cpp

${OBJECTDIR}/MSG_Center.o: MSG_Center.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MSG_Center.o MSG_Center.cpp

${OBJECTDIR}/MutexGuard.o: MutexGuard.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MutexGuard.o MutexGuard.cpp

${OBJECTDIR}/MyLog.o: MyLog.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MyLog.o MyLog.cpp

${OBJECTDIR}/SimpleConfig.o: SimpleConfig.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SimpleConfig.o SimpleConfig.cpp

${OBJECTDIR}/SysMutex.o: SysMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysMutex.o SysMutex.cpp

${OBJECTDIR}/SysUtil.o: SysUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysUtil.o SysUtil.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/tinyXML/tinystr.o: tinyXML/tinystr.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinystr.o tinyXML/tinystr.cpp

${OBJECTDIR}/tinyXML/tinyxml.o: tinyXML/tinyxml.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxml.o tinyXML/tinyxml.cpp

${OBJECTDIR}/tinyXML/tinyxmlerror.o: tinyXML/tinyxmlerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxmlerror.o tinyXML/tinyxmlerror.cpp

${OBJECTDIR}/tinyXML/tinyxmlparser.o: tinyXML/tinyxmlparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxmlparser.o tinyXML/tinyxmlparser.cpp

${OBJECTDIR}/tinyXML/xml_config.o: tinyXML/xml_config.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/xml_config.o tinyXML/xml_config.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/channeltest

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
