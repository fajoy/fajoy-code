#!/usr/bin/python
import simplejson
import urllib
import urllib2
import sys
from config import *


def getVirusTotalJson(md5):
	url = "https://www.virustotal.com/api/get_file_report.json"
	parameters = {"resource": md5,
		 "key": VirustotalApiKey}
	data = urllib.urlencode(parameters)
	req = urllib2.Request(url, data)
	response = urllib2.urlopen(req)
	json = response.read()
	return json

def getTestJson():
	return """
{"report": ["2011-09-05 00:45:24", {"nProtect": "Backdoor/W32.Agent.33128", "CAT-QuickHeal": "Backdoor.Agent.rqr", "McAfee": "Generic BackDoor", "K7AntiVirus": "", "TheHacker": "Backdoor/Agent.rqr", "VirusBuster": "Backdoor.Agent!mgrK77Tn3jM", "NOD32": "Win32/Pepex.G", "F-Prot": "W32/SuspPack.CY.gen!Eldorado", "Symantec": "Backdoor.Trojan", "Norman": "W32/Packed_Upack.A", "ByteHero": "", "TrendMicro-HouseCall": "TROJ_MEREDROP.II", "Avast": "Win32:Malware-gen", "eSafe": "Win32.TRDropper", "ClamAV": "PUA.Packed.UPack-2", "Kaspersky": "Backdoor.Win32.Agent.aknp", "BitDefender": "Trojan.Spy.XXP", "ViRobot": "Backdoor.Win32.Agent.33128", "Emsisoft": "Trojan-Dropper.Agent!IK", "Comodo": "Packed.Win32.MUPACK.~KW", "F-Secure": "Trojan.Spy.XXP", "DrWeb": "Trojan.MulDrop.40222", "VIPRE": "Trojan.Win32.Packer.Upack0.3.9 (ep)", "AntiVir": "TR/Dropper.Gen", "TrendMicro": "TROJ_MEREDROP.II", "McAfee-GW-Edition": "Heuristic.LooksLike.Win32.Suspicious.C", "Sophos": "Mal/Dropper-O", "eTrust-Vet": "Win32/Tnega.SRT", "Jiangmin": "Backdoor/Agent.cahy", "Antiy-AVL": "Backdoor/Win32.Agent.gen", "Microsoft": "Worm:Win32/Small.AF", "SUPERAntiSpyware": "", "Prevx": "", "GData": "Trojan.Spy.XXP", "Commtouch": "W32/SuspPack.CY.gen!Eldorado", "AhnLab-V3": "Win-Trojan/Agent.33128.B", "VBA32": "Backdoor.Win32.Agent.rqr", "PCTools": "Backdoor.Agent!sd6", "Rising": "Suspicious", "Ikarus": "Trojan-Dropper.Agent", "Fortinet": "W32/Dropper.O!tr", "AVG": "BackDoor.Agent.XGF", "Panda": "Trj/Pupack.A", "Avast5": "Win32:Malware-gen"}], "permalink": "http://www.virustotal.com/file-scan/report.html?id=a29d02251f54567edb1d32f7c17ce4c04d5c54e317eb3b2bea2a068da728e59a-1315183524", "result": 1}
"""
def showVirusInfo(jsonString):
        json = simplejson.loads(jsonString)
        report= json.get("report")
        time=report[0]
        url=json.get("permalink")
        virusInfo=report[1]
	result=json.get("result")
        for info in virusInfo:
                print info+" : "+virusInfo[info]
	print "report time : " + time
	print "url : " + url
	print "result : " + str( result)



if __name__ == "__main__":
	json=getVirusTotalJson( sys.argv[1])
	#json=getTestJson()
	showVirusInfo(json)

