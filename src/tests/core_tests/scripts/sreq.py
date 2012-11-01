#!/usr/bin/env python
# coding=utf-8

import sys
import getopt
import httplib
import urllib2

##
def send_get_req(host, port, req, out, bytes_range):
	try :
		params=""
		headers={"Range": "bytes=" + str(bytes_range), "Connection" : "close"}
		http_conn = httplib.HTTPConnection(host, port)
		http_conn.connect()

		http_conn.request('GET', req, params, headers)

		response = http_conn.getresponse()
		if response.status == httplib.OK:
			print response.read()
		elif response.status == httplib.PARTIAL_CONTENT :
			binfile = open(out, 'wb')
			binfile.write(response.read())
			return 
		else :
			print "Server unexpected answer : ", response.status
			return 

	except httplib.HTTPException, e :
		print 'Connection refused ', str(e)
#

##
def send_head_req(host, port, req, out):
	try :
		params=""
		headers={}

		http_conn = httplib.HTTPConnection(host, port)
		http_conn.connect()
	
		http_conn.request('HEAD', req, params, headers)
		
		response = http_conn.getresponse();
		if (response.status == httplib.OK):
			print response.read();
		else :
			print "Server unexpected answer : ", response.status
	except httplib.HTTPException, e :
		print 'Connection refused ', str(e)
#

## Print to stdout usage and do exit
def print_usage_and_exit():
	print "Usage : [./]sreq [--*=|-*=][--|-] -- [OPTIONS] ..."
	print ""
	print "Options :"	
	print ""
	print "  --usage | -u                     Print help and exit"	
	print "  --pr= | -r                       Server request path"
	print "  --of= | -o=      [OPTIONAL]      Out put file path(server answer data), default value PWD/serv_reply.out"
	print "  --host= | -h=    [OPTIONAL]      Host name, default value 127.0.0.1"
	print "  --port= | -p=    [OPTIONAL]      Port number, default value 80"
	print "  --get | -g       [OPTIONAL]      Set request to send HTTP GET, this is default values"
	print "  --head | -d      [OPTIONAL]      Set request to send HTTP HEAD"
	print "  --br | -b        [OPTIONAL]      Set byte range in get-range requst, default value is 0-(full file)"
	sys.exit(0)
#

## Command line arguments parser 
def parse_argv(argv):
	action = send_head_req
	host = "127.0.0.1"
	port = "80"
	out = "serv_reply.out"
	req = None
	brange = "0-"
	if argv != None:
		try: 
			opts, args = getopt.getopt(argv[1:], "h:p:g:d:o:r:u:b",
				["host=", "port=", "get", "head", "of=", "pr=", "usage", "br="])
			for o, a in opts :
				if o in "--usage" :
					print_usage_and_exit()
				if o in "--host=" :
					host = a
				if o in "--port=" :
					port = a
				if o in "--get" :
					action = "get"
				if o in "--head" :
					action = "head"
				if o in "--of=" :
					out = a
				if o in "--pr=" :
					req = a
				if o in "--br=":
					brange = a
			#
		except getopt.error, msg:
			print "Error while parsing command line arguments : ",  msg

	values=[host, port, req, out, brange]
	if all(v == None for v in values) and action == None :
		return (None, None)
	return (values, action)
# 

## Entry point  
def main(argv=sys.argv):
	(values, action) = parse_argv(argv)
	if values == None or action == None :
		print "Failed : not valid argument '", values, "' or bad action cames '", "' action"
		sys.exit(1)
	else :
		if action == "head" :
			send_head_req(values[0], values[1], values[2], values[3])
		elif action == "get" :
			send_get_req(values[0], values[1], values[2], values[3], values[4])
	sys.exit(0)
	print "Done."
#

if __name__ == "__main__":
	main()

