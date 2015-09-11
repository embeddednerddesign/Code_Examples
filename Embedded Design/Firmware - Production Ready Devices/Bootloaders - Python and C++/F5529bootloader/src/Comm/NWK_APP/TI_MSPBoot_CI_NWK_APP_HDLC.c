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
#include <stdbool.h>

#include "msp430.h"
#include "TI_MSPBoot_Common.h"
#include "TI_MSPBoot_CI.h"
#include "TI_MSPBoot_MI.h"
#include "Crc.h"

//
// Macros and definitions
//
/*! MSPBoot version sent as response of COMMAND_TX_VERSION command  */
#define MSPBoot_VERSION                   0xA0

//
//  MSPBoot Commands
//
/* Mass erase with a password */
#define COMMAND_RX_PASSWORD             0x11
/*! Erase a segment :
    0x80    LEN=0x03    0x12    ADDRL   ADDRH   CHK_L   CHK_H   */
#define COMMAND_ERASE_SEGMENT           0x12
/*! Erase application area:
    0x80    LEN=0x01    0x15    CHK_L   CHK_H   */
#define COMMAND_ERASE_APP               0x15
/*! Receive data block :
    0x80    LEN=3+datapayload   0x10    ADDRL   ADDRH   DATA0...DATAn   CHK_L   CHK_H   */
#define COMMAND_RX_DATA_BLOCK           0x10
/*! Transmit MSPBoot version :
    0x80    LEN=0x01    0x19    CHK_L   CHK_H   */
#define COMMAND_TX_VERSION              0x19
/*! Jump to application:
    0x80    LEN=0x01    0x1C    CHK_L   CHK_H   */
#define COMMAND_JUMP2APP                0x1C

//  MSPBoot responses
//
#define RESPONSE_OK                     0x00    /*! Command processed OK */
#define RESPONSE_HEADER_ERROR           0x51    /*! Incorrect header */
#define RESPONSE_CHECKSUM_ERROR         0x52    /*! Packet checksum error */
#define RESPONSE_PACKETZERO_ERROR       0x53    /*! Packet size is zero */
#define RESPONSE_PACKETSIZE_ERROR       0x54    /*! Packet size is invalid */
#define RESPONSE_UNKNOWN_ERROR          0x55    /*! Error in protocol */
#define RESPONSE_INCORRECT_COMMAND      0x56    /*! Invalid command */

#define BSL_MESSAGE_RESPONSE            0x3b    /* BSL core msg response */
#define BSL_CMD_RESPONSE                0x3a    /* BSL core command response */

#define BSL_RESPONSE_OK                 0x00    /* Operation successful */
#define BSL_RESPONSE_FLASH_WRITE_FAIL   0x01    /* Flash write check failed */
#define BSL_RESPONSE_FLASH_BIT_FAIL     0x02    /* Flash fail bit set */
#define BSL_RESPONSE_VOLTAGE_CHANGE     0x03    /* Voltage change during program */
#define BSL_RESPONSE_BSL_LOCKED         0x04    /* BSL locked */
#define BSL_RESPONSE_BSL_PASSWORD_ERR   0x05    /* BSL password error */
#define BSL_RESPONSE_WRITE_FORBIDDEN    0x06    /* Byte write forbidden */
#define BSL_RESPONSE_UNKNOWN_COMMAND    0x07    /* Unknown command */
#define BSL_RESPONSE_LENGTH_ERROR       0x08    /* Packet length exceeds buffer size */

/* packet byte order */
#define PACKET_PROTOCOL         0
#define PACKET_VERSION          1
#define PACKET_SOURCE           2
#define PACKET_DESTINATION      3
#define PACKET_SEQUENCE         4
#define PACKET_COMMAND          5
#define PACKET_DATA_START       6
#define PACKET_BSL_LEN_LOW      6
#define PACKET_BSL_LEN_HIGH     7
#define PACKET_BSL_CMD          8
#define PACKET_BSL_ADDR_LOW     9
#define PACKET_BSL_ADDR_MED     10
#define PACKET_BSL_ADDR_HIGH    11
#define PACKET_BSL_DATA1        12
#define PACKET_BSL_DATA256      (PACKET_BSL_DATA1+256)
#define PACKET_BSL_CRC_LOW      (PACKET_BSL_DATA256+1)
#define PACKET_BSL_CRC_HIGH     (PACKET_BSL_CRC_LOW+1)
#define PAYLOAD_MAX_SIZE        (PACKET_BSL_CRC_HIGH+1)

// see "Generator Communications.doc"
#define MSP430_BSL_PROTOCOL 0x01


/* packet commands */
#define ACK             0xaa
//#define NAK             0x55  // Not used in this code and conflicts with previous define in msp430f5529.h
#define BSL_COMMAND     0x10
#define BSL_RESPONSE    0x11

/* BSL version information */
#define BSL_VERSION_MANUFACTURER        0x01
#define BSL_VERSION_COMMAND_VERSION     0x01
#define BSL_VERSION_API_VERSION         0x01
#define BSL_VERSION_PERIPHERAL_VERSION  0x50

/* protocol framing character */
#define FLAG            0x7e
#define CONTROLESCAPE   0x7d

/* mass erase password */
static const uint8_t gErasePassword[32] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
};

static struct
{
    uint8_t myAddr;                 /* my HDLC address */
    uint8_t data[PAYLOAD_MAX_SIZE]; /* packet data */
    int16_t length;                 /* packet length */
} gPacket;

//
//  Local function prototypes
//
static uint8_t CI_CMD_Rx_Data_Block(uint16_t addr, uint8_t *data, uint8_t len);
static uint8_t CI_CMD_Intepreter(void);
static bool readPacket(void);
static void writePacket(void);
static void packetReset(void);
static uint8_t verifyPacket(void);
static void ackPacket(uint8_t ack);

/******************************************************************************
 *
 * @brief   Initialize the Communication Interface
 *  All layers will be initialized
 *
 * @return none
 *****************************************************************************/
void TI_MSPBoot_CI_Init(void)
{
    TI_MSPBoot_CI_PHYDL_Init();
    packetReset();
    gPacket.myAddr = 128;
}

/*
 * Reset the packet length
 */
static void packetReset(void)
{
    gPacket.length = -1;
}

/*
 * Verify the received packet
 */
static uint8_t verifyPacket(void)   /* return the response code */
{
    uint8_t bslLength;
    uint8_t frameChecksum, i;
    uint16_t packetCRC;
    uint16_t calculatedCRC;

    /* check for the minimum packet length */
    if ((gPacket.length < 11) ||
        (gPacket.data[PACKET_COMMAND] != BSL_COMMAND))
    {
        return RESPONSE_INCORRECT_COMMAND;
    }

    /* check the received BSL command length */
    bslLength = gPacket.data[PACKET_BSL_LEN_LOW];
    if ((gPacket.data[PACKET_BSL_LEN_HIGH] != 0) ||
        ((gPacket.length - bslLength) != 10))
    {
        return RESPONSE_PACKETSIZE_ERROR;
    }

    /* verify the frameChecksum */
    frameChecksum = 0;
    for (i = 0; i <= gPacket.length; i++) {
    	frameChecksum += gPacket.data[i];
    }
    if (frameChecksum != 0)
    {
        return RESPONSE_CHECKSUM_ERROR;
    }

    /* verify the packet CRC */
  
    packetCRC = gPacket.data[gPacket.length - 1];
    packetCRC <<= 8;
    packetCRC += gPacket.data[gPacket.length - 2];
    /* add 2 to bslLength so CRC the length fields too */
    bslLength += 2;
    calculatedCRC = crc16MakeBitwise(&gPacket.data[PACKET_BSL_LEN_LOW], bslLength);
    if (packetCRC != calculatedCRC)
    {
      return RESPONSE_CHECKSUM_ERROR;
    }

    return ACK;
}

/*
 * Acknowledge a received packet
 */
static void ackPacket(uint8_t ack  /* ack return value */)
{
    gPacket.data[PACKET_DESTINATION] = gPacket.data[PACKET_SOURCE];
    gPacket.data[PACKET_SOURCE] = gPacket.myAddr;
    gPacket.data[PACKET_COMMAND] = ack;
    gPacket.length = 6;
    writePacket();
}

/*
 * update the response packet
 */
static void updatePacket(void)
{
    uint16_t crc;
    uint8_t bslLength;

    gPacket.data[PACKET_COMMAND] = BSL_RESPONSE;
    bslLength = gPacket.data[PACKET_BSL_LEN_LOW];
    crc = crc16MakeBitwise(&gPacket.data[PACKET_BSL_LEN_LOW], bslLength + 2);
    gPacket.data[PACKET_BSL_CMD + bslLength] = crc;
    bslLength++;
    gPacket.data[PACKET_BSL_CMD + bslLength] = crc >> 8;
    bslLength++;
    gPacket.length = bslLength + PACKET_BSL_CMD;
}

/******************************************************************************
 *
 * @brief   On packet reception, process the data
 *  BSL-based protocol expects:
 *  HEADER  = 0x80
 *  Lenght  = lenght of CMD + [ADDR] + [DATA]
 *  CMD     = 1 byte with the corresponding command
 *  ADDR    = optional address depending on command
 *  DATA    = optional data depending on command
 *  CHKSUM  = 2 bytes (L:H) with CRC checksum of CMD + [ADDR] + [DATA]
 *
 * @return RET_OK: Communication protocol in progress
 *         RET_JUMP_TO_APP: Last byte received, request jump to application
 *****************************************************************************/
uint8_t TI_MSPBoot_CI_Process(void)
{
    uint8_t ret = RET_NO_DATA;
    uint8_t sendAck;

    if (readPacket())    // On complete packet reception
    {
        if ((gPacket.length > PACKET_DESTINATION) && 
            (gPacket.data[PACKET_PROTOCOL] == MSP430_BSL_PROTOCOL) && 
            (gPacket.data[PACKET_DESTINATION] == gPacket.myAddr)) {
            sendAck = verifyPacket();
            ackPacket(sendAck);
            if (sendAck == ACK)
            {
                /* CI_CMD_Intepreter will set up the packet response */
                ret = CI_CMD_Intepreter();
                updatePacket();
                writePacket();
            }
        }

        packetReset();
    }

    return ret;
}


/******************************************************************************
 *
 * @brief   Process a packet checking the command and sending a response
 *  New commands can be added in this switch statement
 *
 * @param RxData Pointer to buffer with received data including command
 * @param RxLen  Lenght of Received data
 * @param TxData Pointer to buffer which will be updated with a response
 * @param TxLen  Lenght of Response
 *
 * @return RET_OK: Communication protocol in progress
 *         RET_JUMP_TO_APP: Last byte received, request jump to application
 *         RET_PARAM_ERROR: Incorrect command
 *****************************************************************************/
static uint8_t CI_CMD_Intepreter(void)
{
    uint16_t addr;
    uint8_t length;
    bool passwordOk;

    switch (gPacket.data[PACKET_BSL_CMD])
    {
        case COMMAND_RX_PASSWORD:
            /* Erase the application with a password */
            length = gPacket.data[PACKET_BSL_LEN_LOW];
            passwordOk = false;
            if (length == 33)
            {
                passwordOk = true;
                for (addr = 0; addr < (length - 1); addr++)
                {
                    if (gPacket.data[addr + PACKET_BSL_ADDR_LOW] != gErasePassword[addr])
                    {
                        passwordOk = false;
                    }
                }
            }
            if (!passwordOk)
            {
                /* incorrect password */
                gPacket.data[PACKET_BSL_LEN_LOW] = 2;
                gPacket.data[PACKET_BSL_CMD] = BSL_MESSAGE_RESPONSE;
                gPacket.data[PACKET_BSL_CMD+1] = BSL_RESPONSE_BSL_PASSWORD_ERR;
                break;
            }
        case COMMAND_ERASE_APP:
            // Erase the application area
            TI_MSPBoot_MI_EraseApp();
            gPacket.data[PACKET_BSL_LEN_LOW] = 2;
            gPacket.data[PACKET_BSL_CMD] = BSL_MESSAGE_RESPONSE;
            gPacket.data[PACKET_BSL_CMD+1] = BSL_RESPONSE_OK;
        break;
        case COMMAND_RX_DATA_BLOCK:
            // Receive and program a data block specified by an address
            addr = gPacket.data[PACKET_BSL_ADDR_LOW];
            addr += gPacket.data[PACKET_BSL_ADDR_MED] << 8;
            /* subtract the address and cmd bytes from the length */
            length = gPacket.data[PACKET_BSL_LEN_LOW] - 4;
            gPacket.data[PACKET_BSL_LEN_LOW] = 2;
            gPacket.data[PACKET_BSL_CMD] = BSL_MESSAGE_RESPONSE;
            gPacket.data[PACKET_BSL_CMD+1] = CI_CMD_Rx_Data_Block(addr, 
                                                &gPacket.data[PACKET_BSL_DATA1], 
                                                length);
        break;
        case COMMAND_ERASE_SEGMENT:
            // Erase an application area sector as defined by the address
            gPacket.data[PACKET_BSL_LEN_LOW] = 2;
            gPacket.data[PACKET_BSL_CMD] = BSL_MESSAGE_RESPONSE;
            addr = gPacket.data[PACKET_BSL_ADDR_LOW];
            addr += gPacket.data[PACKET_BSL_ADDR_MED] << 8;
            if (TI_MSPBoot_MI_EraseSector(addr) == RET_OK)
            {
                gPacket.data[PACKET_BSL_CMD+1] = BSL_RESPONSE_OK;
            }
            else
            {
                gPacket.data[PACKET_BSL_CMD+1] = BSL_RESPONSE_FLASH_WRITE_FAIL;
            }
        break;
        case COMMAND_TX_VERSION:
            // Transmit MSPBoot version
            gPacket.data[PACKET_BSL_LEN_LOW] = 5;
            gPacket.data[PACKET_BSL_CMD] = BSL_CMD_RESPONSE;
            gPacket.data[PACKET_BSL_CMD+1] = BSL_VERSION_MANUFACTURER;
            gPacket.data[PACKET_BSL_CMD+2] = BSL_VERSION_COMMAND_VERSION;
            gPacket.data[PACKET_BSL_CMD+3] = BSL_VERSION_API_VERSION;
            gPacket.data[PACKET_BSL_CMD+4] = BSL_VERSION_PERIPHERAL_VERSION;
        break;
        case COMMAND_JUMP2APP:
            // Jump to Application
            return RET_JUMP_TO_APP;
        //break;
        default:
            return RET_PARAM_ERROR;
        //break;
    }

    return RET_OK;
}


/******************************************************************************
 *
 * @brief   Receive a block of data
 *
 * @param addr  Start address (16-bit) or area being programmed
 * @param data  Pointer to data being written
 * @param len   Lenght of data being programmed
 *
 * @return  RESPONSE_OK: Result OK
 *          RESPONSE_UNKNOWN_ERROR: Error writting the data
 *****************************************************************************/
static uint8_t CI_CMD_Rx_Data_Block(uint16_t addr, uint8_t *data, uint8_t len)
{
    uint8_t i;
    for (i=0; i < len; i++)
    {
        if ( TI_MSPBoot_MI_WriteByte(addr++, data[i]) != RET_OK)
            return BSL_RESPONSE_FLASH_WRITE_FAIL;
    }

    return BSL_RESPONSE_OK;
}

/*
 * Read a packet, return true if a packet has been received
 */
bool readPacket(void)
{
    uint8_t byte;
    static uint8_t previous;

    if (!TI_MSPBoot_CI_PHYDL_read(&byte))
    {
        return false;
    }

    /* detect the start and end of the packet */
    if (byte == FLAG)
    {
        if (gPacket.length > 0)
        {
            /* the packet is finished */
        	gPacket.length--; // its one to many, now point to checksum byte
            return true;
        }

        /* get ready to receive a packet */
        gPacket.length = 0;
        previous = 0;
        return false;
    }

    if ((gPacket.length > 0) &&
        (previous == CONTROLESCAPE))
    {
        /* replace the previous CONTROLESCAPE data with the correct value */
        gPacket.data[gPacket.length - 1] = byte ^ 0x20;
    }
    else if (gPacket.length > -1)
    {
        gPacket.data[gPacket.length++] = byte;
    }
    previous = byte;

    if (gPacket.length > (PAYLOAD_MAX_SIZE - 1))
    {
        /* we received too many bytes */
        packetReset();
    }

    return false;
}

/*
 * Write a packet
 */
void writePacket(void)
{
    uint8_t byte, i;
    uint8_t frameChecksum;

	frameChecksum = 0;

    __delay_cycles(120);

    TI_MSPBoot_CI_PHYDL_writeByte(FLAG);

    for (i = 0; i < gPacket.length; i++)
    {

    	byte = gPacket.data[i];
        frameChecksum += byte;

        if ((byte == FLAG) || (byte == CONTROLESCAPE))
        {
            TI_MSPBoot_CI_PHYDL_writeByte(CONTROLESCAPE);
            byte ^= 0x20;
        }

        TI_MSPBoot_CI_PHYDL_writeByte(byte);
    }

    frameChecksum = (256 - frameChecksum);
    if (frameChecksum == FLAG) {
    	TI_MSPBoot_CI_PHYDL_writeByte(CONTROLESCAPE);
    	frameChecksum ^= 0x20;
    }
    TI_MSPBoot_CI_PHYDL_writeByte(frameChecksum);

    TI_MSPBoot_CI_PHYDL_writeByte(FLAG);

	while(UCA0STAT & UCBUSY);

    __delay_cycles(120);

}
