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
#include "TI_MSPBoot_MI.h"


//
//  Functions declarations
//
/******************************************************************************
*
 * @brief   Erase a Flash Sector
 *
 * @param  addr    Address in the sector being erased (sector is 512B)
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_EraseSector(uint16_t addr)
{
#ifdef CONFIG_MI_MEMORY_RANGE_CHECK
    // Check address to be within Application range
    if ((addr < APP_START_ADDR) || (addr > APP_END_ADDR))
        return RET_PARAM_ERROR;
#endif
    FCTL1 = FWKEY + ERASE;                      // Enable flash erase
#if (MCLK==1000000)
    FCTL2 = FWKEY + FSSEL_2 + 2;              // Flash timing setup (SMCLK/3) = 333.333Khz
#elif (MCLK==4000000)
    FCTL2 = FWKEY + FSSEL_2 + 8;              // Flash timing setup (SMCLK/9) = 444.444Khz
#elif (MCLK==8000000)
    FCTL2 = FWKEY + FSSEL_2 + 16;             // Flash timing setup (SMCLK/17) = 470.588Khz
#else
#error "Please define a valid MCLK or add configuration"
#endif
    FCTL3 = FWKEY;                              // Disable lock
    *(unsigned int *)addr = 0;                  // Dummy write to erase flash
    FCTL1 = FWKEY;
    FCTL3 = FWKEY+LOCK;                         // Diasble flash write

    return RET_OK;
}

/******************************************************************************
 *
 * @brief   Erase the application area (address obtained from linker file)
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_EraseApp(void)
{
    uint16_t addr;

    for (addr = APP_END_ADDR; addr >APP_START_ADDR; addr -= 512)
    {
        TI_MSPBoot_MI_EraseSector(addr);
    }
}

/******************************************************************************
 *
 * @brief   Write a Byte to Flash memory
 *
 * @param  addr     Address of the Byte being written
 * @param  data     Byte being written
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_WriteByte(uint16_t addr, uint8_t data)
{

#ifdef CONFIG_MI_MEMORY_RANGE_CHECK
    if ((addr < APP_START_ADDR) || (addr > APP_END_ADDR))
        return RET_PARAM_ERROR;
#endif

    FCTL3 = FWKEY;
    FCTL1 = FWKEY+ WRT;                         // Enable flash write
    *(unsigned char*)addr = data;               // Write data to flash
    FCTL1 = FWKEY;                              // Disable flash write
    FCTL3 = FWKEY + LOCK;

    return RET_OK;
}
