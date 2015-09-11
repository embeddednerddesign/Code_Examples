#!/usr/bin/env python
import pymongo, redis, sys, time
import pprint, json, collections

db = pymongo.MongoClient().bldata.fielddata

KeyRegx = ".*"
try:
    KeyRegx = sys.argv[1]
except:
    pass
paths = []
for doc in db.find({"_id": {"$regex": KeyRegx}}, {"_id": 1}):
    paths.append(doc['_id'])

pathsWithDuplicates = []
pathsCount = 0
duplicate = {}
for path in paths:
    pathsCount += 1
    body = ""
    timestamps = []
    values = db.find_one({"_id": path})
    # pull out and list all the existing timestamp values
    for day in values:
        try:
            for hour in values[day]:
                try:
                    for value in values[day][hour]:
                        timestamps.append(value[0])
                except:
                    continue
        except:
            continue
    if [x for x, y in collections.Counter(timestamps).items() if y > 1]:
        duplicate = {path:x}
        print int(x)
        if int(x) > 1426777431000:
            print duplicate
            pathsWithDuplicates.append(duplicate)
#     print "\r%d - %s"%(pathsCount,duplicate),
#     print "%d of %d"%(pathsCount,len(paths))
    
print
print "All paths checked"
print
print "Duplicates found in paths:"
for path in pathsWithDuplicates:
    print path
print "*************************************************************************************"
print "*************************************************************************************"

