################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/carpincho.c \
../src/memoria.c \
../src/servidor.c \
../src/swap.c \
../src/tlb.c 

OBJS += \
./src/carpincho.o \
./src/memoria.o \
./src/servidor.o \
./src/swap.o \
./src/tlb.o 

C_DEPS += \
./src/carpincho.d \
./src/memoria.d \
./src/servidor.d \
./src/swap.d \
./src/tlb.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


