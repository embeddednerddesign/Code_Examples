#!/usr/bin/env python

import sys
sys.path.append("/home/ubuntu/tools")
import random, time, re, uuid, socket, stomp, urllib2, traceback, os, math
from parse import parse
from generatorinfo import generatorinfo
from time import gmtime, strftime

import requests,json
from requests.auth import HTTPDigestAuth
import base64

from stompSend import stompSend

#STOMP config
BROKERHOST  = "blprod.us"
BROKERPORT	= 8123
DESTINATION	= "/queue/event"
stompLogLocation = "/home/ubuntu/bin/stompLog.txt"

#Debug Config
PrintUrls = 1
PrintData = 1
PrintHttpResp = 1
PrintError = 5

#Script config
PostStompMsg = 1
ScriptSleep = 5

def parseEgaugeInResponse(response):

    resp = ""
    startIndex = 0
    # Grid Voltage - Phase 1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "voltage.phase1:%s\n"%response[startIndex:endIndex]
    # Grid Voltage - Phase 2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "voltage.phase2:%s\n"%response[startIndex:endIndex]
    # Grid Voltage - Phase 3
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "voltage.phase3:%s\n"%response[startIndex:endIndex]
    # Grid Current - Phase 1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase1:%s\n"%response[startIndex:endIndex]
    # Grid Current - Phase 2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase2:%s\n"%response[startIndex:endIndex]
    # Grid Current - Phase 3
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase3:%s\n"%response[startIndex:endIndex]
    # Power - Phase 1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "power.phase1:%s\n"%response[startIndex:endIndex]
    powerPhase1 = int(response[startIndex:endIndex])
    # Power - Phase 2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "power.phase2:%s\n"%response[startIndex:endIndex]
    powerPhase2 = int(response[startIndex:endIndex])
    # Power - Phase 3
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "power.phase3:%s\n"%response[startIndex:endIndex]
    powerPhase3 = int(response[startIndex:endIndex])
    # Power - Total
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "power.total:%s\n"%response[startIndex:endIndex]

    totalPower = powerPhase1 + powerPhase2 + powerPhase3 
    resp = "consumed.power.total:%s\n"%totalPower

    return resp

def parseEgaugeOutResponse(response):

    resp = ""
    startIndex = 0
    # Solar Voltage - Phase 1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "voltage.phase1:%s\n"%response[startIndex:endIndex]
    # Solar Voltage - Phase 2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "voltage.phase2:%s\n"%response[startIndex:endIndex]
    # Solar Voltage - Phase 3
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "voltage.phase3:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 1-1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase1_1:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 1-2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase1_2:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 1-2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase1_3:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 2-1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase2_1:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 2-2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase2_2:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 2-2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase2_3:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 3-1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase3_1:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 3-2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase3_2:%s\n"%response[startIndex:endIndex]
    # Solar Current - Phase 3-2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "current.phase3_3:%s\n"%response[startIndex:endIndex]
    # Solar Power - Phase 1
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "power.phase1:%s\n"%response[startIndex:endIndex]
    powerPhase1 = int(response[startIndex:endIndex])
    # Solar Power - Phase 2
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "power.phase2:%s\n"%response[startIndex:endIndex]
    powerPhase2 = int(response[startIndex:endIndex])
    # Solar Power - Phase 3
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "power.phase3:%s\n"%response[startIndex:endIndex]
    powerPhase3 = int(response[startIndex:endIndex])

    totalPower = powerPhase1 + powerPhase2 + powerPhase3 
    resp = "produced.power.total:%s\n"%totalPower

    return resp

def parseEgaugeResponse(response):

    resp = ""
    startIndex = 0
    # Power Consumed
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "consumed.power.total:%s\n"%response[startIndex:endIndex]
    # Power Produced
    startIndex = response.find('<i>',startIndex) + 3
    endIndex = response.find('</i>',startIndex)
    resp += "produced.power.total:%s\n"%response[startIndex:endIndex]
    
    return resp

siteName = sys.argv[1]
myuuid = ''.join(re.findall('..', '%012x' % uuid.getnode())).upper()
hostname = socket.gethostname()
stompSender = stompSend()

if siteName == "UoN":
    username = "siteserver"
    password = "kuakini-755851-ss"
    # 200A eGauge Meter - Incoming to Site
    path = socket.gethostname().replace("siteserver","egauge")
    url = "http://10.10.100.248/cgi-bin/egauge?inst"
    r = requests.get(url, auth=HTTPDigestAuth(username, password))
    egaugeData = r.text
    body = parseEgaugeInResponse(egaugeData)
    stompSender.sendStompMessage(BROKERHOST,BROKERPORT,body,DESTINATION,{"path":path,"type":"report","uuid":myuuid})
    
    # 750A eGauge Meter - Outgoing from Site
    path = socket.gethostname().replace("siteserver","egauge")
    url = "http://10.10.100.249/cgi-bin/egauge?inst"
    r = requests.get(url, auth=HTTPDigestAuth(username, password))
    egaugeData = r.text
    body = parseEgaugeOutResponse(egaugeData)
    stompSender.sendStompMessage(BROKERHOST,BROKERPORT,body,DESTINATION,{"path":path,"type":"report","uuid":myuuid})
elif siteName == "MWWTP":
    username = "owner"
    password = "default"
    path = socket.gethostname().replace("siteserver","egauge")
    url = "http://10.10.100.248/cgi-bin/egauge?inst"
    r = requests.get(url, auth=HTTPDigestAuth(username, password))
    egaugeData = r.text
    body = parseEgaugeResponse(egaugeData)
    stompSender.sendStompMessage(BROKERHOST,BROKERPORT,body,DESTINATION,{"path":path,"type":"report","uuid":myuuid})
else:
    print "Unknown site - Please check SetEnvVars file for SITE_NAME variable"

time.sleep(ScriptSleep) # waiting for thread compare
