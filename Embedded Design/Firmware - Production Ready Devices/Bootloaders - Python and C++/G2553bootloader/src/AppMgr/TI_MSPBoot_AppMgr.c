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
#include "Crc.h"

//
//  Global variables
//
/*! Password sent by Application to force boot mode. This variable is in a fixed
    location and should keep same functionality and location in Boot and App */
#ifdef __IAR_SYSTEMS_ICC__
#           pragma location="RAM_PASSWORD"
    __no_init uint16_t  PassWd;
#       elif defined (__TI_COMPILER_VERSION__)
extern uint16_t  PassWd;
#endif

/*! Status and Control byte. This variable is in a fixed
 location and should keep same functionality and location in Boot and App */
#ifdef __IAR_SYSTEMS_ICC__
#           pragma location="RAM_STATCTRL"
    __no_init uint8_t  StatCtrl;
#       elif defined (__TI_COMPILER_VERSION__)
extern uint8_t  StatCtrl;
#endif

//
//  Local function prototypes
//
static tBOOL TI_MSPBoot_AppMgr_BootisForced(void);

/******************************************************************************
 *
 * @brief   Checks if an Application is valid
 *  Depending on CONFIG_APPMGR_APP_VALIDATE, this function can validate app by:
 *  CONFIG_APPMGR_APP_VALIDATE  |    Function
 *          1                   | Check if reset vector is different from 0xFFFF
 *          2                   | Check CRC_CCITT of application and compare vs
 *                              |  an expected value
 *          3                   | Check CRC8 of application and compare vs
 *                              |  an expected value
 *          other               | Application is expected to be valid
 *
 *
 * @return  TRUE_t if application is valid,
 *          FALSE_t if applicaiton is invalid
 *****************************************************************************/
#if (CONFIG_APPMGR_APP_VALIDATE == 1)
static tBOOL TI_MSPBoot_AppMgr_AppisValid(void)
{
    // Check if Application Reset vector exists
    if (*(volatile uint16_t *)(&_App_Reset_Vector) != 0xFFFF)
        return TRUE_t;
    else
        return FALSE_t;
}
#elif (CONFIG_APPMGR_APP_VALIDATE == 2)
// Check the Applications's Checksum
static tBOOL TI_MSPBoot_AppMgr_AppisValid(void)
{
    extern uint16_t _AppChecksum;   // Address of Application checksum
    extern uint16_t _App_Start;     // Address of Application start
    extern uint16_t _CRC_Size;      // Address of App area being calculated

    // calculate CRC and compare vs expected value in reserved location
    if (crc16MakeBitwise((uint8_t *) & _App_Start, (uint16_t) &_CRC_Size) != _AppChecksum){
    	return FALSE_t;
    }
    else{
        return TRUE_t;
    }
}
#elif (CONFIG_APPMGR_APP_VALIDATE == 3)
// Check the Applications's Checksum
static tBOOL TI_MSPBoot_AppMgr_AppisValid(void)
{
    extern uint8_t _AppChecksum_8;   // Address of Application checksum
    extern uint16_t _App_Start;     // Address of Application start
    extern uint16_t _CRC_Size;      // Address of App area being calculated

    // calculate CRC and compare vs expected value in reserved location
    if (crc8MakeBitwise((uint8_t *) & _App_Start, (uint16_t) &_CRC_Size) != _AppChecksum_8)
        return FALSE_t;
    else
        return TRUE_t;
}
#else
// Always assume that Application is valid
#error "Application is not validated"
#define TI_MSPBoot_AppMgr_AppisValid()   TRUE_t
#endif


/******************************************************************************
 *
 * @brief   Decides whether to stay in MSPBoot or if it should jump to App
 *  MSPBoot:  Boot mode is forced by a call from App, OR
 *          Boot mode is forced by an external event (button pressed), OR
 *          Application is invalid
 *  App:    Boot mode is not forced, AND
 *          Application is valid
 *
 * @return  TRUE_t if application is valid and should be executed
 *          FALSE_t if we must stay in Boot mode
 *****************************************************************************/
tBOOL TI_MSPBoot_AppMgr_ValidateApp(void)
{
    if ((TI_MSPBoot_AppMgr_BootisForced() == FALSE_t) && (TI_MSPBoot_AppMgr_AppisValid() == TRUE_t)){
        return TRUE_t;  // Boot is not forced and App is valid
    }
    else{
        return FALSE_t; // Boot is forced or App is valid
    }
}


/******************************************************************************
 *
 * @brief   Jump to Appplication 
 *          A Reset is forced in order to validate Application after Reset
 *
 * @return  None
 *****************************************************************************/
void TI_MSPBoot_AppMgr_JumpToApp(void)
{
#if defined (HW_RESET_BOR)
    // Force a Software BOR
    PMMCTL0 = PMMPW | PMMSWBOR;
#else
    // Force a PUC reset by writing incorrect Watchdog password
    WDTCTL = 0xDEAD;
#endif
}


/******************************************************************************
 *
 * @brief   Checks if Boot mode is forced
 *          Boot mode is forced by an application call sending a request and password
 *          Or by an external event such as a button press
 *
 * @return  TRUE_t Boot mode is forced
 *          FALSE_t Boot mode is not forced
 *****************************************************************************/
static tBOOL TI_MSPBoot_AppMgr_BootisForced(void)
{
    tBOOL ret = FALSE_t;

    // Check if application is requesting Boot mode and password is correct
    if (((StatCtrl & BOOT_APP_REQ) != 0) && (PassWd == BSL_PASSWORD)){
        ret = TRUE_t;
    }

/*
 * Eliminating this code for now since the Sensor Pack etc will not use a button to force boot
    // Check for an external event such as S3 (P1.3) button in Launchpad
    __delay_cycles(10000);   // Wait for pull-up to go high
    //If S2 button (P4.1) is pressed, force BSL
    if (HW_ENTRY_CONDITION){
        ret = TRUE_t;
    }
*/
    // Clear Status and Control byte and password
    PassWd = 0;
    StatCtrl = 0;
    return ret;
}
