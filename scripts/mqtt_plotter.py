#!/usr/bin/python

import mosquitto
import getopt
import sys

from matplotlib import pyplot as plot

### mosquitto methods

def on_connect(client, userdata, rc):
	print("mqtt connected: rc = " + str(rc))
	# subscribe here to renew subscription on reconnect
	client.subscribe("test/plot")

def on_message(client, userdata, msg):
	print(msg.topic + ": " + str(msg.payload))

	# read new data and update plot storage
	data = float(msg.payload)
	ydata.append(data)
	del ydata[0]

	# adjust axis ranges
	ymin = float(min(ydata))
	ymax = float(max(ydata))
	plot.ylim([ymin - ymin/10, ymax + ymax/10])
	line.set_xdata(range(len(ydata)))
	line.set_ydata(ydata)

	# draw new data
	plot.draw()

### main

### main: command line args

server = "localhost"
port = 1883
topic = "test/plot"

try:
	opts, args = getopt.getopt(sys.argv[1:],"hs:p:t:",["server=", "port=", "topic="])
except getopt.GetoptError:
	print sys.argv[0], " -s <mqtt server> -p <mqtt server port> -t <mqtt topic>"
	sys.exit(-1)

for opt, arg in opts:
	if opt == '-h':
		print sys.argv[0], " -s <mqtt server> -p <mqtt server port> -t <mqtt topic>"
		sys.exit(0)
	elif opt in ("-s", "--server"):
		server = arg
	elif opt in ("-p", "--port"):
		port = arg
	elif opt in ("-t", "--topic"):
		topic = arg
	else:
		print sys.argv[0], " -s <mqtt server> -p <mqtt server port> -t <mqtt topic>"
		sys.exit(0)

### main: prepare live plot

# setup animated plot
plot.ion()

# setup figure size: NB inches
plot.figure(figsize = (15, 10))

# prepare axes and plot
ydata = [0] * 1000
line, = plot.plot(ydata)
plot.ylim([0, 5000])

### main: connect to mqtt

client = mosquitto.Mosquitto();
client.on_connect = on_connect
client.on_message = on_message

client.connect(server, port, 60)
client.subscribe(topic)

### main: loop

client.loop_forever()
