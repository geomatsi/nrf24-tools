#!/usr/bin/python

import matplotlib.pyplot as plot
import matplotlib.dates as mdates
import numpy as np
import datetime

### main

# read data

with open("t.txt") as f:
    data = f.read()

data = data.split('\n') #[0:2000]

x = [float(row.split(';')[0]) for row in data if len(row) > 0]
y = [float(row.split(';')[1]) for row in data if len(row) > 0]

# x-axis: calculate minute interval to have 10 x-axis marks
mint = int((x[len(x) - 1] - x[0])/60/10)

# x-axis: convert epoch to date format
x = map(datetime.datetime.fromtimestamp, x)

# y-axis: threshold
thr = 1000

# cleanup
plot.close('all')

# setup figure size: NB inches
fig = plot.figure(figsize = (15, 10))
fig.canvas.set_window_title('Gas sensor')

# plot 1: gas level

y1 = y
x1 = x

a1 = fig.add_subplot(211)

a1.set_ylabel("adc")
a1.grid(True)

a1.plot(x1, y1, c='r')
a1.legend(['gas level'])

a1.xaxis.set_major_formatter(mdates.DateFormatter('%d-%m %H:%M'))
a1.xaxis.set_major_locator(mdates.MinuteLocator(interval=mint))

for tick in a1.get_xticklabels():
	tick.set_rotation(20)
	tick.set_size('x-small')

# plot 2.1: moving average

y1 = np.convolve(y, np.ones((20,))/20, mode='valid')
x1 = x[0:len(y1)]

a2 = fig.add_subplot(212)

a2.set_ylabel("average")
a2.grid(True)

a2.plot(x1, y1, c='g')

a2.xaxis.set_major_formatter(mdates.DateFormatter('%d-%m %H:%M'))
a2.xaxis.set_major_locator(mdates.MinuteLocator(interval=mint))

for tick in a2.get_xticklabels():
	tick.set_rotation(20)
	tick.set_size('x-small')

# plot 2.2: threshold crosses by moving average

y2 = [thr if (t > thr) else 0.0 for t in y1]
x2 = x1

a2.plot(x2, y2, c='b')
a2.legend(['moving average', "threshold (" + str(thr) + ")"])

# draw data
plot.show()
