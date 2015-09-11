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
//  Include files
//
#include "msp430.h"
#include "TI_MSPBoot_Common.h"
#include "TI_MSPBoot_CI.h"
#include "TI_MSPBoot_AppMgr.h"

/* serial timeout */
#define uS_CYCLES                   (((MCLK/8)/1000)/1000)
#define MS_CYCLES                   ((MCLK/8)/1000)
#define TIMEOUT_MS                  (50)                /* 25 ms */
#define TIMEOUT_CYCLES              (TIMEOUT_MS*MS_CYCLES)
#define TIMER_TIMEOUT_INIT()        { TA0CTL = 0;\
                                      TA0CCR0 = TIMEOUT_CYCLES;\
                                      TA0CCTL0 = 0;}
#define TIMER_TIMEOUT_START()       { TA0CTL = TASSEL_2 + TACLR + MC_1 + ID_3; \
                                      TA0CCTL0 &= ~CCIFG; }
#define TIMER_TIMEOUT_STOP()        { TA0CTL = TASSEL_2 + TACLR + MC_0;\
                                      TA0CCTL0 &= ~CCIFG; }
#define TIMER_TIMEOUT_EXPIRED()     (TA0CCTL0 & CCIFG)
#define TIMER_TIMEOUT_RESET()       (TA0CCTL0 &= ~CCIFG)
#define TIMEOUT_COUNT               (5*MS_CYCLES/TIMEOUT_MS) /* 5 seconds */

#define LED_GREEN   BIT0
#define LED_BLUE    BIT1
#define LED_RED     BIT2

//
//  Local function prototypes
//
static void clock_init(void);
static void HW_init(void);
static void registerReset(void);

/******************************************************************************
 *
 * @brief   Main function
 *  - Initializes the MCU
 *  - Selects whether to run application or bootloader
 *  - If bootloader:
 *      - Initializes the peripheral interface
 *      - Waits for a command
 *      - Sends the corresponding response
 *  - If application:
 *      - Jump to application
 *
 *  @note   USI interrupts are enabled after this function
 * @return  none
 *****************************************************************************/
int main_boot( void )
{
    uint8_t bootCode;
    uint16_t timeoutCount;

    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW + WDTHOLD;

    // Initialize MCU
    HW_init();
    clock_init();

    // comm direction pin for use on sensor pack and pump controller
	P2SEL = 0;
	P2SEL2 = 0;
	P2DIR  |=  COMM_DIR;
	P2OUT  &= ~COMM_DIR;
	P2DIR |= SUNSENSOR_TX_EN;
	P2OUT |= SUNSENSOR_TX_EN;

    /* bootloader activity, blue led, red led */
    P2DIR |= (LED_BLUE | LED_RED | LED_GREEN);
    P2OUT |= (LED_BLUE | LED_RED | LED_GREEN);
    P2OUT &= ~LED_RED; // show bootloader entered

    timeoutCount = 10000;
    while (timeoutCount--) {
    	__delay_cycles(200);
    }
    P2OUT |= LED_RED; // red off

    TI_MSPBoot_CI_Init();      // Initialize the Communication Interface

    TIMER_TIMEOUT_INIT();
    TIMER_TIMEOUT_START();
    timeoutCount = 0;
    unsigned int loopCount;
    while(1)
    {

        // Poll PHY and Data Link interface for new packets
        TI_MSPBoot_CI_PHYDL_Poll();

        // If a new packet is detected, process it
        bootCode = TI_MSPBoot_CI_Process();
        if (bootCode == RET_OK) {
        	P2OUT ^= LED_BLUE; // blue toggle
        }

        if (loopCount++ == 0) { // going to rollover at 65k
        	P2OUT ^= LED_BLUE; // blue toggle
        }

        /* update the timeout timer */
        if (TIMER_TIMEOUT_EXPIRED()) {
            timeoutCount++;
            TIMER_TIMEOUT_RESET();
        }

        if ((bootCode == RET_JUMP_TO_APP) || (timeoutCount >= TIMEOUT_COUNT)) {
            // If Packet indicates a jump to App
            // Validate the application and jump if needed
            if (TI_MSPBoot_AppMgr_ValidateApp() == TRUE_t){
            	registerReset();
            	TI_MSPBoot_APPMGR_JUMPTOAPP();
                // should never get here but but if we do reset the cpu
                TI_MSPBoot_AppMgr_JumpToApp();
            } else {
                P2OUT &= ~(LED_RED | LED_GREEN); // show yellow for jump attempt failed
                P2OUT |= LED_BLUE; // blue off
                timeoutCount = 10000;
                while (timeoutCount--) {
                	__delay_cycles(200);
                }
                P2OUT |= (LED_RED | LED_GREEN); // yellow off
            }
            TIMER_TIMEOUT_INIT();
            TIMER_TIMEOUT_START();
            timeoutCount = 0;
        } else if (bootCode == RET_OK) {
            timeoutCount = 0;
        }

/*
#ifdef NDEBUG
        // Feed the dog every ~1000ms
        WATCHDOG_FEED();
#endif
*/
    }


}

/******************************************************************************
 *
 * @brief   Initializes the MSP430 Clock
 *
 * @return  none
 *****************************************************************************/
//inline static void clock_init(void)
static void clock_init(void)
{
#if (MCLK==1000000)
    // Validate the calibration values (only in Debug)
    ASSERT_H((CALBC1_1MHZ !=0xFF) && (CALDCO_1MHZ != 0xFF));

    BCSCTL1 = CALBC1_1MHZ;              // Set DCO to default of 1Mhz
    DCOCTL = CALDCO_1MHZ;
#elif (MCLK==4000000)
    // Validate the calibration values (only in Debug)
    ASSERT_H((CALBC1_8MHZ !=0xFF) && (CALDCO_8MHZ != 0xFF));

    BCSCTL1 = CALBC1_8MHZ;              // Set DCO to default of 8Mhz
    DCOCTL = CALDCO_8MHZ;
    BCSCTL2 = DIVM_1 | DIVS_1;          // MCLK/SMCLK = 8Mhz/2 = 4Mhz
    #elif (MCLK==8000000)
    // Validate the calibration values (only in Debug)
    ASSERT_H((CALBC1_8MHZ !=0xFF) && (CALDCO_8MHZ != 0xFF));

    BCSCTL1 = CALBC1_8MHZ;              // Set DCO to default of 8Mhz
    DCOCTL = CALDCO_8MHZ;
#else
#error "Please define a valid MCLK or add configuration"
#endif
    BCSCTL3 = LFXT1S_2;                // LFXT1 = VLO
    IFG1 &= ~(OFIFG);                   // Clear OSCFault flag

}


/******************************************************************************
 *
 * @brief   Initializes the basic MCU HW
 *
 * @return  none
 *****************************************************************************/
static void HW_init(void)
{
    // The following routines disable interrupts which are cleared by POR but
    // not by a PUC. In G2xx, these are: Timers, CompA, ADC10
#ifdef HW_RESET_PUC
    TA0CTL = 0x00;
    #ifdef __MSP430_HAS_T1A3__
    TA1CTL = 0x00;
    #endif
    #ifdef __MSP430_HAS_CAPLUS__
    CACTL1 = 0x00;
    #endif
    #ifdef __MSP430_HAS_ADC10__
    ADC10CTL0 = 0x00;
    #endif
#endif
}

/******************************************************************************
 *
 * @brief   Sets registers affected by this bootloader back to reset state
 *
 * @return  none
 *****************************************************************************/
static void registerReset(void)
{
	P1SEL       = 0x00;
	P1SEL2      = 0x00;
	P1REN       = 0x00;
	P2DIR       = 0x00;
	UCA0CTL1    = 0x01;
	UCA0BR0     = 0x00;
	UCA0BR1     = 0x00;
	UCA0MCTL    = 0x00;
	TACTL       = 0x00;
	TACCTL0     = 0x00;
	TACCTL1     = 0x00;
	TACCTL2     = 0x00;
	TAR         = 0x00;
	TACCR0      = 0x00;
	BCSCTL1     = 0x87;
	BCSCTL3     = 0x05;
	DCOCTL      = 0x60;
}
