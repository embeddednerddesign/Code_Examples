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
__RAM_Start = 0x0200;                           /* RAM Start for G2553 is 0x0200              */
__RAM_End = 0x03FF;                             /* RAM End for G2553 is 0x03FF                */
    /* RAM shared between App and Bootloader, must be reserved */
    PassWd = 0x0200;                            /* Password sent by App to force boot  mode   */
    StatCtrl = 0x0202;                          /* Status and Control  byte used by Comm      */
    CI_State_Machine = 0x0203;                  /*  State machine variable used by Comm       */
    CI_Callback_ptr = 0x0204;                   /* Pointer to Comm callback structure         */
    /* Unreserved RAM used for Bootloader or App purposes */
    _NonReserved_RAM_Start = 0x0206;            /* Non-reserved RAM                           */

/* Flash memory addresses */
__Flash_Start = 0xC000;                         /* Start of Flash for G2553 is 0xC000         */
__Flash_End = 0xFFFF;                           /* End of Flash                               */
    /* Reserved Flash locations for Bootloader Area */
    __Boot_Start = 0xFE00;                          /* Boot reserves 512B (0xFC00-0xFFFF)           */
    __Boot_Reset = 0xFFFE;                          /* Boot reset vector                            */
    __Boot_VectorTable = 0xFFE0;                    /* Boot vector table (0xFFE0 for G2452)         */
    __Boot_SharedCallbacks_Len = 4;                 /* Lenght of shared callbacks (2 calls =4B)     */
    __Boot_SharedCallbacks = (__Boot_VectorTable-__Boot_SharedCallbacks_Len); /* Start of Shared callbacks */
    /* Reserved Flash locations for Application Area */
    _AppChecksum = (__Flash_Start);                 /* CRC16 of Application                         */
    _AppChecksum_8 = (__Flash_Start+2);             /* CRC8 of Application                          */
    _App_Start = (__Flash_Start+3);                 /* Application Area                             */
    _App_End = (__Boot_Start-1);                    /* End of application area (before boot)        */
    _CRC_Size = (_App_End - _App_Start +1);         /* Number of bytes calculated for CRC           */
    _App_Vectors =12;                               /* Number of used interrupt vectors             */
    _App_Reset_Vector = (__Boot_Start-2);           /* Address of Application reset vector          */
    _App_Proxy_Vector_Start = (_App_Reset_Vector-(_App_Vectors*4)); /* Proxy interrupt table        */


/* MEMORY definition, adjust based on definitions above */
MEMORY
{
    SFR                     : origin = 0x0000, length = 0x0010
    PERIPHERALS_8BIT        : origin = 0x0010, length = 0x00F0
    PERIPHERALS_16BIT       : origin = 0x0100, length = 0x0100
    // RAM from _NonReserved_RAM_Start - __RAM_End
    RAM                     : origin = 0x0206, length = 0x01FA
    INFOA                   : origin = 0x10C0, length = 0x0040
    INFOB                   : origin = 0x1080, length = 0x0040
    INFOC                   : origin = 0x1040, length = 0x0040
    INFOD                   : origin = 0x1000, length = 0x0040
    // Flash from __Boot_Start -( __Boot_SharedCallbacks or INT_VECTOR_TABLE)
    FLASH                   : origin = 0xFE00, length = 0x01DC
    // Shared callbacks from __Boot_SharedCallbacks + Len (when used)
    BOOT_SHARED_CALLBACKS   : origin = 0xFFDC, length = 0x04
    // Boot vector Table from __Boot_VectorTable- __Boot_Reset
    INT_VECTOR_TABLE        : origin = 0xFFE0, length = 0x001E
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

    .text       : {} >> FLASH |INFOD | INFOC | INFOB   /* CODE                 */
    .cinit      : {} >> FLASH |INFOD | INFOC | INFOB   /* INITIALIZATION TABLES*/
    .const      : {} >> FLASH |INFOD | INFOC | INFOB   /* CONSTANT DATA        */
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

-l msp430g2553.cmd

