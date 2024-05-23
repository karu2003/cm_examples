import numpy as np

from scipy.signal import chirp, spectrogram

import matplotlib.pyplot as plt

t = np.linspace(0, 10, 1500)

w = chirp(t, f0=6, f1=1, t1=11, method='linear', phi =90, vertex_zero = False)

plt.plot(t, w)

plt.title("Linear Chirp, f(0)=6, f(10)=1")

plt.xlabel('t (sec)')

plt.show()