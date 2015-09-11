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
#include "TI_MSPBoot_CI.h"
#include "TI_MSPBoot_AppMgr.h"

#ifdef CONFIG_CI_PHYDL_TIMEOUT
#define MS_CYCLES    ((MCLK/4)/1000)
#define SMBUS_TIMEOUT_CYCLES        (25*MS_CYCLES)
#define TIMER_TIMEOUT_INIT()        { TA1CTL = 0;\
                                      TA1CCR0 = SMBUS_TIMEOUT_CYCLES;\
                                      TA1CCTL0 = 0;}
#define TIMER_TIMEOUT_START()       { TA1CTL = TASSEL_2 + TACLR + MC_1 + ID_2; \
                                      TA1CCTL0 &= ~CCIFG; }
#define TIMER_TIMEOUT_STOP()        { TA1CTL = TASSEL_2 + TACLR + MC_0;\
                                      TA1CCTL0 &= ~CCIFG; }
#define TIMER_TIMEOUT_EXPIRED()     (TA1CCTL0 & CCIFG)
#else
#define TIMER_TIMEOUT_INIT()
#define TIMER_TIMEOUT_START()
#define TIMER_TIMEOUT_STOP()
#define TIMER_TIMEOUT_EXPIRED()     false
#endif

//
//  Local function prototypes
//
void configurePin(unsigned char pin);

/*
 * Initialize the PHYDL module
 */
void TI_MSPBoot_CI_PHYDL_Init(void)
{
	unsigned long baud;
	unsigned short br;

	baud = 57600L;

	// Unlock the port mapping registers.
	PMAPPWD = 0x02D52;
    // Map port 4 pins to the desired functions
    P4MAP1 = PM_UCA1RXD;
    P4MAP2 = PM_UCA1TXD;
    // Lock the port mapping registers.
    PMAPPWD = 0;
    // setup outputs
    P4DIR |= BIT2;
    P4OUT = 0x00;

    // Configure the USCI pins.
	configurePin(0x42); // txMosiPin
	configurePin(0x41); // rxMisoPin
	configurePin(0x00); // clkPin
	configurePin(0x00); // ssPin

	// Keep the Usci in reset while we set it up.
	UCA1CTLW0 |= UCSWRST;

	// Set up the port.
	UCA1MCTL = UCBRF_1 + UCOS16;
	UCA1IRTCTL = UCIRTXPL2 + UCIRTXPL0 + UCIRTXCLK + UCIREN;
	baud = baud * 16;

	// Use the SMCLK.
	UCA1CTL1 |= UCSSEL_2;

	// Initialize the USCI state machine.
	UCA1CTLW0 &= ~UCSWRST;

	// Find the baud rate divider.
	switch (baud)
	{
		case 9600:
			br		= 833;
			break;
		case 28800:
			br		= 280;
			break;
		case 57600:
			br		= 140;
			break;
		case 115200:
			br		= 70;
			break;
		case 230400:
			br		= 35;
			break;
		case 460800:
			br		= 17;
			break;
		case 921600:
			br		= 8;
			break;
		case 1843200:
			br		= 4;
			break;
		default:
			br 		= 833;
			break;
	}

	// Set the divider on the correct port.
	UCA1BRW = br;

    TIMER_TIMEOUT_INIT();
}

/// *****************************************************************************/
/// \brief Configure a pin to work as a serial port.
///
/// \param[in] pin - pin to configure for the serial port.
///
/// \returns nothing
/// *****************************************************************************/
void configurePin(unsigned char pin)
{
	unsigned char port, pinMask;

	// Configure the UCSI pins.
	port = (pin & 0xF0) >> 4;
	pinMask = 0x01 << (pin & 0x0F);

	switch (port)
	{
		case 0:
			break;
		case 1:
			P1SEL |= pinMask;
			break;
		case 2:
			P2SEL |= pinMask;
			break;
		case 3:
			P3SEL |= pinMask;
			break;
		case 4:
			P4SEL |= pinMask;
			break;
		case 5:
			P5SEL |= pinMask;
			break;
		case 6:
			P6SEL |= pinMask;
			break;
		default:
			break;
	}
}

/******************************************************************************
 *
 * @brief   Disables the USI module
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_disable(void)
{
    UCB0CTL1 |= UCSWRST;
    TIMER_TIMEOUT_STOP();
}

/******************************************************************************
 *
 * @brief   Enables the USI module
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_reenable(void)
{
    UCB0CTL1 &= ~UCSWRST;
}

/*
 * Read a byte if one is available
 */
bool TI_MSPBoot_CI_PHYDL_read(uint8_t *byte     /* set to byte value */
                              )                 /* true when byte is valid */
{
    if (UCA1IFG & UCRXIFG)
    {
        *byte = UCA1RXBUF;
        P2OUT ^= 0x02; // toggle blue
        return true;
    }

    return false;
}

/*
 * Write a byte, blocks until the byte can be written
 */
bool TI_MSPBoot_CI_PHYDL_write(uint8_t *byte,   /* data pointer */
                               uint8_t length   /* number of bytes to write */
                               )                /* returns true on success */
{
    uint8_t echoByte;

	TIMER_TIMEOUT_START();
    while (!TIMER_TIMEOUT_EXPIRED())
    {
        if (UCA1IFG & UCTXIFG)
        {
            UCA1TXBUF = *byte;
            length--;

            while (!(UCA1IFG & UCRXIFG)) {}
            // throw away the echo byte(s)
            echoByte = UCA1RXBUF;
            if (length == 0)
            {
                return true;
            }
        }
        TIMER_TIMEOUT_START();
    }

 //   return false;
}

/*
 * Write a single byte to the uart
 */
bool TI_MSPBoot_CI_PHYDL_writeByte(uint8_t byte     /* byte to send */
                                   )                /* returns true on success */
{
    return TI_MSPBoot_CI_PHYDL_write(&byte, 1);
}
