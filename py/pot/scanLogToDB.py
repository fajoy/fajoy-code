#!/usr/bin/python
import os
import MySQLdb
import time
import sys
from config import *
from scanDionaeaLog import *

class LogData:
	time=""
	path=""
	url=""
	md5=""
	def __init__(self,time):
		self.time = time

	def show(self):
		print "%s : url=%s md5=%s path=%s" %(self.time,self.url,self.md5,self.path)

	def setData(self,k,v):
		if k=="md5hash":
			self.md5=v
		elif k=="file":
			self.path=os.path.abspath(v)
		elif k=="url":
			self.url=v

def getLogData():
	os.chdir(dionaeaRootPath)
	global logData
	logData={}
	while 1:
		line = f.readline()
		if not line:
			return
		else:
			m=getNeedLog(line)
			if m:
				lday=m.group("day")
				lmonth=m.group("month")
				lyear=m.group("year")
				ltime=m.group("time")
				ltype=m.group("type")
				lvalue=m.group("value")
				logTime="%s-%s-%s %s" % (lyear,lmonth,lday,ltime)
				if not logTime in logData:
					logData[logTime]=LogData(logTime)
				logData[logTime].setData(ltype,lvalue)

def inserLogData():
	cursor=mySqlInit()
	for log in logData.values():
		try:
			cursor.execute(
			"""INSERT INTO binariesSourceLog (logTime,hostName , filePath,md5,url ) 
			VALUES (%s ,%s, %s, %s, %s);""",
			(log.time,hostName , log.path ,log.md5 , log.url)
			)
			log.show()
		except:
			print "DB already has log " +log.time
			pass

def scanLogFileToDB(path):
	global f
        f = open(path, 'r')
	getLogData()
	inserLogData()

if __name__ == "__main__":
        if len(sys.argv)>1:
                scanPath=os.path.abspath(sys.argv[1])
        else:
                scanPath=getLogPath()
        print "Scan Path :"+scanPath
        if(not os.path.exists(scanPath)):
                print "Path is not exists"
        else:
                scanLogFileToDB(scanPath)

