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
	${OBJECTDIR}/ControlCmdHandler.o \
	${OBJECTDIR}/RecoClientSDK.o \
	${OBJECTDIR}/SysMutex.o \
	${OBJECTDIR}/SysUtil.o


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
LDLIBSOPTIONS=/usr/lib/libpthread.so

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libRecoClientSDK.${CND_DLIB_EXT}

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libRecoClientSDK.${CND_DLIB_EXT}: /usr/lib/libpthread.so

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libRecoClientSDK.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libRecoClientSDK.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/ControlCmdHandler.o: ControlCmdHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ControlCmdHandler.o ControlCmdHandler.cpp

${OBJECTDIR}/RecoClientSDK.o: RecoClientSDK.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RecoClientSDK.o RecoClientSDK.cpp

${OBJECTDIR}/SysMutex.o: SysMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysMutex.o SysMutex.cpp

${OBJECTDIR}/SysUtil.o: SysUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysUtil.o SysUtil.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libRecoClientSDK.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
