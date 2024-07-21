import numpy as np
import matplotlib.pyplot as plt
import os
from matplotlib.colors import LinearSegmentedColormap


def threshold_2Darray(in_array, threshold):
    thr = np.amax(in_array) * threshold
    in_array[in_array < thr] = 0
    return in_array


current_path = os.getcwd()
directory_path = "arm_fCWT/"
file = current_path + "/" + directory_path + "tfm.txt"

with open(file, "r") as fl:
    first_line = fl.readline().strip()
    params = first_line.split()
    n, fn, f0, f1 = map(float, params)

data = np.loadtxt(
    file,
    dtype=np.complex64,
    skiprows=1,
    converters={
        0: lambda s: complex(
            float(s.decode().split(",")[0]), float(s.decode().split(",")[1])
        )
    },
)

print(f"Signal Options: n={n}, fn={fn}, f0={f0}, f1={f1}")
data_complex = data.reshape((int(n), -1), order="F")

data_complex_transposed = np.transpose(data_complex)

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
