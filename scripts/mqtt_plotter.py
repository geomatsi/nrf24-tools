#!/usr/bin/python

import mosquitto
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

### main: prepare live plot

# setup animated plot
plot.ion()

# prepare axes and plot
ydata = [0] * 50
line, = plot.plot(ydata)
plot.ylim([0, 5000])

### main: connect to mqtt

client = mosquitto.Mosquitto();
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.subscribe("test/plot")

### main: loop

client.loop_forever()
