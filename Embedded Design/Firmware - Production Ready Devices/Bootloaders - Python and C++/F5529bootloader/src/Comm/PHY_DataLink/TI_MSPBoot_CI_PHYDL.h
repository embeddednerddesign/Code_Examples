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

#ifndef __TI_MSPBoot_CI_PHYDL_H__
#define __TI_MSPBoot_CI_PHYDL_H__

#include <stdbool.h>

#ifdef USE_PHYDL_CALLBACKS
typedef struct  {
    void (*StartCallback)(void);
    void (*RxCallback)(uint8_t);
    void (*TxCallback)(uint8_t *);
    void (*EndCallback)(void);
    void (*ErrorCallback)(uint8_t);
}t_CI_Callback;

//
// Include files
//
/*! I2C Slave Address implemented by the I2C driver */
#define I2C_SLAVE_ADDR      (0x40)

#define I2C_SLAVE_ADDR_R    ((I2C_SLAVE_ADDR<<1)|BIT0)
#define I2C_SLAVE_ADDR_W    ((I2C_SLAVE_ADDR<<1))

//
// Function prototypes
//
void TI_MSPBoot_CI_PHYDL_Init(t_CI_Callback * CI_Callback);
void TI_MSPBoot_CI_PHYDL_Poll(void);

#else

/* Dummy function, we don't need it when callbacks are removed */
#define TI_MSPBoot_CI_PHYDL_Poll()

/*
 * Initialize the PHYDL module
 */
void TI_MSPBoot_CI_PHYDL_Init(void);

/*
 * Read a byte if one is available
 */
bool TI_MSPBoot_CI_PHYDL_read(uint8_t *byte     /* set to byte value */
                              );                /* true when byte is valid */

/*
 * Write a byte, blocks until the byte cam be written
 */
bool TI_MSPBoot_CI_PHYDL_write(uint8_t *byte,   /* data pointer */
                               uint8_t length   /* number of bytes to write */
                               );               /* returns true on success */

/*
 * Write a single byte to the uart
 */
bool TI_MSPBoot_CI_PHYDL_writeByte(uint8_t byte     /* byte to send */
                                   );               /* returns true on success */

#endif

void TI_MSPBoot_CI_PHYDL_disable(void);
void TI_MSPBoot_CI_PHYDL_reenable(void);

#endif //__TI_MSPBoot_CI_PHYDL_H__
