import matplotlib.pyplot as plt
import matplotlib.animation as animation
import serial

ser = serial.Serial("/dev/ttyACM0",115200)

if ser.is_open==True:
	print("\nAll right, serial port now open. Configuration:\n")
	print(ser, "\n") 

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = [] 
ys = [] 

def animate(i, xs, ys):

    line=ser.readline()
    line_as_list = line.split(b',')
    xs.append(float(line_as_list[0].decode()))
    ys.append(float(line_as_list[1].decode()))

    ax.clear()
    ax.cla()
    ax.plot(xs, ys, label="Chirp")


    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title('This is how I roll...')
    plt.ylabel('Amplitude')
    plt.legend()

ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=0)
plt.show()