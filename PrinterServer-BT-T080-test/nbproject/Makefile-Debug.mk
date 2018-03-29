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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CRC.o \
	${OBJECTDIR}/CascadeCmdHandler.o \
	${OBJECTDIR}/MSGHandleCenter.o \
	${OBJECTDIR}/ProcessBroadcast.o \
	${OBJECTDIR}/SysMutex.o \
	${OBJECTDIR}/SysUtil.o \
	${OBJECTDIR}/broadcast.o \
	${OBJECTDIR}/common.o \
	${OBJECTDIR}/config.o \
	${OBJECTDIR}/dev.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/net_handle.o \
	${OBJECTDIR}/port.o \
	${OBJECTDIR}/reader1000api.o \
	${OBJECTDIR}/starup_handle.o \
	${OBJECTDIR}/tinyxml/tinystr.o \
	${OBJECTDIR}/tinyxml/tinyxml.o \
	${OBJECTDIR}/tinyxml/tinyxmlerror.o \
	${OBJECTDIR}/tinyxml/tinyxmlparser.o


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
LDLIBSOPTIONS=-lpthread -lxml2 -lzmq -lACE

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/printerserver

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/printerserver: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/printerserver ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CRC.o: CRC.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CRC.o CRC.cpp

${OBJECTDIR}/CascadeCmdHandler.o: CascadeCmdHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CascadeCmdHandler.o CascadeCmdHandler.cpp

${OBJECTDIR}/MSGHandleCenter.o: MSGHandleCenter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MSGHandleCenter.o MSGHandleCenter.cpp

${OBJECTDIR}/ProcessBroadcast.o: ProcessBroadcast.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ProcessBroadcast.o ProcessBroadcast.cpp

${OBJECTDIR}/SysMutex.o: SysMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysMutex.o SysMutex.cpp

${OBJECTDIR}/SysUtil.o: SysUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysUtil.o SysUtil.cpp

${OBJECTDIR}/broadcast.o: broadcast.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/broadcast.o broadcast.cpp

${OBJECTDIR}/common.o: common.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/common.o common.cpp

${OBJECTDIR}/config.o: config.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/config.o config.cpp

${OBJECTDIR}/dev.o: dev.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dev.o dev.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/net_handle.o: net_handle.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/net_handle.o net_handle.cpp

${OBJECTDIR}/port.o: port.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/port.o port.cpp

${OBJECTDIR}/reader1000api.o: reader1000api.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/reader1000api.o reader1000api.cpp

${OBJECTDIR}/starup_handle.o: starup_handle.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/starup_handle.o starup_handle.cpp

${OBJECTDIR}/tinyxml/tinystr.o: tinyxml/tinystr.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinystr.o tinyxml/tinystr.cpp

${OBJECTDIR}/tinyxml/tinyxml.o: tinyxml/tinyxml.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinyxml.o tinyxml/tinyxml.cpp

${OBJECTDIR}/tinyxml/tinyxmlerror.o: tinyxml/tinyxmlerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinyxmlerror.o tinyxml/tinyxmlerror.cpp

${OBJECTDIR}/tinyxml/tinyxmlparser.o: tinyxml/tinyxmlparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinyxmlparser.o tinyxml/tinyxmlparser.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/printerserver

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
