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
//  Type Definitions
//
/*! State machine for USI. This variable is in a preserved memory location
    and shouldn't be modified by application */
typedef enum {
    USI_STATE_IDLE=0,           /*! Initialized state waiting for start */
    USI_STATE_START_RECEIVED,   /*! Start bit received */
    USI_STATE_TX_ACK_ADDR,      /*! Send ACK/NACK after address */
    USI_STATE_RXBYTE,           /*! Byte reception */
    USI_STATE_TX_ACK_DATA,      /*! Send ACK after byte RX */
    USI_STATE_TXBYTE,           /*! Byte Transmission */
    USI_STATE_RX_ACK_DATA,      /*! Wait for ACK/NACK after byte TX */
    USI_STATE_TEST_ACK,         /*! Check ACK/NACK after byte TX */
    USI_STATE_PREP_RESTART      /*! Transfer end, wait for restart */
}USI_State_Machine_e;

#ifdef __IAR_SYSTEMS_ICC__
#pragma location="RAM_CISM"
__no_init USI_State_Machine_e CI_State_Machine;
#elif defined (__TI_COMPILER_VERSION__)
extern USI_State_Machine_e CI_State_Machine;
#endif

/*! Comm Interface callback structure */
#ifdef __IAR_SYSTEMS_ICC__
#pragma location="RAM_CICALLBACK"
__no_init t_CI_Callback  * CI_Callback_ptr;
#elif defined (__TI_COMPILER_VERSION__)
extern t_CI_Callback  * CI_Callback_ptr;
#endif

//
//  External variables
//
extern uint8_t   StatCtrl;

//
//  Local Function prototypes
//
#ifdef CONFIG_CI_PHYDL_END_CALLBACK
static void Check_StopFlag(void);
#endif

#ifdef CONFIG_CI_PHYDL_TIMEOUT
#define MS_CYCLES    ((MCLK/4)/1000)
#define SMBUS_TIMEOUT_CYCLES        (25*MS_CYCLES)
#define TIMER_TIMEOUT_INIT()        { TACTL = 0;\
                                      TACCR0 = SMBUS_TIMEOUT_CYCLES;\
                                      TACCTL0 = 0;}
#define TIMER_TIMEOUT_START()       { TACTL = TASSEL_2 + TACLR + MC_1 + ID_2; \
                                      TACCTL0 &= ~CCIFG; }
#define TIMER_TIMEOUT_STOP()        { TACTL = TASSEL_2 + TACLR + MC_0;\
                                      TACCTL0 &= ~CCIFG; }
#define TIMER_TIMEOUT_EXPIRED()     (TACCTL0 & CCIFG)
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
   // P1SEL |= (BIT6 | BIT7);                 // Enable P1[6:7] for USI I2C mode
    CHECK_TIMINGS_INIT();

    USICTL0 = (USIPE6+USIPE7+USISWRST);     // USI in reset, SCL/SDA enabled, I2C slave
    USICTL1 = (USII2C);                     // Enable I2C mode, interrupt disabled
                                            //  Start and counter
    USICKCTL = (USICKPL);                   // SCL inactive state is High
    USICNT = (USIIFGCC);                    // Disable auto clear ctrl

   // Clear all flags
    CI_State_Machine = USI_STATE_IDLE;

    USICTL0 &= ~(USISWRST);                 // Release USI for operation
    USICTL1 &= ~(USIIFG);                   // Clear USI Flag

#ifdef CONFIG_CI_PHYDL_TIMEOUT
    TIMER_TIMEOUT_INIT();                   // Program Timer A1	CCR register
#endif

    // Initialize all callbacks
    CI_Callback_ptr = CI_Callback;
}

/******************************************************************************
 *
 * @brief   Disables the USI module
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_disable(void)
{
    USICTL0 |= USISWRST;
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
    USICTL0 &= ~USISWRST;
}

/******************************************************************************
 *
 * @brief   Checks the Stop flag and calls the callback when set
 *
 * @return  none
 *****************************************************************************/
#ifdef CONFIG_CI_PHYDL_END_CALLBACK
static void Check_StopFlag(void)
{
    if (USICTL1 & USISTP)
    {
        CHECK_TIMINGS_START(BIT2);
        // Reset I2C state machine
        USICTL0 &= ~USIOE;              //Prep for Start condition
        CI_State_Machine = USI_STATE_IDLE; // Reset state machine
        USICTL1 &= ~USISTP;
#ifdef CONFIG_CI_PHYDL_TIMEOUT
        TIMER_TIMEOUT_STOP();
#endif
        if (CI_Callback_ptr->EndCallback != NULL)
            CI_Callback_ptr->EndCallback();            // Call Start callback (if enabled and valid)
        CHECK_TIMINGS_END(BIT2);
    }
}
#endif

/******************************************************************************
*
 * @brief   Polls for USI flags, and maintains the USI state machine
 *  - If USISTTIFG is detected, the state machine is initialized
 *  - If USIIFG is detected, the state machine is executed
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_CI_PHYDL_Poll(void)
{
    uint8_t temp;

    if (USICTL1 & USISTTIFG)                // Check for Start bit flag
    {
        CHECK_TIMINGS_START(BIT0);

#ifdef CONFIG_CI_PHYDL_END_CALLBACK
        if (CI_State_Machine != USI_STATE_IDLE)
        {
            Check_StopFlag();
        }
#endif           
        //USI_State_Machine = USI_STATE_START_RECEIVED;
        USICNT &= 0xE0;                     // Clean USI bit count
        USICNT += 0x08;                     // Bit counter = 8
        USICTL1 &= ~USISTTIFG;              // Clear start flag
#ifdef CONFIG_CI_PHYDL_START_CALLBACK
        if (CI_Callback_ptr->StartCallback != NULL)
            CI_Callback_ptr->StartCallback();            // Call Start callback (if enabled and valid)
#endif
        CI_State_Machine = USI_STATE_TX_ACK_ADDR;
        USICTL1 &= ~(USIIFG|USISTP);    // Clear Start and Stop bits
#ifdef CONFIG_CI_PHYDL_TIMEOUT
        TIMER_TIMEOUT_START();
#endif
        CHECK_TIMINGS_END(BIT0);
    }

    if (USICTL1 & USIIFG)              // Check for USI flag
    {
        CHECK_TIMINGS_START(BIT1);
#ifdef CONFIG_CI_PHYDL_TIMEOUT
        TIMER_TIMEOUT_START();
#endif
        switch (CI_State_Machine)          // Execute state machine
        {
            case  USI_STATE_TX_ACK_ADDR:    // Address +R/W was received, send (N)ACK
                // Check the Address
                if ((USISRL & 0xFE) == (I2C_SLAVE_ADDR << 1))
                {
                    // Message is for us, Check r/w
                    if (USISRL & BIT0)
                        CI_State_Machine = USI_STATE_TXBYTE;   // R/W = 1,  TX
                    else
                        CI_State_Machine = USI_STATE_RXBYTE;   // R/W = 0, RX
                    USICTL0 |= USIOE;       // SDA = output
                    USISRL = 0x00;          // Send ACK
                }
                else
                {
                    // Message is not for us
                    USISRL = 0xFF;          // Send NACK
                    CI_State_Machine = USI_STATE_PREP_RESTART;
                }
                USICNT |= 0x01;             // Bit counter = 1
                USICTL1 &= ~ USIIFG;
            break;
            case  USI_STATE_RXBYTE:
                // Receive one byte
                USICTL0 &= ~USIOE;          // SDA = input
                USICNT |= 0x08;             // Bit counter = 8
                CI_State_Machine = USI_STATE_TX_ACK_DATA;
                USICTL1 &= ~ USIIFG;
            break;
            case  USI_STATE_TX_ACK_DATA:
                // Send ACK after byte reception
                if (CI_Callback_ptr->RxCallback != NULL)
                    CI_Callback_ptr->RxCallback(USISRL);  // On byte RX call RXCallback if valid
                USICTL0 |= USIOE;               // SDA output
                USISRL = 0;                     // Send ack
                USICNT |= 0x01;                 // Bit counter = 1
                CI_State_Machine = USI_STATE_RXBYTE;
                USICTL1 &= ~ USIIFG;
            break;
            case  USI_STATE_TXBYTE:
                // Transmit one byte
                USICTL0 |= USIOE;               // SDA = output
                if (CI_Callback_ptr->TxCallback != NULL)
                    CI_Callback_ptr->TxCallback(&temp);    // On byte TX call TXCallback if valid
                USISRL = temp;                  // Send Data
                USICNT |= 0x08;                 // Bit count = 8
                CI_State_Machine = USI_STATE_RX_ACK_DATA;
                USICTL1 &= ~ USIIFG;
            break;
            case  USI_STATE_RX_ACK_DATA:
                // Get (N)ACK from Master after TX byte
                USICTL0 &= ~USIOE;              // SDA = input
                USICNT |= 0x01;                 // Bit count = 1
                CI_State_Machine = USI_STATE_TEST_ACK;
                USICTL1 &= ~ USIIFG;
            break;
            case  USI_STATE_TEST_ACK:
                // Test the (N)ACK from Master
                if ((USISRL & 0x01) == 0)
                    CI_State_Machine = USI_STATE_TXBYTE;  //Ack rxed ready for next transfer
                else
                {
                    USICTL0 &=  ~USIOE;         // Nack rxed, last transmit
                    CI_State_Machine = USI_STATE_IDLE;   //Re-initialize state machine
                    USICTL1 &= ~ USIIFG;
                }
            break;
            case  USI_STATE_PREP_RESTART:
                // Reset I2C state machine on address NACK
                USICTL0 &= ~USIOE;              //Prep for Start condition
                CI_State_Machine = USI_STATE_IDLE; // Reset state machine
                USICTL1 &= ~ USIIFG;
            break;
            default:
#ifdef CONFIG_CI_PHYDL_TIMEOUT
                TIMER_TIMEOUT_STOP();
#endif
                USICTL1 &= ~ USIIFG;           // Idle, should not get here
            break;

        }
        CHECK_TIMINGS_END(BIT1);
    }
    
#ifdef CONFIG_CI_PHYDL_END_CALLBACK
    Check_StopFlag();   // Check the Stop flag
#endif

#ifdef CONFIG_CI_PHYDL_TIMEOUT
    if (TIMER_TIMEOUT_EXPIRED() != 0x00)
    {
CHECK_TIMINGS_START(BIT1|BIT0|BIT2);
        TIMER_TIMEOUT_STOP();
#ifdef CONFIG_CI_PHYDL_ERROR_CALLBACK
        if (CI_Callback_ptr->ErrorCallback != NULL)
            CI_Callback_ptr->ErrorCallback(1);          // Call Start callback (if enabled and valid)
#endif
CHECK_TIMINGS_END(BIT1|BIT0|BIT2);
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
#	ifdef CONFIG_CI_PHYDL_COMM_SHARED
# 		ifdef __IAR_SYSTEMS_ICC__
#			pragma location="BOOT_APP_VECTORS"
__root const uint16_t Boot2App_Vector_Table[] =
#		elif defined (__TI_COMPILER_VERSION__)
#			pragma DATA_SECTION(Boot2App_Vector_Table, ".BOOT_APP_VECTORS")
#           pragma RETAIN(Boot2App_Vector_Table)
const uint16_t Boot2App_Vector_Table[] =
#		endif
{
    (uint16_t) &TI_MSPBoot_CI_PHYDL_Init,   /*! Initialization routine */
    (uint16_t) &TI_MSPBoot_CI_PHYDL_Poll    /*! Poll routine */
};
#endif
