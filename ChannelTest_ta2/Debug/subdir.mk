################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CascadeCmdHandler.cpp \
../CmdHandler_T.cpp \
../Cmd_Acceptor.cpp \
../ControlCmdHandler.cpp \
../DeviceStatus.cpp \
../MSG_Center.cpp \
../MutexGuard.cpp \
../MyLog.cpp \
../SimpleConfig.cpp \
../SysMutex.cpp \
../SysUtil.cpp \
../main.cpp 

OBJS += \
./CascadeCmdHandler.o \
./CmdHandler_T.o \
./Cmd_Acceptor.o \
./ControlCmdHandler.o \
./DeviceStatus.o \
./MSG_Center.o \
./MutexGuard.o \
./MyLog.o \
./SimpleConfig.o \
./SysMutex.o \
./SysUtil.o \
./main.o 

CPP_DEPS += \
./CascadeCmdHandler.d \
./CmdHandler_T.d \
./Cmd_Acceptor.d \
./ControlCmdHandler.d \
./DeviceStatus.d \
./MSG_Center.d \
./MutexGuard.d \
./MyLog.d \
./SimpleConfig.d \
./SysMutex.d \
./SysUtil.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -I/root/jiezisoft/tools/ACE_wrappers -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


