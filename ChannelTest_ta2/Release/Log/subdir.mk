################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Log/FreeLongLog.cpp \
../Log/FreeLongSourceRegister.cpp 

OBJS += \
./Log/FreeLongLog.o \
./Log/FreeLongSourceRegister.o 

CPP_DEPS += \
./Log/FreeLongLog.d \
./Log/FreeLongSourceRegister.d 


# Each subdirectory must supply rules for building sources it contributes
Log/%.o: ../Log/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -I/root/jiezisoft/tools/ACE_wrappers -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


