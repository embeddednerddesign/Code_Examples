#!/usr/bin/env python
import subprocess,time
from testreport import testreport
from displayoutputs import displayoutputs

DEBUG = 1

#--------------------------------------------------------------------------
class bootloadprocess():
    def __init__(self):
        self.displayOutput = displayoutputs()

#--------------------------------------------------------------------------
    def run(self,testerInfo,jtr,platform):
        if platform == "win" :
            # Test to see if we are working on the Rail Controller or Tracker Controller
            if ((testerInfo.prodFile.find("RailController") == -1) and (testerInfo.prodFile.find("TrackerController") == -1)):
            # We are not, so we are working with the Sun Sensor, Pump Controller, or Interconnect Board
                #--------------------------------------------------------------------------
                for count in range(3):
                    print self.displayOutput.harness["progBoot"],
                    if DEBUG:
                        r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -w " + testerInfo.bootFile + " -g -z [VCC]")
                    else:
                        r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -w " + testerInfo.bootFile + " -g -z [VCC]",stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Bootloader-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                        break
                    else:
                        if count > 1:
                            print self.displayOutput.harness["fail"]
                            jtr.setTestResultFieldValue("Bootloader-Loading", "result", "fail")
                            jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                            return jtr
                        raw_input("\nflasher failed, check programming cable, will retry, enter to try again ")
                        time.sleep(1)
                #--------------------------------------------------------------------------
                time.sleep(1)
                if testerInfo.getUuid:
                    print self.displayOutput.harness["progManuData"],
                    if DEBUG:
                        r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -w C:/manu/" + jtr.testReport["testresult"]["uuid"] + ".txt -g -z [VCC] -e ERASE_SEGMENT")
                    else:
                        r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -w C:/manu/" + jtr.testReport["testresult"]["uuid"] + ".txt -g -z [VCC] -e ERASE_SEGMENT",stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Manufacturing Info-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    else:
                        print self.displayOutput.harness["fail"]
                        jtr.setTestResultFieldValue("Manufacturing Info-Loading", "result", "fail")
                        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                        return jtr
                #--------------------------------------------------------------------------
                time.sleep(2)
                print self.displayOutput.harness["bootApp"],
                if DEBUG:
#                     r = subprocess.call("cd ../../../SharedLibraries/lib/msp430bootloaderprogrammer & python -m msp430.bsl5.bl485 --debug --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -eP " + testerInfo.prodFile + "", shell=True)
                    r = subprocess.call("cd ../../../SharedLibraries/lib/msp430bootloaderprogrammer & python -m msp430.bsl5.bl485 --debug --verbose --verbose --verbose --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -eP " + testerInfo.prodFile + "", shell=True)
                else:
                    r = subprocess.call("cd ../../../SharedLibraries/lib/msp430bootloaderprogrammer & python -m msp430.bsl5.bl485 --debug --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -eP " + testerInfo.prodFile + "", shell=True,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
#                 r = subprocess.call("cd ../../../SharedLibraries/lib/msp430bootloaderprogrammer & python -m msp430.bsl5.bl485 --debug --verbose --verbose --verbose --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -eP " + testerInfo.prodFile + "", shell=True,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
#                 r = subprocess.call("cd ../../../SharedLibraries/lib/msp430bootloaderremote & python -m msp430.bsl5.bl485 --debug --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -eP " + testerInfo.prodFile + "", shell=True)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    jtr.setTestResultFieldValue("Production Firmware-Loading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else :
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Production Firmware-Loading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
                #--------------------------------------------------------------------------
                time.sleep(1)
                print self.displayOutput.harness["readManuData"],
                if DEBUG:
                    r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -r [info.txt,INFO]")
                else:
                    r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -r [info.txt,INFO]",stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    f = open("info.txt", "r")
                    jtr.setFieldValue("infoBlockData", f.read())
                    jtr.setTestResultFieldValue("Manufacturing Info-Reading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else :
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Manufacturing Info-Reading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
                #--------------------------------------------------------------------------
                time.sleep(1)
                print self.displayOutput.harness["readT1"],
                if DEBUG:
                    r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -r [t1.txt,MAIN]")
                else:
                    r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -r [t1.txt,MAIN]",stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    f = open("t1.txt", "r")
                    jtr.setFieldValue("appData", f.read())
                    jtr.setTestResultFieldValue("Main Flash Memory-Reading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else :
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Main Flash Memory-Reading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
                #--------------------------------------------------------------------------
                time.sleep(2)
                subprocess.call("cd ../../../SharedLibraries/lib/msp430bootloaderprogrammer & python -m msp430.bsl5.bl485 --debug --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -j", shell=True,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                print self.displayOutput.harness["appJump"],
                print self.displayOutput.harness["pass"]
            #--------------------------------------------------------------------------
            else: # We are working with either the Tracker Controller or the Rail Controller
            #--------------------------------------------------------------------------
                # Test to see if we are working on the Rail Controller or Tracker Controller
                if (testerInfo.prodFile.find("RailController") == -1):
                # We are working with the Tracker Controller
                    print
                    print self.displayOutput.harness["progIrDABridge"],
                    if DEBUG:
                        r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -w " + testerInfo.bootFile + " -g -z [VCC]")
                    else:
                        r = subprocess.call("../../../SharedLibraries/bin/MSP430Flasher.exe -n MSP430" + testerInfo.processorVariant + " -w " + testerInfo.bootFile + " -g -z [VCC]",stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Tracker Controller IrDA Bridge-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    else:
                        print self.displayOutput.harness["fail"]
                        jtr.setTestResultFieldValue("Tracker Controller IrDA Bridge-Loading", "result", "fail")
                        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                        return jtr
                    print
                    print self.displayOutput.harness["progTCTestCode"],
                    r = subprocess.call("uniflashtc.bat",shell=True)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Tracker Controller Test Code-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    else:
                        print self.displayOutput.harness["fail"]
                        jtr.setTestResultFieldValue("Tracker Controller Test Code-Loading", "result", "fail")
                        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                        return jtr
                #--------------------------------------------------------------------------
                else: # We are working with the Rail Controller
                #--------------------------------------------------------------------------
                    print
                    print self.displayOutput.harness["progRailCont"],
                    if DEBUG:
                        r = subprocess.call("../../../SharedLibraries/bin/lm4flash " + testerInfo.prodFile)
                    else:
                        r = subprocess.call("../../../SharedLibraries/bin/lm4flash " + testerInfo.prodFile,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Rail Controller Test Code-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    else:
                        print self.displayOutput.harness["fail"]
                        jtr.setTestResultFieldValue("Rail Controller Test Code-Loading", "result", "fail")
                        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                        return jtr
                #--------------------------------------------------------------------------
                    print
                    print self.displayOutput.harness["progRailBase"],
                    if DEBUG:
                        r = subprocess.call("../../../SharedLibraries/bin/mspdebug/mspdebug " + testerInfo.mspdebugDriver + " --force-reset \"prog " + testerInfo.bootFile)
                    else:
                        r = subprocess.call("../../../SharedLibraries/bin/mspdebug/mspdebug " + testerInfo.mspdebugDriver + " --force-reset \"prog " + testerInfo.bootFile,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Rail Baseboard Test Code-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    else:
                        print self.displayOutput.harness["fail"]
                        jtr.setTestResultFieldValue("Rail Baseboard Test Code-Loading", "result", "fail")
                        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                        return jtr
        #--------------------------------------------------------------------------
        elif platform == "linux" :
            if (testerInfo.prodFile.find("RailController") == -1):
                #--------------------------------------------------------------------------
                for count in range(3):
                    print self.displayOutput.harness["progBoot"],
                    r = subprocess.call("mspdebug " + testerInfo.mspdebugDriver + " \"prog " + testerInfo.bootFile + "\"",shell=True)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Bootloader-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                        break
                    else:
                        if count > 1:
                            print self.displayOutput.harness["fail"]
                            jtr.setTestResultFieldValue("Bootloader-Loading", "result", "fail")
                            jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                            return jtr
                        raw_input("\nflasher failed, check programming cable, will retry, enter to try again ")
                        time.sleep(1)
                #--------------------------------------------------------------------------
                time.sleep(1)
                if testerInfo.getUuid:
                    print self.displayOutput.harness["progManuData"],
                    r = subprocess.call("mspdebug " + testerInfo.mspdebugDriver + " \"load " + jtr.testReport["testresult"]["uuid"] + ".txt\"",shell=True)
                    if r == 0:
                        print self.displayOutput.harness["pass"]
                        jtr.setTestResultFieldValue("Manufacturing Info-Loading", "result", "pass")
                        jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                    else:
                        print self.displayOutput.harness["fail"]
                        jtr.setTestResultFieldValue("Manufacturing Info-Loading", "result", "fail")
                        jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                        return jtr
                #--------------------------------------------------------------------------
                time.sleep(2)
                print self.displayOutput.harness["bootApp"],
                if (testerInfo.serCommPortBaud != 115200):
                    r = subprocess.call("cd ../../lib/msp430bootloaderprogrammer; python -m msp430.bsl5.bl485 --debug --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -eP " + testerInfo.prodFile + "", shell=True,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                else:
                    r = subprocess.call("cd ../../lib/msp430bootloaderprogrammer; python -m msp430.bsl5.bl485 --debug --port=" + testerInfo.serCommunicationPort + " -eP " + testerInfo.prodFile + "", shell=True,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    jtr.setTestResultFieldValue("Production Firmware-Loading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else :
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Production Firmware-Loading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
                #--------------------------------------------------------------------------
                time.sleep(1)
                print self.displayOutput.harness["readManuData"],
                r = subprocess.call("sudo mspdebug " + testerInfo.mspdebugDriver + " \"hexout 0x" + testerInfo.manuDataStart + " 0x" + testerInfo.manuDataLength + " manudata.txt\"",shell=True)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    jtr.setTestResultFieldValue("Manufacturing Info-Reading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else :
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Manufacturing Info-Reading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
                #--------------------------------------------------------------------------
                time.sleep(1)
                print self.displayOutput.harness["readT1"],
                r = subprocess.call("sudo mspdebug " + testerInfo.mspdebugDriver + " \"hexout 0x" + testerInfo.appCodeStart + " 0x" + testerInfo.appCodeLength + " t1.txt\"",shell=True)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    jtr.setTestResultFieldValue("Main Flash Memory-Reading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else :
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Main Flash Memory-Reading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
                #--------------------------------------------------------------------------
                time.sleep(2)
                subprocess.call("cd ../../lib/msp430bootloaderprogrammer; python -m msp430.bsl5.bl485 --debug --port=" + testerInfo.serCommunicationPort + " -B " + str(testerInfo.serCommPortBaud) + " -j", shell=True,stdin=subprocess.PIPE,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
                print self.displayOutput.harness["appJump"],
                print self.displayOutput.harness["pass"]
            else:
            #--------------------------------------------------------------------------
                print
                print self.displayOutput.harness["progRailCont"],
                r = subprocess.call("../../bin/lm4flash " + testerInfo.prodFile,shell=True)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    jtr.setTestResultFieldValue("Rail Controller Test Code-Loading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else:
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Rail Controller Test Code-Loading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
            #--------------------------------------------------------------------------
                print
                print self.displayOutput.harness["progRailBase"],
                r = subprocess.call("../../bin/mspdebug/mspdebug " + testerInfo.mspdebugDriver + " --force-reset \"prog " + testerInfo.bootFile + "\"",shell=True)
                if r == 0:
                    print self.displayOutput.harness["pass"]
                    jtr.setTestResultFieldValue("Rail Baseboard Test Code-Loading", "result", "pass")
                    jtr.setFieldValue("pass", (jtr.getFieldValue("pass") + 1))
                else:
                    print self.displayOutput.harness["fail"]
                    jtr.setTestResultFieldValue("Rail Baseboard Test Code-Loading", "result", "fail")
                    jtr.setFieldValue("fail", (jtr.getFieldValue("fail") + 1))
                    return jtr
        #--------------------------------------------------------------------------
        print
        print self.displayOutput.harness["programming"],
        print self.displayOutput.harness["pass"]
        return jtr
#--------------------------------------------------------------------------
