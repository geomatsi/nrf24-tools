#!/usr/bin/python

import matplotlib.pyplot as plot
import numpy as np

### main

# setup figure size: NB inches
plot.figure(figsize = (15, 10))

# read data

with open("t.txt") as f:
    data = f.read()

data = data.split('\n')

x = [float(row.split(';')[0]) for row in data if len(row) > 0]
y = [float(row.split(';')[1]) for row in data if len(row) > 0]

# plot 1: gas level

y1 = y
x1 = x

g1 = plot.subplot(211)
g1.set_title("Gas level")
g1.set_xlabel("time")
g1.set_ylabel("gas")

g1.plot(x1, y1, c='r', label='gas level')

# plot 2: sliding avg

y1 = np.convolve(y, np.ones((20,))/20, mode='valid')
x1 = x[0:len(y1)]

g2 = plot.subplot(212)
g2.set_title("Sliding avg")
g2.set_xlabel("time")
g2.set_ylabel("avg")

g2.plot(x1, y1, c='g', label='avg')

# draw new data
plot.show()
