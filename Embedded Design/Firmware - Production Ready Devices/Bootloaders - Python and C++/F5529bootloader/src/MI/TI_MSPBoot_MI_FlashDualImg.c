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
// Local Function prototypes
//
static uint8_t TI_MSPBoot_MI_EraseSectorDirect(uint16_t addr);
static uint8_t TI_MSPBoot_MI_WriteByteDirect(uint16_t addr, uint8_t data);
// Function prototypes
void TI_MSPBoot_MI_EraseAppDirect(uint8_t DownArea);
void TI_MSPBoot_MI_ReplaceApp(void);

//
//  Functions declarations
//
/******************************************************************************
*
 * @brief   Erase a Flash Sector 
 *        This function Erases the sector in Download area 
 *
 * @param  addr    Address in the sector being erased (sector is 512B)
 *                  The actual address will be in Download area
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_EraseSector(uint16_t addr)
{
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
    
#ifdef CONFIG_MI_MEMORY_RANGE_CHECK
    // Check address to be within Application range
    if ((addr < APP_START_ADDR) || (addr > APP_END_ADDR))
        return RET_PARAM_ERROR;
#endif
    // Erase address will be in Download area
    addr -= (APP_START_ADDR- ((uint16_t )&_FLASHDOWN_START));
    // Erase the corresponding area
    TI_MSPBoot_MI_EraseSectorDirect(addr);

    return RET_OK;
}

/******************************************************************************
*
 * @brief   Erase a Flash Sector in Flash Area
 *
 * @param  addr    Address in the sector being erased (sector is 512B)
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_EraseSectorDirect(uint16_t addr)
{
    // Erase the corresponding area
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
 *          Erases the application in Download Area
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_EraseApp(void)
{
    // Erase Download Area
    TI_MSPBoot_MI_EraseAppDirect(1);
 
}

/******************************************************************************
 *
 * @brief   Erase the application area (address obtained from linker file)
 *          It can erase application in Download or App Area
 *
 * @param DownArea 1:Erase Download area, 0: Erase App Area
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_EraseAppDirect(uint8_t DownArea)
{
    uint16_t addr;
    uint16_t Start, End;
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
    extern uint16_t _App_EndDown; // Define Address of Download area

    if (DownArea==0)
    {
        Start = APP_START_ADDR;
        End = APP_END_ADDR;
    }
    else
    {
        Start = ((uint16_t )&_FLASHDOWN_START);
        End = ((uint16_t )&_App_EndDown);
    }

    for (addr = End; addr >Start; addr -= 512)
    {
        TI_MSPBoot_MI_EraseSectorDirect(addr);
    }
}

/******************************************************************************
 *
 * @brief   Write a Byte to Flash memory
 *      This function writes the byte to Download area 
 *
 * @param  addr     Address of the Byte being written in Application area
 *                  The actual address will be in Download area
 * @param  data     Byte being written
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_WriteByte(uint16_t addr, uint8_t data)
{
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
#ifdef CONFIG_MI_MEMORY_RANGE_CHECK
    if ((addr < APP_START_ADDR) || (addr > APP_END_ADDR))
        return RET_PARAM_ERROR;
#endif
    // Write address will be in Download area
    addr -= (APP_START_ADDR- ((uint16_t )&_FLASHDOWN_START));
    // Write the byte
    TI_MSPBoot_MI_WriteByteDirect(addr, data);
    
    return RET_OK;
}

/******************************************************************************
 *
 * @brief   Write a Byte Directly to Flash memory
 *
 * @param  addr     Address of the Byte being written in Flash
 * @param  data     Byte being written
 *
 * @return  RET_OK when sucessful,
 *          RET_PARAM_ERROR if address is outside of Application area
 *****************************************************************************/
uint8_t TI_MSPBoot_MI_WriteByteDirect(uint16_t addr, uint8_t data)
{
    // Write directly to the corresponding area
    FCTL3 = FWKEY;
    FCTL1 = FWKEY+ WRT;                         // Enable flash write
    *(unsigned char*)addr = data;               // Write data to flash
    FCTL1 = FWKEY;                              // Disable flash write
    FCTL3 = FWKEY + LOCK;
    return RET_OK;
}


/******************************************************************************
 *
 * @brief   Erase the application area (address obtained from linker file)
 *          It can erase application in Download or App Area
 *
 * @param DownArea 1:Erase Download area, 0: Erase App Area
 *
 * @return  none
 *****************************************************************************/
void TI_MSPBoot_MI_ReplaceApp(void)
{
    uint16_t addr;
    uint8_t *data;
    extern uint16_t _FLASHDOWN_START; // Define Address of Download area
    extern uint16_t _App_EndDown; // Define Address of Download area

    data = (uint8_t *) (&_FLASHDOWN_START);
    for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr++)
    {
        TI_MSPBoot_MI_WriteByteDirect(addr, *data++);
    }
}
