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

/*
 * Initialize the PHYDL module
 */
void TI_MSPBoot_CI_PHYDL_Init(void)
{
    /* enable the UART pins */
    P1SEL |= (BIT1 | BIT2);
    P1SEL2 |= (BIT1 | BIT2);

    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 |= UCSSEL1;
    UCA0CTL0 = 0;

//    /* set the baud rate for 9600 baud based on a 8MHz clock */
//    UCA0BR1 = 833 >> 8;
//    UCA0BR0 = 833 & 0xff;
//    UCA0MCTL = 2 << 1;

    /* set the baud rate for 9600 baud based on a 8MHz clock */
	UCA0BR0     = 69;
	UCA0BR1     = 0;
	UCA0MCTL    = UCBRS2;

    UCA0CTL1 &= ~UCSWRST;
    TIMER_TIMEOUT_INIT();
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
    if (IFG2 & UCA0RXIFG)
    {
        *byte = UCA0RXBUF;
        return true;
    }

    return false;
}

/*
 * Write a byte, blocks until the byte cam be written
 */
bool TI_MSPBoot_CI_PHYDL_write(uint8_t *byte,   /* data pointer */
                               uint8_t length   /* number of bytes to write */
                               )                /* returns true on success */
{
    TIMER_TIMEOUT_START();
    while (!TIMER_TIMEOUT_EXPIRED())
    {
        if (IFG2 & UCA0TXIFG)
        {
            UCA0TXBUF = *byte;
            length--;
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
