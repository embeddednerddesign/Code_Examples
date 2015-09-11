################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/msp430_4.2.1

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./AppMgr/TI_MSPBoot_VecRed_G2x53.obj" \
"./AppMgr/TI_MSPBoot_AppMgr.obj" \
"./AppMgr/Crc.obj" \
"./Comm/NWK_APP/TI_MSPBoot_CI_NWK_APP_HDLC.obj" \
"./Comm/PHY_DataLink/TI_MSPBoot_CI_PHYDL_USCI_UART.obj" \
"./MI/TI_MSPBoot_MI_Flash.obj" \
"./main.obj" \
"./boot.obj" \
"../Config/lnk_msp430g2553_Config_boot_1KB.cmd" \
-l"libc.a" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include MI/subdir_vars.mk
-include Config/subdir_vars.mk
-include Comm/PHY_DataLink/subdir_vars.mk
-include Comm/NWK_APP/subdir_vars.mk
-include AppMgr/subdir_vars.mk
-include subdir_rules.mk
-include MI/subdir_rules.mk
-include Config/subdir_rules.mk
-include Comm/PHY_DataLink/subdir_rules.mk
-include Comm/NWK_APP/subdir_rules.mk
-include AppMgr/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: G2553bootloader.out

# Tool invocations
G2553bootloader.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP430 Linker'
	"C:/ti/ccsv5/tools/compiler/msp430_4.2.1/bin/cl430" -vmsp --abi=eabi --code_model=small --data_model=small -O0 --opt_for_speed=0 -g --advice:power_severity=suppress --advice:power="all" --gcc --define=MSPBoot_BSL --define=__MSP430G2553__ --diag_warning=225 --display_error_number --printf_support=minimal -z --stack_size=80 -m"G2553bootloader.map" --heap_size=80 -i"C:/ti/ccsv5/ccs_base/msp430/include" -i"C:/ti/ccsv5/tools/compiler/msp430_4.2.1/lib" -i"C:/ti/ccsv5/tools/compiler/msp430_4.2.1/include" --reread_libs --warn_sections --display_error_number --xml_link_info="G2553bootloader_linkInfo.xml" --ram_model -o "G2553bootloader.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(MSP430_EXECUTABLE_OUTPUTS__QUOTED) "G2553bootloader.out"
	-$(RM) "boot.pp" "main.pp" "MI\TI_MSPBoot_MI_Flash.pp" "Comm\PHY_DataLink\TI_MSPBoot_CI_PHYDL_USCI_UART.pp" "Comm\NWK_APP\TI_MSPBoot_CI_NWK_APP_HDLC.pp" "AppMgr\Crc.pp" "AppMgr\TI_MSPBoot_AppMgr.pp" "AppMgr\TI_MSPBoot_VecRed_G2x53.pp" 
	-$(RM) "boot.obj" "main.obj" "MI\TI_MSPBoot_MI_Flash.obj" "Comm\PHY_DataLink\TI_MSPBoot_CI_PHYDL_USCI_UART.obj" "Comm\NWK_APP\TI_MSPBoot_CI_NWK_APP_HDLC.obj" "AppMgr\Crc.obj" "AppMgr\TI_MSPBoot_AppMgr.obj" "AppMgr\TI_MSPBoot_VecRed_G2x53.obj" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-@echo 'Create flash image: TI-TXT'
	-"C:/ti/ccsv5/tools/compiler/msp430_4.2.1/bin/hex430" --ti_txt "G2553bootloader.out" -o "G2553bootloader.txt" -order MS -romwidth 16
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets

