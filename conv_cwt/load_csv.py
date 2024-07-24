import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os

current_path = os.getcwd()
directory_path = "conv_cwt/"
file = current_path + "/" + directory_path + "cwt_csv.txt"

# Чтение данных из CSV файла
data = pd.read_csv(file)

count_scale_0 = (data['Scale'] == 0).sum()
print(f"Количество строк со значением Scale 0: {count_scale_0}")


value_array = data['Value'].to_numpy()

data_complex = value_array.reshape((count_scale_0, -1), order="F")
data_complex = np.transpose(data_complex)

print(data_complex.shape)

plt.figure(figsize=(10, 6))
plt.imshow(data_complex, aspect='auto', cmap='viridis')
plt.colorbar()
plt.title('Data Complex Transposed as Image')
plt.xlabel('Index')
plt.ylabel('Value')
plt.show()

# plt.plot(value_array)
# plt.show()

# data_complex_transposed = np.transpose(data_complex)


