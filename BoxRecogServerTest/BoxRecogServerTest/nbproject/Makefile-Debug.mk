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
	${OBJECTDIR}/BoxNumberCheckAlgo.o \
	${OBJECTDIR}/CascadeCmdHandler.o \
	${OBJECTDIR}/Log/FreeLongLog.o \
	${OBJECTDIR}/MSGHandleCenter.o \
	${OBJECTDIR}/MutexGuard.o \
	${OBJECTDIR}/MyLog.o \
	${OBJECTDIR}/ProcessImage.o \
	${OBJECTDIR}/RTUProtocol.o \
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
LDLIBSOPTIONS=../../../../tools/ACE_wrappers/lib/libACE.so /usr/lib/libRecoClientSDK.so /usr/local/lib/libzmq.so /usr/lib/libJZContaCheckSDK.so

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest

/root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest: ../../../../tools/ACE_wrappers/lib/libACE.so

/root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest: /usr/lib/libRecoClientSDK.so

/root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest: /usr/local/lib/libzmq.so

/root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest: /usr/lib/libJZContaCheckSDK.so

/root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest: ${OBJECTFILES}
	${MKDIR} -p /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/BoxNumberCheckAlgo.o: BoxNumberCheckAlgo.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/BoxNumberCheckAlgo.o BoxNumberCheckAlgo.cpp

${OBJECTDIR}/CascadeCmdHandler.o: CascadeCmdHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CascadeCmdHandler.o CascadeCmdHandler.cpp

${OBJECTDIR}/Log/FreeLongLog.o: Log/FreeLongLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/Log
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Log/FreeLongLog.o Log/FreeLongLog.cpp

${OBJECTDIR}/MSGHandleCenter.o: MSGHandleCenter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MSGHandleCenter.o MSGHandleCenter.cpp

${OBJECTDIR}/MutexGuard.o: MutexGuard.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MutexGuard.o MutexGuard.cpp

${OBJECTDIR}/MyLog.o: MyLog.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MyLog.o MyLog.cpp

${OBJECTDIR}/ProcessImage.o: ProcessImage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ProcessImage.o ProcessImage.cpp

${OBJECTDIR}/RTUProtocol.o: RTUProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTUProtocol.o RTUProtocol.cpp

${OBJECTDIR}/SimpleConfig.o: SimpleConfig.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SimpleConfig.o SimpleConfig.cpp

${OBJECTDIR}/SysMutex.o: SysMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysMutex.o SysMutex.cpp

${OBJECTDIR}/SysUtil.o: SysUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysUtil.o SysUtil.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/tinyXML/tinystr.o: tinyXML/tinystr.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinystr.o tinyXML/tinystr.cpp

${OBJECTDIR}/tinyXML/tinyxml.o: tinyXML/tinyxml.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxml.o tinyXML/tinyxml.cpp

${OBJECTDIR}/tinyXML/tinyxmlerror.o: tinyXML/tinyxmlerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxmlerror.o tinyXML/tinyxmlerror.cpp

${OBJECTDIR}/tinyXML/tinyxmlparser.o: tinyXML/tinyxmlparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxmlparser.o tinyXML/tinyxmlparser.cpp

${OBJECTDIR}/tinyXML/xml_config.o: tinyXML/xml_config.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../../../tools/ACE_wrappers -I/usr/include/mysql -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/xml_config.o tinyXML/xml_config.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/boxrecogservertest

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
