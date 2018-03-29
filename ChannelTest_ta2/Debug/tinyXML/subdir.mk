################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tinyXML/tinystr.cpp \
../tinyXML/tinyxml.cpp \
../tinyXML/tinyxmlerror.cpp \
../tinyXML/tinyxmlparser.cpp \
../tinyXML/xml_config.cpp 

OBJS += \
./tinyXML/tinystr.o \
./tinyXML/tinyxml.o \
./tinyXML/tinyxmlerror.o \
./tinyXML/tinyxmlparser.o \
./tinyXML/xml_config.o 

CPP_DEPS += \
./tinyXML/tinystr.d \
./tinyXML/tinyxml.d \
./tinyXML/tinyxmlerror.d \
./tinyXML/tinyxmlparser.d \
./tinyXML/xml_config.d 


# Each subdirectory must supply rules for building sources it contributes
tinyXML/%.o: ../tinyXML/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -I/root/jiezisoft/tools/ACE_wrappers -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


