################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CmdHandler_T.cpp \
../Cmd_Acceptor.cpp \
../ControlCmdHandler.cpp \
../MSG_Center.cpp \
../MutexGuard.cpp \
../MyLog.cpp \
../MySQLConnectionPool.cpp \
../PlatCmdHandler_T.cpp \
../Plat_Cmd_Acceptor.cpp \
../SimpleConfig.cpp \
../SysMutex.cpp \
../SysUtil.cpp \
../cppmysql.cpp \
../main.cpp 

OBJS += \
./CmdHandler_T.o \
./Cmd_Acceptor.o \
./ControlCmdHandler.o \
./MSG_Center.o \
./MutexGuard.o \
./MyLog.o \
./MySQLConnectionPool.o \
./PlatCmdHandler_T.o \
./Plat_Cmd_Acceptor.o \
./SimpleConfig.o \
./SysMutex.o \
./SysUtil.o \
./cppmysql.o \
./main.o 

CPP_DEPS += \
./CmdHandler_T.d \
./Cmd_Acceptor.d \
./ControlCmdHandler.d \
./MSG_Center.d \
./MutexGuard.d \
./MyLog.d \
./MySQLConnectionPool.d \
./PlatCmdHandler_T.d \
./Plat_Cmd_Acceptor.d \
./SimpleConfig.d \
./SysMutex.d \
./SysUtil.d \
./cppmysql.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -I/root/jiezisoft/tools/ACE_wrappers -I/usr/include/mysql -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


