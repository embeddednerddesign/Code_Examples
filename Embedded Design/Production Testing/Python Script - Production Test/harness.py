#!/usr/bin/env python

import sys, time, os, subprocess
sys.path.append("../../../SharedLibraries/lib")
from serialcomms import serialcomms
from testerinfo import testerinfo
from testreport import testreport
from displayoutputs import displayoutputs
from harnesslib import harnesslib
try:
    import win32api
    import win32print
except:
    pass

#===============================================================================
# Constants used for initialization of the tester and in the bootloading process
#===============================================================================
#===============================================================================
# Start addresses and memory length for the memory sections of the microprocessor
#===============================================================================
MANUFACTURING_DATA_START_ADDRESS = "1880" 
MANUFACTURING_DATA_LENGTH = "0200"
APP_CODE_START = "4400"
APP_CODE_LENGTH = "2000" 
#===============================================================================
# Bootloading specifics of the microprocessor and locations of code files
#===============================================================================
MICROPROCESSOR_VARIANT = "g2553"
MSPDEBUG_DRIVER = "rf2500"
BOOTLOADER_FILE_LOCATION = "../../../TestFirmware/IrdaBridge.txt"
PRODUCTION_FILE_LOCATION = "TrackerController"
#===============================================================================
# Serial port baud rates for the Launchpad board and the target board
#===============================================================================
PROGRAMMER_PORT_BAUD_RATE = "N/A"
COMMUNICATION_PORT_BAUD_RATE = 115200

#===============================================================================
# test data - tester specific
#===============================================================================
class testData(object):
    def __init__(self):
        self.testData = {
        "deviceid":"",
        "moveEast":"",
        "moveWest":"",
        "moveUp":"",
        "moveDown":"",
        "sensorPackComm":"",
        "interconnectComm":"",
        "railComm":"",
        "moduleTemps":"",
        "manuData":"",
        "hardwareVersion":"",
        "manuDate":"",
        "partNumber":"",
        "targetUUID":""
        }

#===============================================================================
# Initialization
#===============================================================================
running = 1
success = 0
DEBUG = 0

# chose an implementation, depending on os ------------------------------------- 
if os.name == 'nt': #sys.platform == 'win32':
    platform = "win"
elif os.name == 'posix':
    platform = "linux"
else:
    raise ImportError("Sorry: no implementation for your platform ('%s') available" % (os.name,))

# initialize test objects ------------------------------------------------------
print
testerInfo = testerinfo(MANUFACTURING_DATA_START_ADDRESS,MANUFACTURING_DATA_LENGTH,APP_CODE_START,APP_CODE_LENGTH,MICROPROCESSOR_VARIANT,MSPDEBUG_DRIVER,PROGRAMMER_PORT_BAUD_RATE,COMMUNICATION_PORT_BAUD_RATE,BOOTLOADER_FILE_LOCATION,PRODUCTION_FILE_LOCATION)
testerInfo.initialize()
displayOutput = displayoutputs()
harnessLib = harnesslib(platform)
print 
#===============================================================================
# Start of Test
#===============================================================================
while running :
    td = testData()
    jtr = testreport("Tracker Controller")
    md,jtr = harnessLib.testInit(testerInfo, jtr)
     
    if (jtr.getFieldValue("summary") == "failed"):   # if some part of the init failed, stop here
        break
       
    jtr.setFieldValue("hardwareVersion", testerInfo.versioninfo)

    # setup the serial port --------------------------------------------------------
    try:
        serTrackerController = serialcomms(testerInfo.serCommunicationPort, testerInfo.serCommPortBaud)
        jtr.setTestResultFieldValue("consoleComm", "result", "pass")
        jtr.setTestResultFieldValue("consoleComm", "note", "Serial Setup Good")
        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
    except:
        print displayOutput.harness["serialInitFail"]
        jtr.setTestResultFieldValue("consoleComm", "result", "fail")
        jtr.setTestResultFieldValue("consoleComm", "note", "Serial Setup Bad")
        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
        jtr.setFieldValue("summary", "failed")
        jtr.commError()
        jtr.generateJSON()
        break
#===============================================================================
# Enter TC Test Mode
#===============================================================================
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-testMode"])
#===============================================================================
# Move East 
#===============================================================================
    print
    print displayOutput.tc["moveEast"],
    # send command - wait for response -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-moveEast"])

    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            td.testData["moveEast"] = response["moveEastResult"]
            jtr.setTestResultFieldValue("Move East", "note", "Move East=%s"%td.testData["moveEast"])
            try:
                if td.testData["moveEast"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Move East", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Move East", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
#===============================================================================
# Move West
#===============================================================================
    print
    print displayOutput.tc["moveWest"],
    # send command - wait for response -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-moveWest"])
        
    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            (td.testData["moveWest"]) = response["moveWestResult"]
            #--------------------------------------------------------------------------
            jtr.setTestResultFieldValue("Move West", "note", "Move West=%s"%td.testData["moveWest"]) 
            #--------------------------------------------------------------------------
            try:
                if td.testData["moveWest"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Move West", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Move West", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
#===============================================================================
# Move Up    
#===============================================================================
    print
    print displayOutput.tc["moveUp"],
    # send command - wait for response -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-moveUp"])
        
    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            (td.testData["moveUp"]) = response["moveUpResult"]
            #--------------------------------------------------------------------------
            jtr.setTestResultFieldValue("Move Up", "note", "Move Up=%s"%td.testData["moveUp"]) 
            #--------------------------------------------------------------------------
            try:
                if td.testData["moveUp"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Move Up", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Move Up", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
#===============================================================================
# Move Down    
#===============================================================================
    print
    print displayOutput.tc["moveDown"],
    # send command - wait for response -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-moveDown"])
        
    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            (td.testData["moveDown"]) = response["moveDownResult"]
            #--------------------------------------------------------------------------
            jtr.setTestResultFieldValue("Move Down", "note", "Move Down=%s"%td.testData["moveDown"]) 
            #--------------------------------------------------------------------------
            try:
                if td.testData["moveDown"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Move Down", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Move Down", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
#===============================================================================
# Sensor Pack Comms
#===============================================================================
    print
    print displayOutput.tc["sensorPackComm"],
    # send command - wait for response -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-sensorPackComm"])
        
    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            (td.testData["sensorPackComm"]) = response["sensorPackCommResult"]
            #--------------------------------------------------------------------------
            jtr.setTestResultFieldValue("Sensor Pack Comm", "note", "Sensor Pack Comm=%s"%td.testData["sensorPackComm"]) 
            #--------------------------------------------------------------------------
            try:
                if td.testData["sensorPackComm"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Sensor Pack Comm", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Sensor Pack Comm", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
    time.sleep(2)
#===============================================================================
# Interconnect Comms
#===============================================================================
    print
    print displayOutput.tc["interconnectComm"],
    # if response is valid, process it -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-interconnectComm"])
        
    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            (td.testData["interconnectComm"]) = response["interconnectCommResult"]
            #--------------------------------------------------------------------------
            jtr.setTestResultFieldValue("Interconnect Comm", "note", "Interconnect Comm=%s"%td.testData["interconnectComm"]) 
            #--------------------------------------------------------------------------
            try:
                if td.testData["interconnectComm"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Interconnect Comm", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Interconnect Comm", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
    time.sleep(2)
#===============================================================================
# Rail Controller Comms
#===============================================================================
    print
    print displayOutput.tc["railComm"],
    # if response is valid, process it -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-railComm"])
        
    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            (td.testData["railComm"]) = response["railCommResult"]
            #--------------------------------------------------------------------------
            jtr.setTestResultFieldValue("Rail Comm", "note", "Rail Comm=%s"%td.testData["railComm"]) 
            #--------------------------------------------------------------------------
            try:
                if td.testData["railComm"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Rail Comm", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Rail Comm", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
    time.sleep(2)
#===============================================================================
# Module Temperatures
#===============================================================================
    print
    print displayOutput.tc["moduleTemps"],
    # if response is valid, process it -----------------------------------------
    response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-moduleTemps"])
        
    # if response is valid, process it -----------------------------------------
    if response != 0:
        try:
            (td.testData["moduleTemps"]) = response["moduleTempsResult"]
            #--------------------------------------------------------------------------
            jtr.setTestResultFieldValue("Module Temps", "note", "Module Temps=%s"%td.testData["moduleTemps"]) 
            #--------------------------------------------------------------------------
            try:
                if td.testData["moduleTemps"] != "pass":
                    # test failed ----------------------------------------------
                    print displayOutput.harness["fail"]
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    jtr.setTestResultFieldValue("Module Temps", "result", "fail")
                else:
                    # test passed ----------------------------------------------
                    print displayOutput.harness["pass"]
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    jtr.setTestResultFieldValue("Module Temps", "result", "pass")
            except:
                print displayOutput.serialComms["frameProcessError"]
                jtr.commError()
            #--------------------------------------------------------------------------
        except:
            print displayOutput.serialComms["frameProcessError"]
            jtr.commError()
    else:
        print displayOutput.harness["noRespTrackerController"]
        jtr.commError()
#===============================================================================
# Exit TC Test Mode
#===============================================================================
#     response  = serTrackerController.sendFrame(serTrackerController.msgs["TC-testMode"])

    print
    print displayOutput.harness["progTCBootloader"],
    r = subprocess.call("uniflashtcboot.bat",shell=True)
    if r == 0:
        print displayOutput.harness["pass"]
        jtr.setTestResultFieldValue("Tracker Controller Bootloader-Loading", "result", "pass")
        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
    else:
        print displayOutput.harness["fail"]
        jtr.setTestResultFieldValue("Tracker Controller Bootloader-Loading", "result", "fail")
        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))


#===============================================================================
# Final Cleanup
#===============================================================================
    print
    
    response = harnessLib.testComplete(testerInfo, jtr, md)
     
    print "\n\n"
     
    if testerInfo.getUuid:
        if response.status != 200:
            print displayOutput.harness["resultPostFail"]
            break

    serTrackerController.port.close()
    #--------------------------------------------------------------------------
print displayOutput.harness["starBorder"]
print displayOutput.harness["testTerminated"]
print displayOutput.harness["starBorder"]
#--------------------------------------------------------------------------

















#-----------------------------------------------------------------------------------------------------------------------------------
# This is how it was originally done. Basically, a version of railsimulator.py that just sends out the characters that are typed by the user.
#-----------------------------------------------------------------------------------------------------------------------------------
# #--------------------------------------------------------------------------
# #--------------------------------------------------------------------------
# class _Getch:
#     """Gets a single character from standard input.  Does not echo to the
# screen."""
#     def __init__(self):
#         try:
#             self.impl = _GetchWindows()
#         except ImportError:
#             self.impl = _GetchUnix()
# 
#     def __call__(self): return self.impl()
# #--------------------------------------------------------------------------
# 
# #--------------------------------------------------------------------------
# #--------------------------------------------------------------------------
# class _GetchWindows:
#     def __init__(self):
#         import msvcrt
# 
#     def __call__(self):
#         import msvcrt
#         def getch_nonblocking():
#             if msvcrt.kbhit():
#                 return msvcrt.getch()
#             return None
#         return getch_nonblocking()
# #--------------------------------------------------------------------------
# 
# #--------------------------------------------------------------------------
# # Initialization
# #--------------------------------------------------------------------------
# # chose an implementation, depending on os
# if os.name == 'nt': #sys.platform == 'win32':
#     platform = "win"
# elif os.name == 'posix':
#     platform = "linux"
# else:
#     raise ImportError("Sorry: no implementation for your platform ('%s') available" % (os.name,))
# 
# print
# testerInfo = testerinfo(MANUFACTURING_DATA_START_ADDRESS,MANUFACTURING_DATA_LENGTH,APP_CODE_START,APP_CODE_LENGTH,MICROPROCESSOR_VARIANT,MSPDEBUG_DRIVER,PROGRAMMER_PORT_BAUD_RATE,COMMUNICATION_PORT_BAUD_RATE,BOOTLOADER_FILE_LOCATION,PRODUCTION_FILE_LOCATION)
# testerInfo.initialize()
# displayOutput = displayOutputs()
# harnessLib = harnesslib(platform)
# print 
# #--------------------------------------------------------------------------
# while running :
#     # setup the serial port
#     try:
#         serTrackerController = serialcomms(testerInfo.serCommunicationPort, testerInfo.serCommPortBaud)
#     except:
#         print displayOutput.harness["serialInitFail"]
#         break
# #--------------------------------------------------------------------------
# # Start of Test
# #--------------------------------------------------------------------------
#     while running:
#         prompt = 1
#         getch = _Getch()
#         while running == 1:
#             if serTrackerController.port.inWaiting():
#                 d = serTrackerController.processchar(serTrackerController.port.read())
#                 if (d != "" and d != None) :
#                     response = serTrackerController.processFrame(d)
#                     print response
#                     prompt = 1
#             c = getch()
#             if c:
#                 print c
#                 if c == "q" or c == "Q":
#                     running = False
#                     break
#                 FRAME = "o%c"%c
#                 serTrackerController.send(FRAME)
#                 prompt = 1
#             if prompt:
#                 sys.stdout.write("> ")
#                 sys.stdout.flush()
#                 prompt = 0
#         '''
#             try:
#                 stomp = s.recv(1024)
#                 if len(stomp):
#                     print stomp.split("\n\n")[0]
#                     print
#                     m  = "f\x01\x00\x01K\06"+stomp  # stompset
#                     send(m)
#                     timeout = time.time() + timeouttime
#             except socket.timeout:
#                 pass
#         '''
# #--------------------------------------------------------------------------
#     print "\n\n"
#      
#     # clean up
#     serTrackerController.port.close()
#     #--------------------------------------------------------------------------
# print displayOutput.harness["starBorder"]
# print displayOutput.harness["testTerminated"]
# print displayOutput.harness["starBorder"]
# #--------------------------------------------------------------------------
