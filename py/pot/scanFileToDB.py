#!/usr/bin/python
import os
import MySQLdb
import time
import hashlib
import sys
from config import *



def md5(fileName, excludeLine="", includeLine=""):
    """Compute md5 hash of the specified file"""
    m = hashlib.md5()
    try:
        fd = open(fileName,"rb")
    except IOError:
        print "Unable to open the file in readmode:", filename
        return
    content = fd.readlines()
    fd.close()
    for eachLine in content:
        if excludeLine and eachLine.startswith(excludeLine):
            continue
        m.update(eachLine)
    m.update(includeLine)
    return m.hexdigest()

def checkBin():
	binTable= getBinTable()
	path =scanPath
        dirList=os.listdir(path)
	existFilePath=[];
        for fname in dirList:
		fpath=path+fname
		existFilePath.append(fpath)
	for r in binTable:
		rowFilePath=r["filePath"]
		if( rowFilePath in existFilePath):
	 	 	existFilePath.remove(rowFilePath)
			print "DB already has "+rowFilePath
	for newPath in existFilePath:
		inserBin(newPath)
		print "Inser " +newPath 
	
def inserBin(fpath):
	fatime = getFileCreateDate(fpath,"%Y-%m-%d %H:%M:%S")
	fmd5=md5(fpath)
	cursor=mySqlInit()
	cursor.execute(
	"""INSERT INTO binaries (hostName , filePath,md5, createTime) 
	VALUES (%s ,%s, %s, %s);""",
	(hostName , fpath , fmd5 , fatime)
	)
	inserVirusTotalDB(fmd5)

def getFileCreateDate(fpath,strftime):
	return	time.strftime(strftime,time.localtime(os.path.getctime(fpath)))

	

def inserVirusTotalDB(md5):
         cursor=mySqlInit()
         try:
               cursor.execute(
                        """INSERT INTO virustotalDB (md5 ,updateTime )
                        VALUES (%s, %s);""",(
			md5 , "2000-01-01 00:00:00"
			)
		)
         except:
		print "virustotalDB already has "+ md5


def getBinTable():
	cursor =mySqlInit() 
	cursor.execute("SELECT  filePath,md5, createTime FROM binaries WHERE hostName = %s",(hostName))
	result = cursor.fetchall()
	return result

if __name__ == "__main__":
        if len(sys.argv)>1:
		scanPath=os.path.abspath(sys.argv[1])+"/"
	else:
		scanPath=getBinariesPath()
	print "Scan Path :"+scanPath
	if(not os.path.exists(scanPath)):
		print "Path is not exists"
	else:
		checkBin()
