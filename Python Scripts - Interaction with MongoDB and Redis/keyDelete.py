#!/usr/bin/env python
import sys
import pymongo, redis

db = pymongo.MongoClient().bldata.fielddata
rc = redis.Redis()

if len(sys.argv) == 2:
    keys = [sys.argv[1]]
else:
    keys = open("keys.txt","r").readlines()

for key in keys:
    key = key.strip()
    print key
print "Delete these keys? YES/no", 
ans = raw_input()

if ans == "YES":
    for key in keys:
        print ">"+key+"<"
        key = key.strip()
        print db.remove({"_id": key})
        print db.remove({"_id": key+".history"})
        print rc.delete(key)






