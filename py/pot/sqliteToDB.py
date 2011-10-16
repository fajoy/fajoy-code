#!/usr/bin/python
import os
import MySQLdb
import time
import hashlib
import sys
from config import *
from pysqlite2 import dbapi2 as sqlite

def sqliteInit():
	con = sqlite.connect(sqlitePath)
	cursor = con.cursor()
        return cursor
	

def inserBin(fpath):
	fatime = time.strftime("%Y/%m/%d %H:%M:%S",
        	time.localtime(os.path.getctime(fpath)))
	fmd5=md5(fpath)
	cursor=mySqlInit()
	cursor.execute(
	"""INSERT INTO binaries (hostName , filePath,md5, createTime) 
	VALUES (%s ,%s, %s, %s);""",
	(hostName , fpath , fmd5 , fatime)
	)
	inserVirusTotalDB(fmd5)

def getBinTable():
	cursor =mySqlInit() 
	cursor.execute("SELECT  filePath,md5, createTime FROM binaries WHERE hostName = %s",(hostName))
	result = cursor.fetchall()
	return result

def importConnectionsTB():
	print  "start import ConnectionsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM connections ")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		row[5]=time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(row[5]))
		try:
			cursor.execute(
			"""
			INSERT INTO `connections` (`hostName`, `connection`, `connection_type`, `connection_transort`,
				 `connection_protocol`, `connection_timestamp`,
				 `connection_root`, `connection_parent`, `local_host`,
				 `local_port`, `remote_host`, `remote_hostname`, `remote_port`) VALUES (%s, %s,%s,%s, %s, %s, %s, %s, %s, %s, %s,%s, %s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import ConnectionsTB"

def importOffersTB():
	print  "start import OffersTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM offers ")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
			INSERT INTO `offers` (`hostName`, `offer`, `connection`, `offer_url`)
			 VALUES (%s, %s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import OffersTB"

def importDownloadsTB():
	print  "start import DownloadsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM downloads ")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
			INSERT INTO `downloads` (`hostName`,`download` ,`connection`, `download_url`, `download_md5_hash`)
			 VALUES (%s,%s, %s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import DownloadsTB"

def importEmu_profilesTB():
	print  "start import Emu_profilesTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM emu_profiles")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
			INSERT INTO `emu_profiles` (`hostName`, `emu_profile`, `connection`, `emu_profile_json`)
			 VALUES (%s, %s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Emu_profilesTB"

def importLoginsTB():
	print  "start import LoginsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM logins")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
			INSERT INTO `logins` (`hostName`, `login`, `connection`, `login_username`, `login_password`)
			 VALUES (%s, %s,%s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import LoginsTB"

def importMssql_fingerprintsTB():
	print  "start import Mssql_fingerprintsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM mssql_fingerprints")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
INSERT INTO `mssql_fingerprints` (`hostName`, `mssql_fingerprint`, `connection`, `mssql_fingerprint_hostname`, `mssql_fingerprint_appname`, `mssql_fingerprint_cltintname`)
			 VALUES (%s, %s,%s,%s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Mssql_fingerprintsTB"

def importMysql_commandsTB():
	print  "start import Mysql_commandsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM logins")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
			INSERT INTO `mysql_commands` (`hostName`, `mysql_command`, `connection`, `mysql_command_status`, `mysql_command_cmd`)
			VALUES (%s, %s,%s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Mysql_commandsTB"

def importMysql_command_argsTB():
	print  "start import Mysql_command_argsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM mysql_command_args")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
			INSERT INTO `mysql_command_args` (`hostName`, `mysql_command_arg`, `mysql_command`, `mysql_command_arg_index`, `mysql_command_arg_data`)
			VALUES (%s, %s,%s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Mysql_command_argsTB"

def importMysql_command_opsTB():
	print  "start import Mysql_command_opsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM mysql_command_ops")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		try:
			cursor.execute(
			"""
			INSERT INTO `mysql_command_ops` (`mysql_command_op`, `mysql_command_cmd`, `mysql_command_op_name`)
			VALUES (%s, %s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Mysql_command_opsTB()"

def importSip_commandsTB():
	print  "start import Sip_commandsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM sip_commands")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
INSERT INTO `sip_commands` (`hostName`, `sip_command`, `connection`, `sip_command_method`, `sip_command_call_id`, `sip_command_user_agent`, `sip_command_user_allow`)
			VALUES (%s, %s,%s,%s,%s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Sip_commandsTB"

def importSip_addrsTB():
	print  "start import Sip_addrsTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM sip_addrs")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
INSERT INTO `sip_addrs` (`hostName`, `sip_addr`, `sip_command`, `sip_addr_type`, `sip_addr_display_name`, `sip_addr_uri_scheme`, `sip_addr_uri_user`, `sip_addr_uri_password`, `sip_addr_uri_host`, `sip_addr_uri_port`)
			VALUES (%s, %s,%s,%s,%s,%s, %s,%s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Sip_addrsTB"


def importSip_viasTB():
	print  "start import Sip_viasTB"
	cursor=mySqlInit()
	cur=sqliteInit()
	cur.execute("SELECT * FROM sip_vias")
	while 1:
		row=cur.fetchone()
		if not row:
			break;
		row=list(row)
		row.insert(0,hostName)
		try:
			cursor.execute(
			"""
INSERT INTO `sip_vias` (`hostName`, `sip_via`, `sip_command`, `sip_via_protocol`, `sip_via_address`, `sip_via_port`)
			VALUES (%s, %s,%s,%s,%s,%s);
			""",
			row
			)
			print row
		except:
			pass
	print "end import Sip_viasTB"

def importDB():
	#importMysql_command_opsTB()
	importConnectionsTB()
	importOffersTB()
	importDownloadsTB()
	importEmu_profilesTB()
	importLoginsTB()
	importMssql_fingerprintsTB()
	importMysql_commandsTB()
	importMysql_command_argsTB()
	importSip_commandsTB()
	importSip_addrsTB()
	importSip_viasTB()
	
if __name__ == "__main__":
        if len(sys.argv)>1:
		sqlitePath=os.path.abspath(sys.argv[1])
	print "sqlite Path :"+sqlitePath
	if(not os.path.exists(sqlitePath)):
		print "Path is not exists"
	else:
		importDB()
