#!/usr/bin/env python

import sys, time, socket, os, re, uuid, json, urllib2, stomp, string
from time import gmtime, strftime

import smtplib
import mimetypes
from email.mime.multipart import MIMEMultipart
from email import encoders
from email.message import Message
from email.mime.audio import MIMEAudio
from email.mime.base import MIMEBase
from email.mime.image import MIMEImage
from email.mime.text import MIMEText

EmailFrom = "situationreporter@gmail.com"
EmailToListSituations = ['embeddednerddesign@gmail.com','nvyssotski@brightleafpower.com','choover@brightleafpower.com','ssims@brightleafpower.com','sduckworth@brightleafpower.com']
EmailToListReport = ['embeddednerddesign@gmail.com','nvyssotski@brightleafpower.com']
EmailSubject = ""
EmailBody = ""
EmailUsername = ""
EmailPassword = ""
BROKERHOST  = "blprod.us"
BROKERPORT  = 8123

DESTINATION = "/queue/situation"
myuuid = ''.join(re.findall('..', '%012x' % uuid.getnode())).upper()
# myuuid = sys.argv[0]

keyMonitoringFlagsFileLocation = "/home/ubuntu/bin/keyMonitoringFlags.json"
expressionsFileLocation = "/home/ubuntu/bin/expressions.json"
situationReportFileLocation = "/home/ubuntu/bin/situation_report.txt"
situationReportBackupFileLocation = "/home/ubuntu/bin/situation_report_bak.txt"

punc = string.punctuation.replace('.','') 

class stompListener(object):
    def on_error(self, headers, message):
        print 'received an error %s' % message

    def on_message(self, headers, message):
        print 'received an message %s' % message

    def on_connected(self, headers, message):
        #print 'stomp connected %s' % message
        pass

    def on_disconnected(self):
        os.kill(os.getpid(),1)


def transform(e):
    # create a list to hold the raw words that make up the keys in the expression
    rwords = []
    # as long as there are still markers in the string we're not done
    while e.find("#") != -1:
        # get the index of the next start marker
        startindex = e.find("#")
        # get the index of the next end marker
        endindex = e.find("$")
        # extract the key inside the markers and add it to the list
        rwords.append(e[startindex+1:endindex])
        # replace the start and end markers so that evaluating the expression will work
        e = e.replace("#","v['",1)
        e = e.replace("$","'][1]",1)
    return (e,rwords)
    
#     rwords = []
#     s = list(e)
#     words = ''.join([o for o in s if not o in punc]).split()
#     donewords = {}
#     for word in words:
#         if donewords.has_key(word):
#             continue 
#         if word.count("us."):
#             if word[-16:] == "version.software":
#                 e = e.replace(word,"v['%s'][1][:3]"%word)
#             else:
#                 e = e.replace(word,"v['%s'][1]"%word)
#             rwords.append(word)
#             donewords[word] = ""
#     return (e,rwords)

def prepEmail(body):
    startIndex = body.find("SITUATION:    ")
    endIndex = body.find(" @")
    subjectIssue = body[startIndex+14:endIndex]
    startIndex = body.find("TEST FAILED:\n")
    endIndex = body.find("DATA:")
    subjectLocation = body[startIndex+12:endIndex]
    subjectLocation = subjectLocation.replace("\n","")
    startIndex = subjectLocation.find("u")
    endIndex = subjectLocation.find("generator.g")
    subjectLocation = subjectLocation[startIndex:endIndex+13]
    subject = 'Alert:' + subjectLocation + ' ' + subjectIssue 
    
    return subject,body


def sendEmail(emailFrom,emailTo,subject,body,fileToSend="",username="",password=""):
    msg = MIMEMultipart()
    msg["From"] = emailFrom
    msg["To"] = ', '.join(emailTo)
    msg["Subject"] = subject
    body = MIMEText(body, 'plain')
    msg.attach(body)

    if fileToSend != "":
        ctype, encoding = mimetypes.guess_type(fileToSend)
        if ctype is None or encoding is not None:
            ctype = "application/octet-stream"
        
        maintype, subtype = ctype.split("/", 1)
        
        if maintype == "text":
            fp = open(fileToSend)
            # Note: we should handle calculating the charset
            attachment = MIMEText(fp.read(), _subtype=subtype)
            fp.close()
    #     elif maintype == "image":
    #         fp = open(fileToSend, "rb")
    #         attachment = MIMEImage(fp.read(), _subtype=subtype)
    #         fp.close()
    #     elif maintype == "audio":
    #         fp = open(fileToSend, "rb")
    #         attachment = MIMEAudio(fp.read(), _subtype=subtype)
    #         fp.close()
    #     else:
    #         fp = open(fileToSend, "rb")
    #         attachment = MIMEBase(maintype, subtype)
    #         attachment.set_payload(fp.read())
    #         fp.close()
    #         encoders.encode_base64(attachment)
        fileToSendName = "situation_report.txt"
        attachment.add_header("Content-Disposition", "attachment", filename=fileToSendName)
        msg.attach(attachment)
    
    server = smtplib.SMTP("smtp.gmail.com:587")
    server.starttls()
    server.login(username,password)
    server.sendmail(emailFrom, emailTo, msg.as_string())
    server.quit()



print "loading key monitoring flags"
# if we have a key monitoring flags file, open it.  If not, create one and open it 
try:
    keyMonitoringFlagsFile = open(keyMonitoringFlagsFileLocation,"r") 
    keyFlags = json.load(keyMonitoringFlagsFile)
except:
    keyMonitoringFlagsFile = open(keyMonitoringFlagsFileLocation,"w") 
    keyFlags = {}
keyMonitoringFlagsFile.close()

# if we have situation report file, open it and append to it. If not, create a new one and open it
situationReportFile = open(situationReportFileLocation,"a+")
    
print "loading all values"

expressions = json.load(open(expressionsFileLocation,"r"))['expressions']
    
j = urllib2.urlopen('http://bldash.us/services/q?type=values&path=^us.*').read()
v = json.loads(j)

broker = stomp.Connection(host_and_ports=[(BROKERHOST,BROKERPORT)])
l = stompListener()
broker.add_listener(l)
broker.start()
broker.connect()

year  = strftime("%Y", gmtime())
month = strftime("%m", gmtime())
day   = strftime("%d", gmtime())
hour  = strftime("%H", gmtime())
min   = strftime("%M", gmtime())
sec   = strftime("%S", gmtime())
timestamp = year + "-" + month + "-" + day + " " + hour + ":" + min + ":" + sec + " GMT"

# this will act as border between entries in the report
reportBody = "*******************************************************************\n"
reportBody += timestamp + "\n"
reportBody += "*******************************************************************\n\n"

#************************************************************************************************
try:
    # if we have a backup situation report file, open it and read it. If not, create one and consider it updated
    situationReportFileBackup = open(situationReportBackupFileLocation,"r")
    situationReportOld = situationReportFileBackup.read()
    situationReportFileBackup.close()
    # have to add in the mandatory header before we compare
    situationReportOld += reportBody
except:
    situationReportOld = ""
#************************************************************************************************

for e in expressions:
    expression = e['expression']
    (e['expression'],e['words']) = transform(e['expression'])
    # build the text that will be used in the reports
    cleanExpression = expression.replace("#","")
    cleanExpression = cleanExpression.replace("$","")
    body = "SITUATION:    " + e['desc'] + " @ " + timestamp + "\n"
    body += "TEST FAILED:\n" + cleanExpression + "\n"
    body += "DATA:\n"

    invalidData = False
    for word in e['words']:
        body += "%s:"%word
        body += str(v[word][1])+"\n"
        if v[word][1] == "N/A":
            invalidData = True
    try:
        if eval(e['expression']):
            print 'true    ', expression
            # check if this key is already in the key monitoring flags file
            if expression not in keyFlags:
                # add the new key
                keyFlags[expression] = 1
                # the key isn't in there already, so send the report
                print "report sent"
                reportBody += body + "\n\n"
#********************************************************************************************
# this is being commented out because it is adding entries to the database that I don't think we even want.  At least, not right now
#********************************************************************************************
#                 path = e['path']
#                 headers={"path":path,"uuid":myuuid,"type":"report"}
#                 broker.send(body,destination=DESTINATION,headers=headers)
#********************************************************************************************
                EmailSubject,EmailBody = prepEmail(body)
                sendEmail(EmailFrom,EmailToListSituations,EmailSubject,EmailBody,"",EmailUsername,EmailPassword)
    
    #             body = "log:km - " + e['desc'] + "%0a" + body.replace("\n","%0a")
#********************************************************************************************
# this is being commented out because it is adding entries to the database that I don't think we even want.  At least, not right now
#********************************************************************************************
#                 body = "log:km - " + e['desc'] + "\n" + body
#                 headers={"path":path,"uuid":myuuid,"type":"situation"}
#                 broker.send(body,destination='/queue/event',headers=headers)
#********************************************************************************************
            else:
                print "report not sent"
        else:
            print 'false   ', e['expression']
            # check if this key is already in the key monitoring flags file
            if expression in keyFlags:
                if not invalidData:
                    # the key is in here, so remove it now that the test came back False
                    keyFlags.pop(expression,False)
                    reportBody += "REMOVED:\n"
                    reportBody += body + "\n\n"
    except:
        print "*******************"
        print "Error evaluating expression:"
        print e['expression']
        print "*******************"

# before we leave, update the key monitoring flags file
keyMonitoringFlagsFile = open(keyMonitoringFlagsFileLocation,"w") 
json.dump(keyFlags, keyMonitoringFlagsFile)
keyMonitoringFlagsFile.close()

# before we leave, update the situation report
situationReportFile.write(reportBody)
situationReportFile.close()

# read back out the updated report for comparison and determining if changes were made
situationReportFile = open(situationReportFileLocation,"r")
situationReportNew = situationReportFile.read()

# only send out the report if it has been updated
if situationReportNew != situationReportOld:
    # once per hour, send out an extra email with the situation report attached
    if int(hour) == 0 and int(min) < 2:
        print "sending out the report"
        sendEmail(EmailFrom,EmailToListReport,"Situation Report","",situationReportFileLocation,EmailUsername,EmailPassword)
        # update the backup file to match
        situationReportFileBackup = open(situationReportBackupFileLocation,"w")
        situationReportFileBackup.write(situationReportNew)

situationReportFileBackup.close()
situationReportFile.close()

time.sleep(1) # waiting for thread compare


    
