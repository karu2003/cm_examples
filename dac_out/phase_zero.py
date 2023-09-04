# https://x410.dev/cookbook/enabling-ssh-x11-forwarding-in-visual-studio-code-for-remote-development/
import numpy as np
import matplotlib.pyplot as plt
import math

sample_rate = 50000.
dT = 1/sample_rate
duration = 0.01
N = int(sample_rate * duration)
f0 = 700.0
f1 = 5500.0
# f1 = 3550.0
Pi2 = 2*np.pi

t = np.linspace(0, duration, N)
wwww = np.zeros(N)
sweep = np.linspace(f0, f1, N)
wwww1 = np.zeros(N)

w0 = f0*Pi2
w1 = f1*Pi2
wF = sample_rate * Pi2
print("w1",w1)
print("wF",wF)

beta = (f1-f0)/duration
phase = Pi2*(f0*t+0.5*beta*t*t)
print("phase",phase) 

for i, a in enumerate(t):
    wwww[i] = w0*a+(w1-w0)*(a*a/2*N)

for i, a in enumerate(sweep):
    wwww1[i] = a * Pi2

result = zip(wwww,wwww1)
result = set(result)

print("w1-w0",(w1-w0)/Pi2)

# mod = np.fmod((w1-w0),Pi2)
# print("mod",mod)

dW = (w1-w0)/N


dN = (N/2)*(w1+w0)
dN = (w1+w0)
print("dN",dN)
dClean = Pi2*round(dN/Pi2)
wClean = (dClean/N)-w0

print("wClean",wClean)
print("freq",wClean/Pi2)

for i, a in enumerate(t):
    wwww1[i] = w0*a+(wClean-w0)*(a*a/2*N)

# for i, a in enumerate(t):
#     wwww1[i] = w0*a+B*a



plt.subplot(211)
plt.plot(np.sin(wwww))
plt.subplot(212)
plt.plot(np.sin(phase))
plt.show()
