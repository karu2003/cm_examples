import matplotlib.pyplot as plt
import numpy as np

audio_data = [11.0, 3.0, 4.05, 9.0, 10.3, 8.0, 4.934, 5.11]
# print(np.fft.rfft(audio_data))
print(np.fft.fft(audio_data))      
dfft = 10.*np.log10(abs(np.fft.rfft(audio_data)))
# dfft = 10.*np.log10(abs(np.fft.fft(audio_data)))

plt.subplot(211)
plt.plot(np.arange(len(audio_data)),audio_data)
plt.subplot(212)
plt.plot(np.arange(len(dfft))*10.,dfft)
plt.show()