################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../lnk_msp430g2553.cmd 

CFG_SRCS += \
../msp430g2xx3_usci_i2c_slave_rx.cfg 

C_SRCS += \
../msp430g2xx3_usci_i2c_slave_rx.c 

OBJS += \
./msp430g2xx3_usci_i2c_slave_rx.obj 

GEN_SRCS += \
./configPkg/compiler.opt \
./configPkg/linker.cmd 

C_DEPS += \
./msp430g2xx3_usci_i2c_slave_rx.pp 

GEN_MISC_DIRS += \
./configPkg/ 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_OPTS += \
./configPkg/compiler.opt 

GEN_SRCS__QUOTED += \
"configPkg\compiler.opt" \
"configPkg\linker.cmd" 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

C_DEPS__QUOTED += \
"msp430g2xx3_usci_i2c_slave_rx.pp" 

OBJS__QUOTED += \
"msp430g2xx3_usci_i2c_slave_rx.obj" 

C_SRCS__QUOTED += \
"../msp430g2xx3_usci_i2c_slave_rx.c" 

CFG_SRCS__QUOTED += \
"../msp430g2xx3_usci_i2c_slave_rx.cfg" 

GEN_OPTS__FLAG += \
--cmd_file="./configPkg/compiler.opt" 

GEN_CMDS__FLAG += \
-l"./configPkg/linker.cmd" 


