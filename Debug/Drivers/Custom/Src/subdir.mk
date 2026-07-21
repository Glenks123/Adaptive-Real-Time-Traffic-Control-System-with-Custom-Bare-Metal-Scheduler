################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Custom/Src/systick.c \
../Drivers/Custom/Src/uart.c 

OBJS += \
./Drivers/Custom/Src/systick.o \
./Drivers/Custom/Src/uart.o 

C_DEPS += \
./Drivers/Custom/Src/systick.d \
./Drivers/Custom/Src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Custom/Src/%.o Drivers/Custom/Src/%.su Drivers/Custom/Src/%.cyclo: ../Drivers/Custom/Src/%.c Drivers/Custom/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Custom-2f-Src

clean-Drivers-2f-Custom-2f-Src:
	-$(RM) ./Drivers/Custom/Src/systick.cyclo ./Drivers/Custom/Src/systick.d ./Drivers/Custom/Src/systick.o ./Drivers/Custom/Src/systick.su ./Drivers/Custom/Src/uart.cyclo ./Drivers/Custom/Src/uart.d ./Drivers/Custom/Src/uart.o ./Drivers/Custom/Src/uart.su

.PHONY: clean-Drivers-2f-Custom-2f-Src

