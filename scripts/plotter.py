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
plt.title('Underrun Simulation (Sine Wave)')
#plt.savefig("Underrun Simulation (Sine Wave).png", dpi=300)
plt.show()

