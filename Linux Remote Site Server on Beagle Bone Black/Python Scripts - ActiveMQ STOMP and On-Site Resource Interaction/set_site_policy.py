#!/usr/bin/env python

import sys, urllib2, os, time, re, socket, uuid, stomp, json
from generatorinfo import generatorinfo
from stomplistener import stomplistener

# #===============================================================================
# # Network Elements - Init
# #===============================================================================
if len(sys.argv) < 2:
    print "Requires 1 argument: PolicyFile"
    print "Example:"
    print "setTrackerConfigData.py SitePolicy.txt"
    print "will extract the policy data from the file and send it to the generators on the site."
    sys.exit(1)
 
siteName = sys.argv[1]
GenInfo = generatorinfo(sys.argv[2], sys.argv[3], sys.argv[4])
windAlarm = sys.argv[5]
SitePoliciesFileLocation = sys.argv[6]

# ******************************************************************
# Load the policies from file
# ******************************************************************
print "loading site policies"
# read the policies file, which is a list of dicts
SitePolicies = json.loads(open(SitePoliciesFileLocation,"r").read())
# first item is Global Policies
GlobalPolicies = SitePolicies[0]
try:
    # second item is Generator-specific Policies
    GeneratorPolicies = SitePolicies[1]
except:
    # sometimes we may not need to have Generator-specific policies, and that's ok
    pass

# ******************************************************************
# Extract the values and build the URLs for the Generator-Specific Policies
# ******************************************************************
# iterate through the list of sets of generator-specific policies
# also keep a list of all the generator numbers in the Generator-Specific policies
GeneratorsSpecified = []
for generatorNumber in GeneratorPolicies["GeneratorPolicies"]:
    for key in generatorNumber:
        genSpecificUrlArgs = ""
        # the Generator number IP address is the same as the key, so make it an int and save it
        urlGenIP = int(key)
        GeneratorsSpecified.append(int(key))
        for value in generatorNumber[key]:
            genSpecificUrlArgs += "SwFlag:%s&"%value
        # finish off the command
        genSpecificUrlArgs += "paramNull:empty&&&"
        # base url for the web command, with the key used as the Generator number IP address
        url =  "http://10.10.100.%d/saveGenPage-id=123&"%(urlGenIP+GenInfo.GeneratorNumberToIPConversion)
        url += genSpecificUrlArgs
        # try sending the command to save the new info
        try:
            print "sending -> "
            print url
            gendata = urllib2.urlopen(url).read()
        except urllib2.URLError:
            print "not found"

# ******************************************************************
# Extract the values and build the URL for the Global Policies
# ******************************************************************
globalUrlArgs = ""
# iterate through the list of global policy values 
for value in GlobalPolicies["GlobalPolicies"]:
    # add 1 argument per item
    # global wind alarm is handled by a passed in variable, so make sure it hasn't been included here as well
    if value.find("ALARM_WIND") == -1:
        globalUrlArgs += "SwFlag:%s&"%value
# now add the global wind alarm based on the passed in variable
globalUrlArgs += "SwFlag:ALARM_WIND,%s&"%windAlarm
# finish off the command
globalUrlArgs += "paramNull:empty&&&"
# ******************************************************************
# Update all the generators with the global policies
# ******************************************************************
for g in range(GenInfo.GeneratorRangeStart,GenInfo.GeneratorRangeEnd+1):
    if not g in GeneratorsSpecified:
        # this is the base for the web command
        url =  "http://10.10.100.%d/saveGenPage-id=123&"%(g+GenInfo.GeneratorNumberToIPConversion)
        # add the policy arguments
        url += globalUrlArgs
        # try sending the command to save the new info
        try:
            print "sending -> "
            print url
            gendata = urllib2.urlopen(url).read()
        except urllib2.URLError:
            print "not found"
      
print
print "Done"
print
time.sleep(0.5)
            



