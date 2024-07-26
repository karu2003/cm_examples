import serial
import time
import sys

import numpy as np
import matplotlib.pyplot as plt
import os
from matplotlib.colors import LinearSegmentedColormap

port = "/dev/ttyUSB0"
port = "/dev/ttyACM0"
baudrate = 115200
baudrate = 250000
baudrate = 500000
baudrate = 1000000

def read_parameters_and_data(ser):
    parameters = None
    data = []

    command_received = False
    spinner = ["|", "/", "-", "\\"]

    while True:
        line = ser.readline().decode("utf-8").strip()
        if line.startswith("§"):
            parameters = line[1:].split()
            command_received = True
            data_count = 0
        elif command_received:
            if line == "":
                break
            real, imag = map(float, line.split(","))
            data.append((real, imag))
            data_count += 1
            if data_count % 100 == 0:
                for symbol in spinner:
                    sys.stdout.write("\r" + symbol)
                    sys.stdout.flush()
                    time.sleep(0.01)

    return parameters, data

while True:

    ser = serial.Serial(port, baudrate)

    print("Press the user button on Coral Micro")
    parameters, data = read_parameters_and_data(ser)
    n, fn, f0, f1 = map(float, parameters)
    print("\n")
    print(f"Signal Options: n={n}, fn={fn}, f0={f0}, f1={f1}")

    ser.close()

    # data_array = np.array(data, dtype=float)
    # data_c = data_array.view(np.complex128)
    # data_complex = data_c.reshape((int(n), -1), order="F")
    # data_complex_transposed = np.transpose(data_complex)

    data_complex_transposed = (
        np.array(data, dtype=float).view(np.complex128).reshape((int(n), -1), order="F").T
    )

    # data_complex_transposed = threshold_2Darray(data_complex_transposed, 0.3)

    data_max_only = np.zeros_like(data_complex_transposed)
    max_indices = np.argmax(data_complex_transposed, axis=1)

    for row, max_index in enumerate(max_indices):
        data_max_only[row, max_index] = data_complex_transposed[row, max_index]
        # data_max_only[row, max_index] = 255

    colors = ["#ffffff", "#ff0000"]
    cmap_name = "white_red"
    cm = LinearSegmentedColormap.from_list(
        cmap_name, colors, N=100
    )  # N - количество уровней

    ytick_labels = np.around(np.linspace(int(f1), int(f0), int(fn)), 2)
    ytick_positions = np.linspace(0, data_complex_transposed.shape[0] - 1, int(fn))

    plt.subplot(1, 2, 1)  # 1 строка, 2 колонки, первый подграфик
    plt.imshow(np.abs(data_complex_transposed), aspect="auto")  # , interpolation="none")
    plt.colorbar()
    plt.title("Original CWT absolute values")
    plt.ylabel("Frequency")
    plt.xlabel("Time")
    plt.yticks(fontsize=8)
    plt.yticks(ytick_positions, ytick_labels)


    plt.subplot(1, 2, 2)  # 1 строка, 2 колонки, второй подграфик
    plt.imshow(np.abs(data_max_only), aspect="auto", cmap=cm)  # , interpolation="none")
    plt.colorbar()
    plt.title("Max values with custom cmap")
    plt.ylabel("Frequency")
    plt.xlabel("Time")
    plt.yticks(fontsize=8)
    plt.yticks(ytick_positions, ytick_labels)

    plt.tight_layout()
    plt.show()
