#!/usr/bin/python
import MySQLdb
from virusTotal import *
from config import *

def updateVirusTotalDB(md5,jsonString):
	json = simplejson.loads(jsonString)
	result=json.get("result")
	if (result == 1):
		report= json.get("report")
		time=report[0]
		url=json.get("permalink")
		virusInfo=report[1]
		cursor=mySqlInit()
		cursor.execute(
	       	"""UPDATE virustotalDB SET submitTime = %s ,url =%s, updateTime=null , status=%s
		   WHERE md5 = %s ;
	        """,
	        (time,url,md5,"gotJson")
	       	),
		#updateVTDBstatus('gotJson',md5)
	        for info in virusInfo:
			try:
			     cursor.execute(
			     """INSERT INTO virustotalInfo (md5 , antivirusName , virusType)
			     VALUES (%s, %s, %s);""",
			     (md5,info ,virusInfo[info])
			     )
			except:
				cursor.execute(
				"""UPDATE virustotalInfo SET virusType = %s  
				WHERE (antivirusName = %s ) AND ( md5 = %s ) ; """,
				(virusInfo[info],info,md5)
				)
				pass
		print "update virustotalDB md5 :" + md5
	else:
		updateVTDBstatus("NoData",md5)
		print "virustotal not found virus data md5 : "+ md5
		pass

def updateVTDBstatus(status,md5):
	cursor=mySqlInit()
	cursor.execute(
	"""UPDATE virustotalDB SET status = %s ,updateTime= null
	WHERE md5 = %s  ; """,
	(status,md5)
	)
	

def updateVirusTotal():
        cursor =mySqlInit()
        cursor.execute("""
	SELECT  md5
	FROM  virustotalDB
	ORDER BY  updateTime ASC 
	LIMIT 0 , 4
	""")
        result = cursor.fetchall()
	for row in result :
		try:
		       md5=row["md5"]
		       json=getVirusTotalJson(md5)
		       updateVirusTotalDB(md5,json)
		except:
			updateVTDBstatus("updateError",md5)
			print "update error md5 : " + md5
			pass


if __name__ == "__main__":
	updateVirusTotal()
