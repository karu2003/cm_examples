import numpy as np
import matplotlib.pyplot as plt
import os

current_path = os.getcwd()

directory_path = "arm_fCWT/"
file = current_path + "/" + directory_path + "wavelet.txt"

data = np.loadtxt(file)

plt.plot(data)
plt.title('Wavelet')
plt.xlabel('Sample Index')
plt.ylabel('Amplitude')
plt.show()