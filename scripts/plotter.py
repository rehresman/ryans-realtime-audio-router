import matplotlib.pyplot as plt
import numpy as np
from scipy.io import wavfile

_,file = wavfile.read('audio/out.wav')

y = file / np.max(file)
print(file)
x = np.arange(0,len(file))
plt.figure(figsize=(16,8))
plt.plot(x, y)
plt.ylabel('Amplitude')
plt.xlabel('Samples (48k/s)')
plt.title('Constant Frequency Sine Wave Integrity Over 10s')
#plt.savefig("Constant Frequency Sine Wave Integrity Over 10s.png", dpi=300)
plt.show()

