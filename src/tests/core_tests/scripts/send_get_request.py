#!/usr/bin/env python
# coding=utf-8

import sys
import getopt
import httplib
import urllib2

def print_text(txt):
	lines = txt.split('\n')
	for line in lines:
		print line.strip()
#

def send_get_req(host, port, req):
	try :
		params=""
		headers={"Range": "bytes=0-10", "Connection" : "close"}
		http_serv = httplib.HTTPConnection(host, port)
		http_serv.connect()

		http_serv.request('GET', req, params, headers)

		response = http_serv.getresponse()
		if response.status == httplib.OK:
			print "Server answer :"
			print_text(response.read())
			return
		print 'Failed with status : ', response.status
	except httplib.HTTPException :
		print 'Connection refused'
#


def main():
	send_get_req("127.0.0.1", 80, "/system.log")
	sys.exit(0)
#

main()

