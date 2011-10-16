#!/usr/bin/python
import os
import MySQLdb
import time
import sys
import tarfile
from config import *
from scanDionaeaLog import *
from scanLogToDB import *



def getFileModifiDate(fpath,strftime):
	return  time.strftime(strftime,time.localtime(os.path.getmtime(fpath)))

def mvLogData(fpath):
	newLogName= getFileModifiDate(fpath,"%Y%m%d%H%M%S")+".log"
	newLogName= os.path.abspath(tarLogPath+"/"+newLogName)
	os.rename(fpath,newLogName)
	print "mv %s %s"%(fpath,newLogName)
	return newLogName
		

def tarFile(fpath):
	tarPath=os.path.abspath(fpath+".bz2")
	tar = tarfile.open(tarPath, "w:bz2")
	tarName=os.path.basename(fpath)
    	tar.add(fpath,"/"+tarName)
	tar.close()
	print "tar %s"%(tarPath)
	return 1


if __name__ == "__main__":
        if len(sys.argv)>1:
                scanPath=os.path.abspath(sys.argv[1])
        else:
                scanPath=getLogPath()
	        print "Scan Path :"+scanPath
       	if(not os.path.exists(scanPath)):
               	print "Path is not exists"
        else:
		newPath=mvLogData(scanPath)
		scanLogFileToDB(newPath)
		if tarFile(newPath):
			os.remove(newPath)
			print "del %s" %(newPath)


