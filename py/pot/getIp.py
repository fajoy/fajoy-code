#!/usr/bin/python
import re
import urllib2

def getLocalAddress():
        pattern = re.compile("\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}")
        headers  = {'User-Agent':'Firefox','Referer':'http://www.google.com'}
        request = urllib2.Request('http://checkip.dyndns.org',{},headers)
        response = urllib2.urlopen(request)
        ip  = pattern.findall(response.read())
        return ip[0]

if __name__ == "__main__":
	print getLocalAddress()

