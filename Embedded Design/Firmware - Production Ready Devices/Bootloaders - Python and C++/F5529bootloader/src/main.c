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
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR _App_End
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
#define TIMEOUT_COUNT               (30*MS_CYCLES/TIMEOUT_MS) /* 5 seconds */

// status
// D206 R4 G4 B4
#define LED0_RED_BIT             0x01
#define LED0_GREEN_BIT           0x04
#define LED0_BLUE_BIT            0x02
#define LED0_RED()               P2OUT &= ~LED0_RED_BIT;
#define LED0_GREEN()             P2OUT &= ~LED0_GREEN_BIT
#define LED0_BLUE()              P2OUT &= ~LED0_BLUE_BIT;
#define LED0_BLUE_TOGGLE()       P2OUT ^= LED0_BLUE_BIT;
#define LED0_BLACK()			 P2OUT |= (LED0_BLUE_BIT | LED0_RED_BIT | LED0_GREEN_BIT);
#define LED0_WHITE()			 P2OUT &= ~(LED0_BLUE_BIT | LED0_RED_BIT | LED0_GREEN_BIT);
#define LED0_YELLOW()            P2OUT &= ~(LED0_RED_BIT | LED0_GREEN_BIT); P2OUT |= LED0_BLUE_BIT;

//
//  Local function prototypes
//
static void FastClock(void);
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
    FastClock();

    // Validate the application and jump if needed
    //if (TI_MSPBoot_AppMgr_ValidateApp() == TRUE_t){
    //	TI_MSPBoot_APPMGR_JUMPTOAPP();
    //}

    /* bootloader activity starts with red, blinks blue */
    P2DIR |= (LED0_BLUE_BIT | LED0_RED_BIT | LED0_GREEN_BIT);
    LED0_BLACK();

    LED0_RED();
    timeoutCount = 20000;
    while (timeoutCount--) {
    	__delay_cycles(200);
    }
    LED0_BLACK();

    TI_MSPBoot_CI_Init();      // Initialize the Communication Interface
//    P4OUT &= ~BIT0;

    TIMER_TIMEOUT_INIT();
    TIMER_TIMEOUT_START();
    timeoutCount = 0;
    unsigned int loopCount;

    while(1) {
        // Poll PHY and Data Link interface for new packets
        TI_MSPBoot_CI_PHYDL_Poll();

        // If a new packet is detected, process it
        bootCode = TI_MSPBoot_CI_Process();
        if (bootCode == RET_OK) {
        	LED0_BLUE_TOGGLE();
        }

        if (loopCount++ == 0) { // going to rollover at 65k
        	LED0_BLUE_TOGGLE();
            // TI_MSPBoot_CI_PHYDL_writeByte(0x00);
        }

        /* update the timeout timer */
        if (TIMER_TIMEOUT_EXPIRED()) {
            timeoutCount++;
            TIMER_TIMEOUT_RESET();
        }

        if ((bootCode == RET_JUMP_TO_APP) || (timeoutCount >= TIMEOUT_COUNT)) {
//        if (bootCode == RET_JUMP_TO_APP) {
            // If Packet indicates a jump to App
            // Validate the application and jump if needed
            if (TI_MSPBoot_AppMgr_ValidateApp() == TRUE_t){
            	TI_MSPBoot_APPMGR_JUMPTOAPP();
                // should never get here but but if we do reset the cpu
                TI_MSPBoot_AppMgr_JumpToApp();
            } else {
            	LED0_WHITE();
                timeoutCount = 1000;
                while (timeoutCount--) {
                	__delay_cycles(200);
                }
                LED0_BLACK();
                // for debugging, TI_MSPBoot_CI_PHYDL_writeByte(0x00);
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
static void FastClock(void)
{
	UCSCTL3 = SELREF_2;									// Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;									// Set ACLK = REFO
	UCSCTL0 = 0x0000;									// Set lowest possible DCOx, MODx

	// Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
	do
		{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);		// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;								// Clear fault flags
		}while (SFRIFG1&OFIFG);							// Test oscillator fault flag

	__bis_SR_register(SCG0);							// Disable the FLL control loop
	UCSCTL1 = DCORSEL_5;								// Select DCO range 16MHz operation
	UCSCTL2 = 0x10F4;									// Set DCO Multiplier for 8MHz, (N + 1) * FLLRef = Fdco, (249 + 1) * 32768 = 8MHz
	__bic_SR_register(SCG0);							// Enable the FLL control loop

	// Worst-case settling time for the DCO when the DCO range bits have been
	// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	// UG for optimization.
	// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
	__delay_cycles(250000);
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
/*
	P1SEL = 0x00;
	P1SEL2 = 0x00;
	P1REN = 0x00;
	P2DIR = 0x00;
	UCA0CTL1 = 0x01;
	UCA0BR0 = 0x00;
	UCA0BR1 = 0x00;
	UCA0MCTL = 0x00;
	TA1CTL = 0x00;
	TA1CCTL0 = 0x00;
	TA1CCTL1 = 0x00;
	TA1CCTL2 = 0x00;
	TA1R = 0x00;
	TA1CCR0 = 0x00;
	BCSCTL1 = 0x87;
	BCSCTL3 = 0x05;
	DCOCTL = 0x60;
*/
}
