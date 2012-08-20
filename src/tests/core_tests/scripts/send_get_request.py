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
	send_get_req("thepiratebay.se", 80, "torrent/7540111/The_Script_-_Hall_of_Fame_(feat._will.i.am)_[Single_-_2012].torrent")
	sys.exit(0)
#

main()

