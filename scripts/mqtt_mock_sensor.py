#!/usr/bin/python

import mosquitto
import time

### mosquitto methods

def on_connect(writer, userdata, rc):
	print("mqtt connected: rc = " + str(rc))

def on_publish(writer, userdata, mid):
	print("mqtt published: mid = " + str(mid))

### main

with open("test.dat") as f:
    data = f.read()

data = data.split('\n')
values = [float(row.split(';')[1]) for row in data if len(row) > 0]

### main: connect to mqtt

writer = mosquitto.Mosquitto();

writer.on_connect = on_connect
writer.on_publish = on_publish

writer.connect("localhost", 1883, 60)
writer.loop_start()

### main: loop

for v in values:
	writer.publish("test/plot", v)
	time.sleep(1)

writer.disconnect()
