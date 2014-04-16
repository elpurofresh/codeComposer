# default to compiling with debug information
CCOPTS ?= -g --symdebug:dwarf --optimize_with_debug -qq -pdsw225 

# required C preprocessor flags
CPPFLAGS = -D__MSP430G2553__ -I C:/ti/ccsv5/tools/compiler/msp430_4.1.1/include -I "C:/ti/grace_1_10_04_36/packages"  -I "C:/ti/grace_1_10_04_36/packages/ti/mcu/msp430//include"

# the compiler and archiver commands
CC = C:/ti/ccsv5/tools/compiler/msp430_4.1.1/bin/cl430 -c --obj_directory=objs/  -vmsp
AR = C:/ti/ccsv5/tools/compiler/msp430_4.1.1/bin/ar430 rq
RM = cmd.exe /q /c del /f
MKDIR = mkdir

# create object sub-directory, if necessary
ifeq (,$(wildcard objs/))
    $(shell $(MKDIR) objs\)
endif

all: csl.lib

objs\CSL_init.obj: CSL_init.c makefile
	@echo cl430 $< ...
	$(CC) $(CCOPTS) $(CPPFLAGS) $<

objs\GPIO_init.obj: GPIO_init.c makefile
	@echo cl430 $< ...
	$(CC) $(CCOPTS) $(CPPFLAGS) $<

objs\BCSplus_init.obj: BCSplus_init.c makefile
	@echo cl430 $< ...
	$(CC) $(CCOPTS) $(CPPFLAGS) $<

objs\USCI_B0_init.obj: USCI_B0_init.c makefile
	@echo cl430 $< ...
	$(CC) $(CCOPTS) $(CPPFLAGS) $<

objs\System_init.obj: System_init.c makefile
	@echo cl430 $< ...
	$(CC) $(CCOPTS) $(CPPFLAGS) $<

objs\WDTplus_init.obj: WDTplus_init.c makefile
	@echo cl430 $< ...
	$(CC) $(CCOPTS) $(CPPFLAGS) $<

csl.lib: objs\CSL_init.obj objs\GPIO_init.obj objs\BCSplus_init.obj objs\USCI_B0_init.obj objs\System_init.obj objs\WDTplus_init.obj
	@ echo ar430 $@ $< ...
	$(AR) $@ $^

clean:
	-@$(RM) csl.lib
	-@$(RM) objs\CSL_init.obj
	-@$(RM) objs\GPIO_init.obj
	-@$(RM) objs\BCSplus_init.obj
	-@$(RM) objs\USCI_B0_init.obj
	-@$(RM) objs\System_init.obj
	-@$(RM) objs\WDTplus_init.obj



