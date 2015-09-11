#!/usr/bin/env python
import os,time,socket,sys,serial
from displayoutputs import displayoutputs

frame = []
frameLastChar = 0
frameChecksum = 0
frameIndex    = 0
timeout       = 0

DEBUG         = 0
FLAG          = 0x7E
CONTROLESCAPE = 0x7D

#--------------------------------------------------------------------------
class serialcomms():
    def __init__(self, port, baud):
        self.displayOutput = displayoutputs()
        self.port = serial.Serial(port, baud, timeout=0.2)
        self.msgs = {
        "jumpToBoot":{"frame":"o%c"%'^',"waitTime":5},
        "identity":{"frame":"o%c"%'?',"waitTime":10},
        "summaryRequest":{"frame":"o%c"%'r',"waitTime":10},
        "testComplete":{"frame":"o%c"%'C',"waitTime":10},
        "SP-illuminationOn":{"frame":"o%c"%'1',"waitTime":5},
        "SP-illuminationOff":{"frame":"o%c"%'2',"waitTime":5},
        "PC-relayState":{"frame":"o%c"%'1',"waitTime":5},
        "PC-sensorPowerState":{"frame":"o%c"%'2',"waitTime":5},
        "PC-pumpOff":{"frame":"o%c"%'0',"waitTime":5},
        "PC-pumpOn":{"frame":"o%c"%'1',"waitTime":5},
        "IC-relayEnable":{"frame":"o%c%c%c%c"%('+','+','+','1'),"waitTime":5},
        "IC-relayDisable":{"frame":"o%c%c%c%c"%('+','+','+','0'),"waitTime":5},
        "IC-calibrateStep0":{"frame":"o%c%c"%('C','0'),"waitTime":20},
        "IC-calibrateStep1":{"frame":"o%c%c"%('C','1'),"waitTime":10},
        "IC-calibrateStep2":{"frame":"o%c%c"%('C','2'),"waitTime":10},
        "IC-calibrateStep3":{"frame":"o%c%c"%('C','3'),"waitTime":10},
        "IC-calibrateStep4":{"frame":"o%c%c"%('C','4'),"waitTime":10},
        "IC-calibrateStep5":{"frame":"o%c%c"%('C','5'),"waitTime":10},
        "IC-calibrateStep6":{"frame":"o%c%c"%('C','6'),"waitTime":10},
        "IC-saveCalInfo":{"frame":"o%c"%'S',"waitTime":10},
        "TC-testMode":{"frame":"o%c"%'T',"waitTime":1},
        "TC-moveEast":{"frame":"o%c"%'1',"waitTime":20},
        "TC-moveWest":{"frame":"o%c"%'2',"waitTime":20},
        "TC-moveUp":{"frame":"o%c"%'3',"waitTime":20},
        "TC-moveDown":{"frame":"o%c"%'4',"waitTime":20},
        "TC-sensorPackComm":{"frame":"o%c"%'5',"waitTime":10},
        "TC-interconnectComm":{"frame":"o%c"%'6',"waitTime":10},
        "TC-railComm":{"frame":"o%c"%'7',"waitTime":10},
        "TC-moduleTemps":{"frame":"o%c"%'8',"waitTime":10},
        "RC-rtcComms":{"frame":"o%c"%'1',"waitTime":25},
        "RC-flashComms":{"frame":"o%c"%'2',"waitTime":60},
        "RC-rs485Comms":{"frame":"o%c"%'3',"waitTime":10},
        "RC-portVoltage":{"frame":"o%c"%'4',"waitTime":10},
        "RC-digitalInputs":{"frame":"o%c"%'5',"waitTime":10},
        "RC-permissionInput":{"frame":"o%c"%'6',"waitTime":10},
        "RC-estops":{"frame":"o%c"%'7',"waitTime":10},
        "RC-ethernet":{"frame":"o%c"%'8',"waitTime":20},
        "RC-ir":{"frame":"o%c"%'9',"waitTime":20},
        "RC-testPortPowerLEDs":{"frame":"o%c"%'A',"waitTime":20},
        "RC-testPortLEDsRed":{"frame":"o%c"%'B',"waitTime":10},
        "RC-testPortLEDsBlue":{"frame":"o%c"%'C',"waitTime":10},
        "RC-testPortLEDsGreen":{"frame":"o%c"%'D',"waitTime":10},
        "RC-test7SegmentDisplays":{"frame":"o%c"%'E',"waitTime":10},
        "RC-testAllIndicators":{"frame":"o%c"%'F',"waitTime":10},
        "RC-testsComplete":{"frame":"o%c"%'G',"waitTime":10},
        "RC-testSysLED":{"frame":"o%c"%'H',"waitTime":10},
        }
#--------------------------------------------------------------------------

#--------------------------------------------------------------------------
    def send(self, s):
        checksum = 0
        c = '\x7e'
        self.port.write(c)
        try:
            c = ord(c)
        except:
            pass
        if DEBUG:
            print "< 0x%02X %4d %c"%(c,c,c)
        for c in s:
            try:
                c = ord(c)
            except:
                pass
            if DEBUG:
                print "< 0x%02X %4d %c"%(c,c,c)
            if c == 0x7e or c == 0x7d:
                self.port.write("\x7d")
                checksum += 0x7d
                c = c ^ 0x20
            self.port.write(chr(c))
            checksum += c
        checksum = (256 - (0xff & checksum))&0xff
        self.port.write(chr(checksum))
        c = checksum
        if DEBUG:
            print "< 0x%02X %4d %c"%(c,c,c)
        c = "\x7e"
        self.port.write(c)
        try:
            c = ord(c)
        except:
            pass
        if DEBUG:
            print "< 0x%02X %4d %c"%(c,c,c)

#--------------------------------------------------------------------------
    def processchar(self, c):
        global frame, frameIndex, frameChecksum, frameLastChar, frameChecksum
        
        processedFrame = ""
        if c:
            c = ord(c)
            if DEBUG:
                print "> %2d 0x%02X %4d %c"%(frameIndex,c,c,c)
            if c == FLAG:
                if frameIndex > 1:
                    frameChecksum = 0
                    for i in range(frameIndex):
                        frameChecksum = 0xff&(frameChecksum + ord(frame[i]))
                    if frameChecksum == 0:
                        processedFrame = frame[:-1] # send everything except last char which is FLAG
                    else:
                        pass
                frame = ""
                frameLastChar = 0
                frameIndex    = 0
                return processedFrame
            if frameLastChar == CONTROLESCAPE:
                frame = frame[:-1] + chr(c ^ 0x20)
            else:
                frame += chr(c)
                frameIndex += 1
            frameLastChar = c
        else:
            return processedFrame

#--------------------------------------------------------------------------
    def processFrame(self, frame):
        reply = {}
        if frame[1] == 'P': # one-key reply
            for kv in frame[2:].split('\n'):
                kva = kv.strip().split(':')
                if len(kva[0]): # skip empties
                    reply[kva[0]] = kva[1]
        return reply

#--------------------------------------------------------------------------
    def sendFrame(self, frameType):
        frame = frameType["frame"]
        timeout = frameType["waitTime"]
        for count in range(3):
            self.send(frame)
            # wait for response
            receiveTimeout = time.time() + timeout
            while (time.time() < receiveTimeout):
                if self.port.inWaiting() :
                    c = self.processchar(self.port.read())
                    if (c != "" and c != None) :
                        return(self.processFrame(c))
        return 0

#--------------------------------------------------------------------------
