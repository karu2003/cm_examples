import numpy as np
import matplotlib.pyplot as plt

import os.path

filename = 'test_signal.h'

# __location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
# filename = __location__+'/'+filename

sample_rate = 96000.
f0 = 700.0
f1 = 7000.0
Pi2 = 2*np.pi
duration = 0.001
N = int(sample_rate * duration)

t = np.linspace(0, duration, N)
beta = (f1-f0)/duration
phase = Pi2*(f0*t+0.5*beta*t*t)
chirp_signal = np.sin(phase)

# plt.plot(chirp_signal)
# plt.show()

output_file = '# define NSAMP ' + str(N) + '\n'
output_file += '# define FSAMP ' + str(sample_rate) + '\n'
output_file += 'float pattern1[NSAMP] = {'
for i in range(N):
    output_file += str(chirp_signal[i]) 
    if i != N-1:
        output_file += ','
output_file += '};'

fout = open(filename, 'w')

fout.writelines(output_file)
fout.close()

# dfft = 10.*np.log10(abs(np.fft.rfft(chirp_signal)))
# f_res = sample_rate / N
# plt.plot(np.arange(len(dfft))*f_res,dfft)
# plt.show() 
