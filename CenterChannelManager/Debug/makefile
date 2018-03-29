################################################################################
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include tinyXML/subdir.mk
-include build/Release/GNU-Linux-x86/tinyXML/subdir.mk
-include build/Release/GNU-Linux-x86/subdir.mk
-include build/Release/GNU-Linux-x86/Log/subdir.mk
-include build/Debug/GNU-Linux-x86/tinyXML/subdir.mk
-include build/Debug/GNU-Linux-x86/subdir.mk
-include build/Debug/GNU-Linux-x86/Log/subdir.mk
-include Log/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# 所有目标
all: CenterChannelManager

# 工具调用
CenterChannelManager: $(OBJS) $(USER_OBJS)
	@echo '正在构建目标： $@'
	@echo '正在调用： GNU C++ 链接器'
	g++ -L/root/jiezisoft/tools/ACE_wrappers -L/usr/lib/mysql/ -o"CenterChannelManager" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo '已结束构建目标： $@'
	@echo ' '

# 其他目标
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) CenterChannelManager
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
