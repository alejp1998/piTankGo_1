################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fsm.c \
../piTankGo_1.c \
../player.c \
../ruedas.c \
../tmr.c \
../torreta.c \
../xbox360.c 

OBJS += \
./fsm.o \
./piTankGo_1.o \
./player.o \
./ruedas.o \
./tmr.o \
./torreta.o \
./xbox360.o 

C_DEPS += \
./fsm.d \
./piTankGo_1.d \
./player.d \
./ruedas.d \
./tmr.d \
./torreta.d \
./xbox360.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\SysGCC\Raspberry\include" -I"C:\SysGCC\Raspberry\include\wiringPi" -O0 -g3 -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


