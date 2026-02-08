import matplotlib.pyplot as plt
import numpy as np
from scipy.io import wavfile

_,file = wavfile.read('out.wav')

y = file / np.max(file)

x = np.arange(0,len(file))
plt.figure(figsize=(16,8))
plt.plot(x, y)
plt.ylabel('Amplitude')
plt.xlabel('Samples (48k/s)')
plt.title('Low Frequency Sine Sweep Integrity Over 10s')
plt.savefig("Low Frequency Sine Sweep Integrity Over 10s.png", dpi=300)
plt.show()

