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

//
//  Type Definitions
//

//#define CHECK_TIMINGS

#ifdef  CHECK_TIMINGS
#define CHECK_TIMINGS_INIT()        {P2DIR |= (0x07);}      // Todo: Remove
#define CHECK_TIMINGS_START(x)      {P2OUT |= x;}           // Todo: Remove
#define CHECK_TIMINGS_END(x)        {P2OUT &= ~x;}           // Todo: Remove
#else
#define CHECK_TIMINGS_INIT()
#define CHECK_TIMINGS_START(x)
#define CHECK_TIMINGS_END(x)
#endif



//
//  External variables
//
extern uint8_t   StatCtrl;

//
//  Local Function prototypes
//
/*! Comm Interface callback structure */
#ifdef __IAR_SYSTEMS_ICC__
#pragma location="RAM_CICALLBACK"
__no_init t_CI_Callback  * CI_Callback_ptr;
#elif defined (__TI_COMPILER_VERSION__)
extern t_CI_Callback  * CI_Callback_ptr;
#endif

typedef enum {
    USCI_STATE_IDLE=0,          /*! Initialized state waiting for start */
    USCI_STATE_RECEIVING,       /*! Receiving packet */
}USCI_State_Machine_e;

#ifdef __IAR_SYSTEMS_ICC__
#pragma location="RAM_CISM"
__no_init USCI_State_Machine_e CI_State_Machine;
#elif defined (__TI_COMPILER_VERSION__)
extern USCI_State_Machine_e CI_State_Machine;
#endif

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
#endif

//
//  Function definitions
//
/******************************************************************************
*
 * @brief   Initializes the USI I2C Slave interface
 *  - Sets corresponding GPIOs for I2C functionality
 *  - Resets and then configures the I2C
 *  - Initializes the I2C state machine
 *  - Initializes the Start, TX, RX callbacks
 *
 *  @param  StartCallback   Pointer to start callback (called after Start reception)
 *  @param  RxCallback      Pointer to RX callback (called after received Byte)
 *  @param  TxCallback      Pointer to TX callback (called after transmitted Byte)
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_Init(t_CI_Callback * CI_Callback)
{
    P1SEL |= (BIT6 | BIT7);                 // Enable P1[6:7] for USCI_B0 I2C mode
    P1SEL2 |= (BIT6 | BIT7);                //

    CHECK_TIMINGS_INIT();

    UCB0CTL1 |= UCSWRST;                    // Enable SW reset
    UCB0CTL0 = UCMODE_3 + UCSYNC;           // I2C Slave, synchronous mode
    UCB0I2COA = I2C_SLAVE_ADDR;             // set own (slave) address
    UCB0CTL1 &= ~UCSWRST;                   // Clear SW reset, resume operation
#ifdef CONFIG_CI_PHYDL_TIMEOUT
    TIMER_TIMEOUT_INIT();                   // Program Timer A1	CCR register
#endif
    // Initialize all callbacks
    CI_Callback_ptr = CI_Callback;
    CI_State_Machine = USCI_STATE_IDLE;
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
#ifdef CONFIG_CI_PHYDL_TIMEOUT
    TIMER_TIMEOUT_STOP();
#endif
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

/******************************************************************************
*
 * @brief   Polls for USCI flags
 *  - Start and RX/TX are sepparated due to sepparate interrupt vectors
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_Poll(void)
{
    uint8_t temp;
    uint8_t flag_stat, flag_ifg2;
    
    // Read flags at the beggining of function in order to avoid race conditions
    flag_ifg2 = IFG2;
    flag_stat = UCB0STAT;

    if (flag_stat & UCSTTIFG)                // Check for Start bit flag
    {
        CHECK_TIMINGS_START(BIT0);
#ifdef CONFIG_CI_PHYDL_END_CALLBACK
        if ((CI_State_Machine != USCI_STATE_IDLE) && 
            (CI_Callback_ptr->EndCallback != NULL))
        {
            // Special case when a stop was cleared by a new Start
            UCB0STAT &= ~UCSTPIFG;              // Clear stop flag
            #ifdef CONFIG_CI_PHYDL_TIMEOUT
                TIMER_TIMEOUT_STOP();
            #endif
            CI_Callback_ptr->EndCallback(); // Call Stop callback (if enabled and valid)
            CI_State_Machine = USCI_STATE_IDLE;
            return;
        }
#endif            

#ifdef CONFIG_CI_PHYDL_START_CALLBACK
        if (CI_Callback_ptr->StartCallback != NULL)
        {
            CI_Callback_ptr->StartCallback(); // Call Start callback (if enabled and valid)
        }
#endif
        CI_State_Machine = USCI_STATE_RECEIVING; 
        UCB0STAT &= ~UCSTTIFG;              // Clear start flag
#ifdef CONFIG_CI_PHYDL_TIMEOUT
        TIMER_TIMEOUT_START();
#endif
        CHECK_TIMINGS_END(BIT0);
    }
    else if (flag_ifg2 & UCB0RXIFG)
    {
       CHECK_TIMINGS_START(BIT1);

        temp = UCB0RXBUF;
        // Send ACK after byte reception
        if (CI_Callback_ptr->RxCallback != NULL)
            CI_Callback_ptr->RxCallback(temp);  // On byte RX call RXCallback if valid
#ifdef CONFIG_CI_PHYDL_TIMEOUT
        TIMER_TIMEOUT_START();
#endif
        CHECK_TIMINGS_END(BIT1);
    }
    else if (flag_ifg2 & UCB0TXIFG)              // Check for USI flag
    {
        CHECK_TIMINGS_START(BIT0 + BIT1);
        // Send ACK after byte reception
        if (CI_Callback_ptr->TxCallback != NULL)
        {
            CI_Callback_ptr->TxCallback(&temp);  // On byte RX call RXCallback if valid
            UCB0TXBUF = temp;
        }
#ifdef CONFIG_CI_PHYDL_TIMEOUT
        TIMER_TIMEOUT_START();
#endif
        CHECK_TIMINGS_END(BIT0+BIT1);
    }
    else if (flag_stat & UCSTPIFG)                // Check for Stop bit flag
    {
        CHECK_TIMINGS_START(BIT2);
        UCB0STAT &= ~UCSTPIFG;              // Clear start flag
#ifdef CONFIG_CI_PHYDL_TIMEOUT
        TIMER_TIMEOUT_STOP();
#endif
#ifdef CONFIG_CI_PHYDL_END_CALLBACK
        if (CI_Callback_ptr->EndCallback != NULL)
            CI_Callback_ptr->EndCallback();          // Call Start callback (if enabled and valid)
#endif
        CI_State_Machine = USCI_STATE_IDLE;
        CHECK_TIMINGS_END(BIT2);
    }
#ifdef CONFIG_CI_PHYDL_TIMEOUT
    else if (TIMER_TIMEOUT_EXPIRED() != 0x00)
    {
        TIMER_TIMEOUT_STOP();
#ifdef CONFIG_CI_PHYDL_ERROR_CALLBACK
        if (CI_Callback_ptr->ErrorCallback != NULL)
            CI_Callback_ptr->ErrorCallback(1);          // Call Start callback (if enabled and valid)
#endif
    }
#endif
}


//
//  Constant table
//
/*! Peripheral Interface vectors for Application:
    These vectors are shared with application and can be used by the App to
    initialize and use the Peripheral interface
 */
#   ifdef CONFIG_CI_PHYDL_COMM_SHARED
#       ifdef __IAR_SYSTEMS_ICC__
#           pragma location="BOOT_APP_VECTORS"
__root const uint16_t Boot2App_Vector_Table[] =
#       elif defined (__TI_COMPILER_VERSION__)
#           pragma DATA_SECTION(Boot2App_Vector_Table, ".BOOT_APP_VECTORS")
#           pragma RETAIN(Boot2App_Vector_Table)
const uint16_t Boot2App_Vector_Table[] =
#       endif
{
    (uint16_t) &TI_MSPBoot_CI_PHYDL_Init,   /*! Initialization routine */
    (uint16_t) &TI_MSPBoot_CI_PHYDL_Poll    /*! Poll routine */
};
#endif

