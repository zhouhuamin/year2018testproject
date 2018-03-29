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
	${OBJECTDIR}/CascadeCmdHandler.o \
	${OBJECTDIR}/MSGHandleCenter.o \
	${OBJECTDIR}/RFID_label.o \
	${OBJECTDIR}/SysMutex.o \
	${OBJECTDIR}/SysUtil.o \
	${OBJECTDIR}/common.o \
	${OBJECTDIR}/config.o \
	${OBJECTDIR}/dev.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/net_handle.o \
	${OBJECTDIR}/port.o \
	${OBJECTDIR}/reader1000api.o \
	${OBJECTDIR}/starup_handle.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /root/jiezisoft/bin/Debug/GNU-Linux-x86/etagservertest

/root/jiezisoft/bin/Debug/GNU-Linux-x86/etagservertest: ${OBJECTFILES}
	${MKDIR} -p /root/jiezisoft/bin/Debug/GNU-Linux-x86
	${LINK.cc} -o /root/jiezisoft/bin/Debug/GNU-Linux-x86/etagservertest ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CascadeCmdHandler.o: CascadeCmdHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CascadeCmdHandler.o CascadeCmdHandler.cpp

${OBJECTDIR}/MSGHandleCenter.o: MSGHandleCenter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MSGHandleCenter.o MSGHandleCenter.cpp

${OBJECTDIR}/RFID_label.o: RFID_label.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RFID_label.o RFID_label.cpp

${OBJECTDIR}/SysMutex.o: SysMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysMutex.o SysMutex.cpp

${OBJECTDIR}/SysUtil.o: SysUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../tools/ACE_wrappers -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysUtil.o SysUtil.cpp

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

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /root/jiezisoft/bin/Debug/GNU-Linux-x86/etagservertest

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
