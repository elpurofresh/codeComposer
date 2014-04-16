################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Library/CTS_HAL.obj: ../Library/CTS_HAL.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv5/tools/compiler/msp430/bin/cl430" -vmsp --abi=coffabi -O0 -g --include_path="C:/ti/ccsv5/ccs_base/msp430/include" --include_path="C:/andres/Code/codeComposer/MSP430_LaunchPad/Labs/Lab10b-2553" --include_path="C:/andres/Code/codeComposer/MSP430_LaunchPad/Labs/Lab10b-2553/Library" --include_path="C:/ti/ccsv5/tools/compiler/msp430/include" --gcc --define=__MSP430G2553__ --diag_warning=225 --display_error_number --printf_support=minimal --preproc_with_compile --preproc_dependency="Library/CTS_HAL.pp" --obj_directory="Library" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Library/CTS_Layer.obj: ../Library/CTS_Layer.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv5/tools/compiler/msp430/bin/cl430" -vmsp --abi=coffabi -O0 -g --include_path="C:/ti/ccsv5/ccs_base/msp430/include" --include_path="C:/andres/Code/codeComposer/MSP430_LaunchPad/Labs/Lab10b-2553" --include_path="C:/andres/Code/codeComposer/MSP430_LaunchPad/Labs/Lab10b-2553/Library" --include_path="C:/ti/ccsv5/tools/compiler/msp430/include" --gcc --define=__MSP430G2553__ --diag_warning=225 --display_error_number --printf_support=minimal --preproc_with_compile --preproc_dependency="Library/CTS_Layer.pp" --obj_directory="Library" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


