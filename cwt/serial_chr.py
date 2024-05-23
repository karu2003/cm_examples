
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import serial

fig, ax = plt.subplots()
line, = ax.plot(np.random.rand(10))
ax.set_ylim(-1, 1)
xdata, ydata = [0]*100, [0]*100
raw = serial.Serial("/dev/ttyACM0",115200)
raw.flush()

y = []
count=1
serial_available = False    
while True:
    while raw.in_waiting:
        for line in raw.read():
            print(str(count) + str(': ') + chr(line) )
            count = count+1

