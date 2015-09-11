/******************************************************************************/
/* lnk_msp430g2452.cmd - LINKER COMMAND FILE FOR LINKING MSP430G2452 PROGRAMS     */
/*                                                                            */
/*   Usage:  lnk430 <obj files...>    -o <out file> -m <map file> lnk.cmd     */
/*           cl430  <src files...> -z -o <out file> -m <map file> lnk.cmd     */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* These linker options are for command line linking only.  For IDE linking,  */
/* you should set your linker options in Project Properties                   */
/* -c                                               LINK USING C CONVENTIONS  */
/* -stack  0x0100                                   SOFTWARE STACK SIZE       */
/* -heap   0x0100                                   HEAP AREA SIZE            */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/****************************************************************************/
/* SPECIFY THE SYSTEM MEMORY MAP                                            */
/****************************************************************************/
/* The following definitions can be changed to customize the memory map for a different device
 *   or other adjustments
 *  Note that the changes should match the definitions used in MEMORY and SECTIONS
 *
 */
/* RAM Memory Addresses */
__RAM_Start = 0x2400;                           /* RAM Start for F5529 is 0x2400              */
__RAM_End = 0x43FF;                             /* RAM End for F5529 is 0x43FF                */
    /* RAM shared between App and Bootloader, must be reserved */
    PassWd = 0x2400;                            /* Password sent by App to force boot  mode   */
    StatCtrl = 0x2402;                          /* Status and Control  byte used by Comm      */
    CI_State_Machine = 0x2403;                 /* State machine variable used by Comm    */
    CI_Callback_ptr = 0x2404;                   /* Pointer to Comm callback structure         */
    /* Unreserved RAM used for Bootloader or App purposes */
    _NonReserved_RAM_Start = 0x2406;            /* Non-reserved RAM                           */

/* Flash memory addresses */
__Flash_Start = 0x4400;                         /* Start of Flash for F5529 is 0x4400         */
__Flash_End = 0xFFFF;                    /* End of Flash - Bank A                      */
    /* Reserved Flash locations for Bootloader Area */
    __Boot_Start = 0xF800;                          /* Boot reserves 2kB (0x13C00-0x143FF)           */
    __Boot_Reset = 0xFFFE;                          /* Boot reset vector                            */
    __Boot_End = 0xFFFF;
    __Boot_VectorTable = 0xFF80;                    /* Boot vector table (0xFFE0 for F5529)         */
    __Boot_SharedCallbacks_Len = 4;                 /* Length of shared callbacks (2 calls = 4B)     */
    __Boot_SharedCallbacks = (__Boot_VectorTable-__Boot_SharedCallbacks_Len); /* Start of Shared callbacks */
    /* Reserved Flash locations for Application Area */
    _App_Isr_USCIA1 = 0xF766;
    _App_Isr_TrapIsr = 0xF786;
    _AppChecksum = (__Flash_Start);                 /* CRC16 of Application                         */
    _AppChecksum_8 = (__Flash_Start+2);             /* CRC8 of Application                          */
    _App_Start = (__Flash_Start+4);                 /* Application Area                             */
    _App_End = (__Boot_Start-1);                    /* End of application area (before boot)        */
    _CRC_Size = (_App_End - _App_Start +1);         /* Number of bytes calculated for CRC           */
    _App_Vectors = 22;                               /* Number of used interrupt vectors             */
    _App_Reset_Vector = (__Boot_Start-2);           /* Address of Application reset vector (0xFDFE) */
    _App_Proxy_Vector_Start = (_App_Reset_Vector-(_App_Vectors*4)); /* Proxy interrupt table(0xFDCE)*/

/* MEMORY definition, adjust based on definitions above */

MEMORY
{
    SFR                     : origin = 0x0000, length = 0x0010
    PERIPHERALS_8BIT        : origin = 0x0010, length = 0x00F0
    PERIPHERALS_16BIT       : origin = 0x0100, length = 0x0100
    // RAM from _NonReserved_RAM_Start - __RAM_End
    RAM                     : origin = 0x2406, length = 0x1FFA
    USBRAM                  : origin = 0x1C00, length = 0x0800
    INFOA                   : origin = 0x1980, length = 0x0080
    INFOB                   : origin = 0x1900, length = 0x0080
    INFOC                   : origin = 0x1880, length = 0x0080
    INFOD                   : origin = 0x1800, length = 0x0080
    // Flash from __Boot_Start -( __Boot_SharedCallbacks or INT_VECTOR_TABLE)
    //FLASH                   : origin = 0xFC00, length = 0x03DC
    FLASH                   : origin = 0xF800, length = 0x077C
    FLASH2                  : origin = 0x10000,length = 0x14400
    // Shared callbacks from __Boot_SharedCallbacks + Len (when used)
    BOOT_SHARED_CALLBACKS   : origin = 0xFF7C, length = 0x04
    // Boot vector Table from __Boot_VectorTable- __Boot_Reset
    INT_VECTOR_TABLE        : origin = 0xFF80, length = 0x007E
    // Boot reset from __Boot_Reset-__Flash_End
    RESET                   : origin = 0xFFFE, length = 0x0002
}

/****************************************************************************/
/* SPECIFY THE SECTIONS ALLOCATION INTO MEMORY                              */
/****************************************************************************/

SECTIONS
{
    .bss        : {} > RAM                /* GLOBAL & STATIC VARS              */
    .data       : {} > RAM                /* GLOBAL & STATIC VARS              */
    .sysmem     : {} > RAM                /* DYNAMIC MEMORY ALLOCATION AREA    */
    .stack      : {} > RAM (HIGH)         /* SOFTWARE SYSTEM STACK             */

    .text       : {}>> FLASH | FLASH2     /* CODE                              */
    .text:_isr  : {} > FLASH              /* ISR CODE SPACE                    */
    .cinit      : {} > FLASH              /* INITIALIZATION TABLES             */
//#ifdef (__LARGE_DATA_MODEL__)
    .const      : {} > FLASH | FLASH2     /* CONSTANT DATA                     */
//#else
//    .const      : {} > FLASH              /* CONSTANT DATA                     */
//#endif
    .cio        : {} > RAM                /* C I/O BUFFER                      */

    .infoA     : {} > INFOA              /* MSP430 INFO FLASH MEMORY SEGMENTS */
    .infoB     : {} > INFOB
    .infoC     : {} > INFOC
    .infoD     : {} > INFOD

    .BOOT_APP_VECTORS : {} > BOOT_SHARED_CALLBACKS
    /* MSP430 INTERRUPT VECTORS          */
    .BOOT_VECTOR_TABLE : {} > INT_VECTOR_TABLE

    .reset       : {}               > RESET  /* MSP430 RESET VECTOR         */ 
}

/****************************************************************************/
/* INCLUDE PERIPHERALS MEMORY MAP                                           */
/****************************************************************************/

-l msp430f5529.cmd

