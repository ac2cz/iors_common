################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/agw_tnc.c \
../src/ax25_tools.c \
../src/crc.c \
../src/hmac_sha256.c \
../src/iors_command.c \
../src/keyfile.c \
../src/sha256.c \
../src/str_util.c 

C_DEPS += \
./src/agw_tnc.d \
./src/ax25_tools.d \
./src/crc.d \
./src/hmac_sha256.d \
./src/iors_command.d \
./src/keyfile.d \
./src/sha256.d \
./src/str_util.d 

OBJS += \
./src/agw_tnc.o \
./src/ax25_tools.o \
./src/crc.o \
./src/hmac_sha256.o \
./src/iors_command.o \
./src/keyfile.o \
./src/sha256.o \
./src/str_util.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../inc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/agw_tnc.d ./src/agw_tnc.o ./src/ax25_tools.d ./src/ax25_tools.o ./src/crc.d ./src/crc.o ./src/hmac_sha256.d ./src/hmac_sha256.o ./src/iors_command.d ./src/iors_command.o ./src/keyfile.d ./src/keyfile.o ./src/sha256.d ./src/sha256.o ./src/str_util.d ./src/str_util.o

.PHONY: clean-src

