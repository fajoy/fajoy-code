#!/usr/bin/python
import MySQLdb
import os

def getLocalAddress():
	import re
	import urllib2
        pattern = re.compile("\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}")
        headers  = {'User-Agent':'Firefox','Referer':'http://www.google.com'}
        request = urllib2.Request('http://checkip.dyndns.org',{},headers)
        response = urllib2.urlopen(request)
        ip  = pattern.findall(response.read())
        return ip[0]

hostName = getLocalAddress() #inser DB PRIMARY index. 
#hostName = os.getenv('HOSTNAME')
#hostName = "machine name"
dionaeaRootPath="/opt/dionaea/"
binariesPath="/opt/dionaea/var/dionaea/binaries/"
logPath="/opt/dionaea/var/log/dionaea.log"
sqlitePath="/opt/dionaea/var/dionaea/logsql.sqlite"
tarLogPath="/opt/dionaea/var/log/"
VirustotalApiKey="1caa9d65b48d53646187d9fceffae3db6b9957da506a72298b5da0089fa9f3c9"


def mySqlInit():
        conn = MySQLdb.connect(host="",
                port=,
                user="",
                passwd="",
                db="dionaen")
        cursor = conn.cursor (MySQLdb.cursors.DictCursor)
        return cursor

def getBinariesPath():
	return os.path.abspath(binariesPath)+"/"

def getLogPath():
	return os.path.abspath(logPath)


