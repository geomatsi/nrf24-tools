#!/usr/bin/python

import matplotlib.pyplot as plot
import matplotlib.dates as mdates
import numpy as np
import datetime
import getopt
import sys

### math

# simple moving average
def sma_filter(x, n):
	return np.convolve(y, np.ones((n,))/n, mode='valid')

# weighted moving average
def wma_filter(x, w):
	wf = map(float, w)
	ws = np.sum(w)
	return np.convolve(y, wf/ws, mode='valid')

### main

### main: command line args

data = "t.txt"
threshold = 1000

try:
	opts, args = getopt.getopt(sys.argv[1:],"hd:t:",["data=", "threshold="])
except getopt.GetoptError:
	print sys.argv[0], " -d <data file> -t <threshold>"
	sys.exit(-1)

for opt, arg in opts:
	if opt == '-h':
		print sys.argv[0], " -d <data file> -t <threshold>"
		sys.exit(0)
	elif opt in ("-d", "--data"):
		data = arg
	elif opt in ("-t", "--threshold"):
		threshold = int(arg)
	else:
		print sys.argv[0], " -d <data file> -t <threshold>"
		sys.exit(0)

# read data

try:
	with open(data) as f:
		data = f.read()
except IOError as e:
	print "Open file {0}: I/O error({1}): {2}".format(data, e.errno, e.strerror)
	sys.exit(-1)

data = data.split('\n') #[0:2000]

x = [float(row.split(';')[0]) for row in data if len(row) > 0]
y = [float(row.split(';')[1]) for row in data if len(row) > 0]

# x-axis: calculate minute interval to have 10 x-axis marks
mint = int((x[len(x) - 1] - x[0])/60/10)

if mint == 0:
	print "WARN: data range is less than 10 minutes, use 1min axis marks"
	mint = int((x[len(x) - 1] - x[0])/60)

# x-axis: convert epoch to date format
x = map(datetime.datetime.fromtimestamp, x)

# cleanup
plot.close('all')

# setup figure size: NB inches
fig = plot.figure(figsize = (15, 10))
fig.canvas.set_window_title('Gas sensor')

# plot 1: gas level

y1 = y
x1 = x

a1 = fig.add_subplot(311)

a1.set_ylabel("adc")
a1.grid(True)

a1.plot(x1, y1, c='r')
a1.legend(['gas level'])

a1.xaxis.set_major_formatter(mdates.DateFormatter('%d-%m %H:%M'))
a1.xaxis.set_major_locator(mdates.MinuteLocator(interval=mint))

for tick in a1.get_xticklabels():
	tick.set_rotation(10)
	tick.set_size('x-small')

# plot 2.1: simple moving average

y1 = sma_filter(y, 20)

x1 = x[0:len(y1)]

a2 = fig.add_subplot(312)

a2.set_ylabel("simple average")
a2.grid(True)

a2.plot(x1, y1, c='g')

a2.xaxis.set_major_formatter(mdates.DateFormatter('%d-%m %H:%M'))
a2.xaxis.set_major_locator(mdates.MinuteLocator(interval=mint))

for tick in a2.get_xticklabels():
	tick.set_rotation(10)
	tick.set_size('x-small')

# plot 2.2: threshold crosses by moving average

y2 = [threshold if (t > threshold) else 0.0 for t in y1]
x2 = x1

a2.plot(x2, y2, c='b')
a2.legend(['moving average', "threshold (" + str(threshold) + ")"])

# plot 3.1: weighted moving average

y1 = wma_filter(y, [1,3,6,3,1])

x1 = x[0:len(y1)]

a3 = fig.add_subplot(313)

a3.set_ylabel("weighted average")
a3.grid(True)

a3.plot(x1, y1, c='g')

a3.xaxis.set_major_formatter(mdates.DateFormatter('%d-%m %H:%M'))
a3.xaxis.set_major_locator(mdates.MinuteLocator(interval=mint))

for tick in a3.get_xticklabels():
	tick.set_rotation(10)
	tick.set_size('x-small')

# plot 3.2: threshold crosses by moving average

y2 = [threshold if (t > threshold) else 0.0 for t in y1]
x2 = x1

a3.plot(x2, y2, c='b')
a3.legend(['moving average', "threshold (" + str(threshold) + ")"])


# draw data
plot.show()
