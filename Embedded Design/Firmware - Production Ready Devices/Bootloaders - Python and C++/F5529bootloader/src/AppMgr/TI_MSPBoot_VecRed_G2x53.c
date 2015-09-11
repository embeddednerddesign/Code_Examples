/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

//
// Include files
//
#include "msp430.h"
#include "TI_MSPBoot_Common.h"
#include "TI_MSPBoot_AppMgr.h"

//
//  External variables from linker file
//
extern uint16_t _App_Proxy_Vector_Start[];  /* Proxy table address */
extern uint16_t _App_Isr_USCIA1;
extern uint16_t _App_Isr_TrapIsr;

//
//  Macros and definitions
//
#define UNUSED 0x3FFF       /* Value written to unused vectors */
/*! Macro used to calculate address of vector in Application Proxy Table */
#define APP_PROXY_VECTOR(x)     ((uint16_t)&_App_Proxy_Vector_Start[x*2])
#define APP_ISR_USCIA1_ADDR     ((uint16_t )&_App_Isr_USCIA1)
#define APP_ISR_TRAPISR_ADDR     ((uint16_t )&_App_Isr_TrapIsr)

//
//  Constant table
//
/*! MSPBoot Vector Table: It's fixed since it can't be erased and modified.
 *  Points to a proxy vector table in Application area*/
#   ifdef __IAR_SYSTEMS_ICC__
#       pragma location="BOOT_VECTOR_TABLE"
__root const uint16_t Vector_Table[] =
#   elif defined (__TI_COMPILER_VERSION__)
#       pragma DATA_SECTION(Vector_Table, ".BOOT_VECTOR_TABLE")
#       pragma RETAIN(Vector_Table)
const uint16_t Vector_Table[] =
#   endif
{
    UNUSED,                                     // FF80 = unused
    UNUSED,                                     // FF82 = unused
    UNUSED,                                     // FF84 = unused
    UNUSED,                                     // FF86 = unused
    UNUSED,                                     // FF88 = unused
    UNUSED,                                     // FF8A = unused
    UNUSED,                                     // FF8C = unused
    UNUSED,                                     // FF8E = unused
    UNUSED,                                     // FF90 = unused
    UNUSED,                                     // FF92 = unused
    UNUSED,                                     // FF94 = unused
    UNUSED,                                     // FF96 = unused
    UNUSED,                                     // FF98 = unused
    UNUSED,                                     // FF9A = unused
    UNUSED,                                     // FF9C = unused
    UNUSED,                                     // FF9E = unused
    UNUSED,                                     // FFA0 = unused
    UNUSED,                                     // FFA2 = unused
    UNUSED,                                     // FFA4 = unused
    UNUSED,                                     // FFA6 = unused
    UNUSED,                                     // FFA8 = unused
    UNUSED,                                     // FFAA = unused
    UNUSED,                                     // FFAC = unused
    UNUSED,                                     // FFAE = unused
    UNUSED,                                     // FFB0 = unused
    UNUSED,                                     // FFB2 = unused
    UNUSED,                                     // FFB4 = unused
    UNUSED,                                     // FFB6 = unused
    UNUSED,                                     // FFB8 = unused
    UNUSED,                                     // FFBA = unused
    UNUSED,                                     // FFBC = unused
    UNUSED,                                     // FFBE = unused
    UNUSED,                                     // FFC0 = unused
    UNUSED,                                     // FFC2 = unused
    UNUSED,                                     // FFC4 = unused
    UNUSED,                                     // FFC6 = unused
    UNUSED,                                     // FFC8 = unused
    UNUSED,                                     // FFCA = unused
    UNUSED,                                     // FFCC = unused
    UNUSED,                                     // FFCE = unused
    UNUSED,                                     // FFD0 = unused
    APP_ISR_TRAPISR_ADDR,                        // FFD2 = RTC
    APP_ISR_TRAPISR_ADDR,                        // FFD4 = PORT2
    APP_ISR_TRAPISR_ADDR,                        // FFD6 = TIMER2_A1
    APP_ISR_TRAPISR_ADDR,                        // FFD8 = TIMER2_A0
    APP_ISR_TRAPISR_ADDR,                        // FFDA = USCI_B1
    APP_ISR_USCIA1_ADDR,                        // FFDC = USCI_A1
    APP_ISR_TRAPISR_ADDR,                        // FFDE = PORT1
    APP_ISR_TRAPISR_ADDR,                        // FFE0 = TIMER1_A1
    APP_ISR_TRAPISR_ADDR,                        // FFE2 = TIMER1_A0
    APP_ISR_TRAPISR_ADDR,                        // FFE4 = DMA
    APP_ISR_TRAPISR_ADDR,                        // FFE6 = USB_UBM
    APP_ISR_TRAPISR_ADDR,                        // FFE8 = TIMER0_A1
    APP_ISR_TRAPISR_ADDR,                        // FFEA = TIMER0_A0
    APP_ISR_TRAPISR_ADDR,                        // FFEC = ADC12
    APP_ISR_TRAPISR_ADDR,                        // FFEE = USCI_B0
    APP_ISR_TRAPISR_ADDR,                        // FFF0 = USCI_A0
    APP_ISR_TRAPISR_ADDR,                        // FFF2 = WDT
    APP_ISR_TRAPISR_ADDR,                        // FFF4 = TIMER0_B1
    APP_ISR_TRAPISR_ADDR,                        // FFF6 = TIMER0_B0
    APP_ISR_TRAPISR_ADDR,                        // FFF8 = COMP_B
    APP_ISR_TRAPISR_ADDR,                        // FFFA = UNMI
    APP_ISR_TRAPISR_ADDR,                        // FFFC = SYSNMI
};



