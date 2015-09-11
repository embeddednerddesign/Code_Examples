#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

"""\
Brightleaf MSP430 BSL5 implementation using RS485.
"""

import sys
from msp430.bsl5 import bsl5
import serial
import struct
import logging
import time

from optparse import OptionGroup
import msp430.target
import msp430.memory


# possible answers
BSL5_UART_ERROR_CODES = {
        0x51: 'Header incorrect',
        0x52: 'Checksum incorrect',
        0x53: 'Packet size zero',
        0x54: 'Packet size exceeds buffer',
        0x55: 'Unknown error',
        0x56: 'Unknown baud rate',
}

BSL5_ACK = '\x00'

PACKET_PROTOCOL = 0x01
PACKET_VERSION = 0x01
PACKET_COMMAND_BSL = 0x10
PACKET_COMMAND_BSL_RESPONSE = 0x11
PACKET_COMMAND_ACK = 0xaa
PACKET_COMMAND_NAK = 0x55
PACKET_CRC_ERROR = 0x52
PACKET_SIZE_ERROR = 0x54
PACKET_COMMAND_ERROR = 0x56
PACKET_FLAG = 0x7e
PACKET_ESCAPE = 0x7d

def crc_update(crc, byte):
    x = ((crc >> 8) ^ ord(byte)) & 0xff
    x ^= x >> 4
    return ((crc << 8) ^ (x << 12) ^ (x << 5) ^ x) & 0xffff


class SerialBL485(bsl5.BSL5):
    """\
    Implementation of the BSL protocol over the serial port.
    """

    def __init__(self):
        bsl5.BSL5.__init__(self)
        self.serial = None
        self.logger = logging.getLogger('BL485')
        self.extra_timeout = None
        self.invertRST = False
        self.invertTEST = False
        self.swapResetTest = False
        self.testOnTX = False
        self.blindWrite = False
        # delay after control line changes
        self.control_delay = 0.05


    def open(self, port=0, baudrate=9600, ignore_answer=False):
        self.ignore_answer = ignore_answer
        self.logger.info('Opening serial port %r' % port)
        try:
            self.serial = serial.serial_for_url(
                port,
                baudrate=baudrate,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=1,
            )
        except AttributeError:  # old pySerial versions do not have serial_for_url
            print "serial",
            print baudrate
            self.serial = serial.Serial(
                port,
                baudrate=baudrate,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=1,
            )


    def __del__(self):
        self.close()


    def close(self):
        """Close serial port"""
        if self.serial is not None:
            self.logger.info('closing serial port')
            self.serial.close()
            self.serial = None

    def strtohex(self, s):
        "Convert a binary string to hexadecimal"

        return ":".join("0x{:02x}".format(ord(c)) for c in s)

    # interface specific commands

    # override the default password command because the brightleaf firmaware
    # actually returns the proper response
    def BSL_RX_PASSWORD(self, password):
        answer = self.bsl(bsl5.BSL_RX_PASSWORD, password, expect=2)
        self.check_answer(answer)

    # override the default write command because the brightleaf firmaware
    # actually returns the proper response
    def BSL_RX_DATA_BLOCK(self, address, data):
        packet = bsl5.three_bytes(address) + data
        answer = self.bsl(bsl5.BSL_RX_DATA_BLOCK, packet, expect=2)
        self.check_answer(answer)

    def encodeHDLC(self, packet):
        "Encode PACKET_FLAG and PACKET_ESCAPE chars properly"

        p = ''
        for b in packet:
            if (ord(b) == PACKET_FLAG) or (ord(b) == PACKET_ESCAPE):
                p += struct.pack('<BB', PACKET_ESCAPE, (ord(b) ^ 0x20))
            else:
                p += b

        return p

    def txHDLCpacket(self, packet):
        """\
        Low level access to the serial communication.

        This function sends a command. In case of a failure read timeout or 
        rejected commands by the slave, it will raise an exception.
        """

        self.logger.debug('Packet %s' % self.strtohex(packet))

        # start with the HDLC header
        txdata = struct.pack('<BB', PACKET_PROTOCOL, PACKET_VERSION)
        txdata += struct.pack('<BBB', self.options.sourceAddress, self.options.destAddress, 0)
        txdata += struct.pack('<B', PACKET_COMMAND_BSL)
        # now add the BSL packet information
        txdata += packet
        
        # add the frameChecksum
        frameChecksum = 0
        for c in txdata :
            frameChecksum = (frameChecksum + ord(c)) % 255
        frameChecksum = (255 - frameChecksum)
        txdata += chr(frameChecksum)

        # escape PACKET_FLAG and PACKET_ESCAPE characters and then
        # add the framing characters
        txdata = struct.pack('<B', PACKET_FLAG) + self.encodeHDLC(txdata)
        txdata += struct.pack('<B', PACKET_FLAG)

        self.logger.debug('HDLC Packet: %s' % self.strtohex(txdata))
        self.serial.write(txdata)
        self.serial.flush()

    def rxHDLCpacket(self):
        "Low level packet receive function"

        self.logger.debug('Waiting to receive packet...')

        # look for a PACKET_FLAG character
        rxpacket = ''
        flagStr = struct.pack('<B', PACKET_FLAG)
        escStr = struct.pack('<B', PACKET_ESCAPE)
        timeouts = 0
        tries = 0
        while (timeouts < 10) and (tries < 1000):
            tries += 1
            c = self.serial.read(1)
            if c != '':
                rxpacket += c
                if c == flagStr:
                    break
            else:
                timeouts += 1

        if c != flagStr:
            self.logger.debug('Received data: %s' % self.strtohex(rxpacket))
            raise bsl5.BSL5Error("Didn't receive packet start character")
        self.logger.debug('Found packet flag character')

        # receive the packet
        last_c = ''
        rxpacket = ''
        timeouts = 0
        tries = 0
        while (timeouts < 10) and (tries < 100):
            tries += 1
            c = self.serial.read(1)
            if c != '':
                if c == flagStr:
                    break
                elif last_c == escStr:
                    rxpacket += stuct.pack('<B', ord(c[0]) ^ 0x20)
                elif c != escStr:
                    rxpacket += c
                last_c = c
            else:
                timeouts += 1

        self.logger.debug('Received: %s' % self.strtohex(rxpacket))
        if c != flagStr:
            raise bsl5.BSL5Error("Packet didn't end with a frame character")

        return rxpacket

    def rxHDLCpacketWithAddr(self, address):
        "Receive a HDLC packet with the specified destination address"

        for attempt in range(10):
            packet = self.rxHDLCpacket()
            if (len(packet) > 4) and (ord(packet[3]) == address):
                return packet

        raise bsl5.BSL5Error("Didn't receive a packet with address %d" % address)

    def isACK(self, packet):
        "Return true if the packet is a HDLC ACK packet"

        if len(packet) != 7:
            raise bsl5.BSL5Error("Incorrect ack packet length")

        if ord(packet[5]) == PACKET_CRC_ERROR:
            raise bsl5.BSL5Error("Target reported a packet CRC error")

        if ord(packet[5]) == PACKET_SIZE_ERROR:
            raise bsl5.BSL5Error("Target reported a packet size error")

        if ord(packet[5]) == PACKET_COMMAND_ERROR:
            raise bsl5.BSL5Error("Target reported a packet command error")

        if ord(packet[5]) == PACKET_COMMAND_ACK:
            return True

        return False

    def decodeBSL(self, packet):
        "Return the BSL portion of the HDLC packet"

        if (len(packet) < 6) or (ord(packet[5]) != PACKET_COMMAND_BSL_RESPONSE):
            return ''
        return packet[6:(len(packet)-1)]

    def bsl(self, cmd, message='', expect=None):
        """\
        Low level access to the serial communication.

        This function sends a command and waits until it receives an answer
        (including timeouts). It will return a string with the data part of
        the answer. In case of a failure read timeout or rejected commands by
        the slave, it will raise an exception.

        If the parameter "expect" is not None, "expect" bytes are expected in
        the answer, an exception is raised if the answer length does not match.
        If "expect" is None, the answer is just returned.

        Frame format:
        +-----+----+----+-----------+----+----+
        | HDR | LL | LH | D1 ... DN | CL | CH |
        +-----+----+----+-----------+----+----+
        """
        # Sometimes this function is called with a messsage type of string and
        # other times message is a bytearray. This class assumes string.
        if type(message) == bytearray:
            message = "".join(map(chr, message))

        # send the command to the slave
        self.logger.debug('Command 0x%02x %s' % (cmd, message.encode('hex')))
        # prepare command with checksum
        txdata = struct.pack('<HB', 1+len(message), cmd) + message
        txdata += struct.pack('<H', reduce(crc_update, txdata, 0xffff))   # append checksum
        self.txHDLCpacket(txdata)

        # check for an ACK
        if not self.isACK(self.rxHDLCpacketWithAddr(self.options.sourceAddress)):
            raise bsl5.BSL5Error('No ACK received (timeout?)')
        self.logger.debug("Received ACK, reading response")

        # now check the response code
        # we need the entire packet without the FLAGS to calculate the frameChecksum
        rxpacket = self.rxHDLCpacketWithAddr(self.options.sourceAddress)
        # calculate the frameChecksum
        frameChecksum = 0
        for b in rxpacket :
            frameChecksum = (frameChecksum + ord(b)) % 255
        if frameChecksum != 0 :
            raise bsl5.BSL5Error('frameChecksum error')

        # now we only want the BSL portion of the packet
        rxpacket = self.decodeBSL(rxpacket)
#        rxpacket = self.decodeBSL(self.rxHDLCpacketWithAddr(self.options.sourceAddress))
        self.logger.debug('BSL packet: %s' % self.strtohex(rxpacket))
        if len(rxpacket) < 5: 
            raise bsl5.BSL5Error('Received packet is too short')

        packet_length, = struct.unpack_from("<H", rxpacket)
        # add 4 to packet length to account for the CRC and length fields and frameChecksum
        if len(rxpacket) != (packet_length + 4): 
            raise bsl5.BSL5Timeout('Packet length error, expected %d but got %d' % (packet_length+4, len(rxpacket)))

        # remove the length, command and crc from the returned data
        data = rxpacket[2:packet_length+2]

        crc, = struct.unpack("<H", rxpacket[packet_length+2:])
        crc_expected = reduce(crc_update, rxpacket[:packet_length+2], 0xffff)
        if crc != crc_expected:
            raise bsl5.BSL5Error('CRC error, expected 0x%04x but got 0x%04x' % (crc_expected, crc))

        if expect is not None and len(data) != expect:
            raise bsl5.BSL5Error('expected %d bytes, got %d bytes' % (expect, len(data)))

        return data

class SerialBSL5Target(SerialBL485, msp430.target.Target):
    """Combine the serial BSL backend and the common target code."""

    def __init__(self):
        msp430.target.Target.__init__(self)
        SerialBL485.__init__(self)
        self.patch_in_use = False

    def add_extra_options(self):
        group = OptionGroup(self.parser, "Communication settings")

        group.add_option("-p", "--port",
                dest="port",
                help="Use com-port",
                default=0)

        group.add_option("-s", "--source",
                dest="sourceAddress",
                type="int",
                help="rs485 source address (defaults to 0)",
                default=0)

        group.add_option("-d", "--dest",
                dest="destAddress",
                type="int",
                help="rs485 destination address (defaults to 128)",
                default=128)

        group.add_option("-B", "--baud-rate",
                dest="baud",
                type="int",
                help="baud rate to use (defaults to 9600)",
                default=9600)

        self.parser.add_option_group(group)

        group = OptionGroup(self.parser, "BSL settings")

        group.add_option("--no-start",
                dest="start_pattern",
                action="store_false",
                help="no not use ROM-BSL start pattern on RST+TEST/TCK",
                default=True)

        group.add_option("--password",
                dest="password",
                action="store",
                help="transmit password before doing anything else, password is given in given (TI-Text/ihex/etc) file",
                default=None,
                metavar="FILE")

        group.add_option("--ignore-answer",
                dest="ignore_answer",
                action="store_true",
                help="do not wait for answer to BSL commands",
                default=False)

        group.add_option("--control-delay",
                dest="control_delay",
                type="float",
                help="set delay in seconds (float) for BSL start pattern",
                default=0.01)

        self.parser.add_option_group(group)


    def parse_extra_options(self):
        if self.verbose > 1:   # debug infos
            if hasattr(serial, 'VERSION'):
                sys.stderr.write("pySerial version: %s\n" % serial.VERSION)


    def close_connection(self):
        self.close()


    def open_connection(self):
        self.logger = logging.getLogger('BL485')
        self.open(
            self.options.port,
            self.options.baud,
            ignore_answer = self.options.ignore_answer,
        )
        self.control_delay = self.options.control_delay

        if self.options.do_mass_erase:
            self.logger.info("Mass erase...")
            self.BSL_RX_PASSWORD('\xff'*30 + '\0'*2)
            time.sleep(1)
            # remove mass_erase from action list so that it is not done
            # twice
            self.remove_action(self.mass_erase)
        else:
            if self.options.password is not None:
                password = msp430.memory.load(self.options.password).get_range(0xffe0, 0xffff)
                self.logger.info("Transmitting password: %s" % (password.encode('hex'),))
                self.BSL_RX_PASSWORD(password)

        # configure the buffer
        #~ self.detect_buffer_size()

    # override reset method: use control line
    def reset(self):
        pass

def main():
    # run the main application
    bsl_target = SerialBSL5Target()
    bsl_target.main()

if __name__ == '__main__':
    main()
