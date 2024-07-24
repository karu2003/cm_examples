import numpy as np
import matplotlib.pyplot as plt
import os
from matplotlib.colors import LinearSegmentedColormap


def threshold_2Darray(in_array, threshold):
    thr = np.amax(in_array) * threshold
    in_array[in_array < thr] = 0
    return in_array


# def normalize_data(data):
#     data_min = np.min(data)
#     data_max = np.max(data)
#     return (data - data_min) / (data_max - data_min)

def normalize_data(data):
    # Заменяем NaN на 0
    data = np.nan_to_num(data, nan=0.0)
    data_min = np.min(data)
    data_max = np.max(data)
    return (data - data_min) / (data_max - data_min)


# def normalize_data(data):
#     data_min = np.min(data)
#     data_max = np.max(data)
#     return 2 * (data - data_min) / (data_max - data_min) - 1


current_path = os.getcwd()
directory_path = "conv_cwt/"
file = current_path + "/" + directory_path + "cwt.txt"

with open(file, "r") as fl:
    first_line = fl.readline().strip()
    params = first_line.split()
    n, fn, f0, f1 = map(float, params)

num_frequencies = int(2 * int(n) - 1)

data = np.loadtxt(file, dtype=np.float32, skiprows=1)

# data = (data - data.min()) / (data.max() - data.min())
# print(data)

data = normalize_data(data)

# print(data)


if data.size % num_frequencies != 0:
    raise ValueError(f"Размер данных должен делиться на {num_frequencies} без остатка")

print(f"Signal Options: n={n}, fn={fn}, f0={f0}, f1={f1}")
data_complex = data.reshape((num_frequencies, -1), order="F")
print(data_complex.shape)

data_complex_transposed = np.transpose(data_complex)

print(data_complex_transposed.shape)

data_complex_transposed = threshold_2Darray(data_complex_transposed, 0.3)

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
