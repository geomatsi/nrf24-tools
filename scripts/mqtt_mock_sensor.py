#!/usr/bin/python

import mosquitto
import time
import getopt
import sys

### mosquitto methods

def on_connect(writer, userdata, rc):
	print("mqtt connected: rc = " + str(rc))

def on_publish(writer, userdata, mid):
	print("mqtt published: mid = " + str(mid))

### main

### main: command line args

data = "test.dat"
server = "localhost"
port = 1883
topic = "test/plot"

try:
	opts, args = getopt.getopt(sys.argv[1:],"hd:s:p:t:",["data=","server=", "port=", "topic="])
except getopt.GetoptError:
	print sys.argv[0], " -d <data file> -s <mqtt server> -p <mqtt server port> -t <mqtt topic>"
	sys.exit(-1)

for opt, arg in opts:
	if opt == '-h':
		print sys.argv[0], " -d <data file> -s <mqtt server> -p <mqtt server port> -t <mqtt topic>"
		sys.exit(0)
	elif opt in ("-d", "--data"):
		data = arg
	elif opt in ("-s", "--server"):
		server = arg
	elif opt in ("-p", "--port"):
		port = arg
	elif opt in ("-t", "--topic"):
		topic = arg
	else:
		print sys.argv[0], " -d <data file> -s <mqtt server> -p <mqtt server port> -t <mqtt topic>"
		sys.exit(0)

### main: prepare mock sensor data

with open(data) as f:
    data = f.read()

data = data.split('\n')
values = [float(row.split(';')[1]) for row in data if len(row) > 0]

### main: connect to mqtt

writer = mosquitto.Mosquitto();

writer.on_connect = on_connect
writer.on_publish = on_publish

writer.connect(server, port, 60)
writer.loop_start()

### main: loop

for v in values:
	writer.publish(topic, v)
	time.sleep(1)

writer.disconnect()
