################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Comm/PHY_DataLink/TI_MSPBoot_CI_PHYDL_USCI_UART.obj: C:/Users/ENDLaptop/Desktop/Embedded\ Nerd\ Design/Bright\ Leaf\ Power/svn/pca\ testers/TestFirmware/bootloaders/F5529bootloader/src/Comm/PHY_DataLink/TI_MSPBoot_CI_PHYDL_USCI_UART.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv5/tools/compiler/msp430_4.2.1/bin/cl430" -vmspx --abi=eabi -O2 --include_path="C:/ti/ccsv5/ccs_base/msp430/include" --include_path="C:/ti/ccsv5/tools/compiler/msp430_4.2.1/include" --include_path="C:/Users/ENDLaptop/Desktop/Embedded Nerd Design/Bright Leaf Power/svn/pca testers/TestFirmware/bootloaders/F5529bootloader/src/" --include_path="C:/Users/ENDLaptop/Desktop/Embedded Nerd Design/Bright Leaf Power/svn/pca testers/TestFirmware/bootloaders/F5529bootloader/src/MI" --include_path="C:/Users/ENDLaptop/Desktop/Embedded Nerd Design/Bright Leaf Power/svn/pca testers/TestFirmware/bootloaders/F5529bootloader/src/Comm" --include_path="C:/Users/ENDLaptop/Desktop/Embedded Nerd Design/Bright Leaf Power/svn/pca testers/TestFirmware/bootloaders/F5529bootloader/src/AppMgr" --advice:power_severity=suppress --advice:power="all" --gcc --define=MSPBoot_BSL --define=__MSP430F5529__ --diag_warning=225 --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="Comm/PHY_DataLink/TI_MSPBoot_CI_PHYDL_USCI_UART.pp" --obj_directory="Comm/PHY_DataLink" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


