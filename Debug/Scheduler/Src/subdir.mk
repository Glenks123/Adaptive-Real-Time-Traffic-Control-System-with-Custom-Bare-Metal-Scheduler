################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Scheduler/Src/scheduler.c 

OBJS += \
./Scheduler/Src/scheduler.o 

C_DEPS += \
./Scheduler/Src/scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
Scheduler/Src/%.o Scheduler/Src/%.su Scheduler/Src/%.cyclo: ../Scheduler/Src/%.c Scheduler/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Scheduler-2f-Src

clean-Scheduler-2f-Src:
	-$(RM) ./Scheduler/Src/scheduler.cyclo ./Scheduler/Src/scheduler.d ./Scheduler/Src/scheduler.o ./Scheduler/Src/scheduler.su

.PHONY: clean-Scheduler-2f-Src

