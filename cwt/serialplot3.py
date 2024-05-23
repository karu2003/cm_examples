import matplotlib.pyplot as plt
import matplotlib.animation as animation
import serial

ser = serial.Serial("/dev/ttyUSB0",115200) #/dev/ttyACM0 /dev/ttyUSB2
# ser = serial.Serial("/dev/ttyUSB0", 115200, rtscts=False, dsrdtr=False, xonxoff=False)
# ser = serial.Serial("/dev/ttyUSB0", 115200, xonxoff=True)

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
    print(line_as_list)
    xs.append(int(line_as_list[0]))
    ys.append(float(line_as_list[1].decode()))
    # try:
    #     ys.append(float(line_as_list[1].decode().rstrip('\\r0')))        
    # except ValueError as e:
    #     print("error",{e},"on line",)
    #     print(">>>>>>>>>>>>>>>>>>>><<",line_as_list[1])
    # else:
    #     xs.append(int(line_as_list[0]))
    
    

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