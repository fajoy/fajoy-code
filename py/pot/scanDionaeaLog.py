#!/usr/bin/python
import sys
import os
import MySQLdb
import time
import re
from config import *

"""
log ex:
[08092011 16:53:06] incident incident.c:123-debug:      url: (string) http://77.37.133.16:4263/kjrlcypo
[08092011 16:53:06] incident incident.c:123-debug:      file: (string) var/dionaea/binaries/0fae1085f66bfb53f2e1b2743c
[08092011 16:53:06] incident incident.c:123-debug:      md5hash: (string) 0fae1085f66bfb53f2e1b2743cee4fde
"""
def getNeedLog(text):
        #m = re.match(r"\[(..)(..)(....) (.{8})\]\s+\S+\s+\S+\s+(url|file|md5hash):\s+\S+\s+(\S+)", t)
        m = re.match(r"\[(?P<day>..)(?P<month>..)(?P<year>....) (?P<time>.{8})\]\s+\S+\s+\S+\s+(?P<type>url|file|md5hash):\s+\S+\s+(?P<value>\S+)", text)
	return m

def showLogFile():
        f = open(scanPath, 'r')
        while 1:
                line = f.readline()
                if not line:
                        break
                else:
                        m=getNeedLog(line)
                        if m:
                                lday=m.group("day")
                                lmonth=m.group("month")
                                lyear=m.group("year")
                                ltime=m.group("time")
                                ltype=m.group("type")
                                lvalue=m.group("value")
                                print "%s-%s-%s %s : %s = %s" %(lyear,lmonth,lday,ltime,ltype,lvalue)

if __name__ == "__main__":
        if len(sys.argv)>1:
                scanPath=os.path.abspath(sys.argv[1])
        else:
                scanPath=getLogPath()
        print "Scan Path :"+scanPath
        if(not os.path.exists(scanPath)):
                print "Path is not exists"
	else:
		showLogFile()

