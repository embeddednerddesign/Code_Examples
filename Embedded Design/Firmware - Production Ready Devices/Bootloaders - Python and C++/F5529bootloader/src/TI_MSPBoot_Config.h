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

#ifndef __TI_MSPBoot_CONFIG_H__
#define __TI_MSPBoot_CONFIG_H__

//
// Include files
//
#define MCLK                    (8000000L)  /*! MCLK Frequency in Hz */

/*! Watchdog feed sequence */
#define WATCHDOG_FEED()         {WDTCTL = (WDTPW+WDTCNTCL+WDTSSEL+WDTIS0);}
/*! Hardware entry condition in order to force bootloader mode */
#define HW_ENTRY_CONDITION      (!(P1IN & BIT3))
/*! HW_RESET_BOR: Force a software BOR in order to reset MCU 
    HW_RESET_PUC: Force a PUC in order to reset MCU 
 */
#define HW_RESET_PUC

//
// Configuration MACROS
//
/* MSPBoot SIMPLE:
 *  If MSPBoot_SIMPLE is defined for the project (in this file or in project preprocessor
 *  options), the following settings will apply:
 *
 *  NDEBUG = defined: Debugging is disabled, ASSERT_H function is ignored,
 *      GPIOs are not used for debugging
 *
 *  CONFIG_APPMGR_APP_VALIDATE =1: Application is validated by checking its reset
 *      vector. If the vector is != 0xFFFF, the application is valid
 *
 *  CONFIG_MI_MEMORY_RANGE_CHECK = undefined: Address range of erase and Write
 *      operations are not validated. The algorithm used by SimpleProtocol
 *      doesn't write to invalid areas anyways
 *
 *  CONFIG_CI_PHYDL_COMM_SHARED = undefined: Communication interface is not used by
 *      application. Application can still initialize and use I2C driver on its own
 *
 *  CONFIG_CI_PHYDL_START_CALLBACK = undefined: USI I2C Start callback is not needed
 *      and is not implemented to save flash space
 */
#if defined(MSPBoot_SIMPLE)
#define NDEBUG
#define CONFIG_APPMGR_APP_VALIDATE    (1)
#undef CONFIG_MI_MEMORY_RANGE_CHECK
#undef CONFIG_CI_PHYDL_TIMEOUT
#undef CONFIG_CI_PHYDL_COMM_SHARED
#undef CONFIG_CI_PHYDL_START_CALLBACK
#undef CONFIG_CI_PHYDL_END_CALLBACK
#undef CONFIG_CI_PHYDL_ERROR_CALLBACK

/* MSPBoot BSL-based:
 *  If MSPBoot_BSL is defined for the project (in this file or in project preprocessor
 *  options), the following settings will apply:
 *
 *  NDEBUG = defined: Debugging is disabled, ASSERT_H function is ignored,
 *      GPIOs are not used for debugging
 *
 *  CONFIG_APPMGR_APP_VALIDATE =2: Application is validated by checking its CRC.
 *      An invalid CRC over the whole Application area will keep the mcu in MSPBoot
 *
 *  CONFIG_MI_MEMORY_RANGE_CHECK = defined: Address range of erase and Write
 *      operations are validated. BSL-based commands can write/erase any area
 *      including MSPBoot, but this defition prevents modifications to area outside
 *      of Application
 *
 *  CONFIG_CI_PHYDL_COMM_SHARED = defined: Communication interface can be used by
 *      application. Application can call MSPBoot initialization and poll routines
 *      to use I2C.
 *
 *  CONFIG_CI_PHYDL_START_CALLBACK = defined: USI I2C Start callback is required by
 *      BSL-based protocol and is implemented
 */
#elif defined(MSPBoot_BSL)
#define NDEBUG
#define CONFIG_APPMGR_APP_VALIDATE    (2)
#define CONFIG_MI_MEMORY_RANGE_CHECK
#undef  CONFIG_CI_PHYDL_TIMEOUT
#define CONFIG_CI_PHYDL_COMM_SHARED
#define CONFIG_CI_PHYDL_START_CALLBACK
#undef CONFIG_CI_PHYDL_END_CALLBACK
#undef  CONFIG_CI_PHYDL_ERROR_CALLBACK

/* MSPBoot SMBUS:
 *  If MSPBoot_SMBUS is defined for the project (in this file or in project preprocessor
 *  options), the following settings will apply:
 *
 *  NDEBUG = defined: Debugging is disabled, ASSERT_H function is ignored,
 *      GPIOs are not used for debugging
 *
 *  CONFIG_APPMGR_APP_VALIDATE =2: Application is validated by checking its CRC.
 *      An invalid CRC over the whole Application area will keep the mcu in MSPBoot
 *
 *  CONFIG_MI_MEMORY_RANGE_CHECK = defined: Address range of erase and Write
 *      operations are validated. BSL-based commands can write/erase any area
 *      including MSPBoot, but this defition prevents modifications to area outside
 *      of Application
 *
 *  CONFIG_CI_PHYDL_COMM_SHARED = defined: Communication interface can be used by
 *      application. Application can call MSPBoot initialization and poll routines
 *      to use I2C.
 *
 *  CONFIG_CI_PHYDL_START_CALLBACK = defined: USI I2C Start callback is required by
 *      BSL-based protocol and is implemented
 */
#elif defined(MSPBoot_SMBUS)
#define NDEBUG
#define CONFIG_APPMGR_APP_VALIDATE    (3)
#define CONFIG_MI_MEMORY_RANGE_CHECK
#define CONFIG_CI_PHYDL_TIMEOUT
#define CONFIG_CI_PHYDL_COMM_SHARED
#define CONFIG_CI_PHYDL_START_CALLBACK
#define CONFIG_CI_PHYDL_END_CALLBACK
#define  CONFIG_CI_PHYDL_ERROR_CALLBACK
#else
#error "Define a proper configuration in TI_MSPBoot_Config.h or in project preprocessor options"
#endif

#endif            //__TI_MSPBoot_CONFIG_H__
