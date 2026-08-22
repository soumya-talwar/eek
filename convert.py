import numpy as np
from scipy.io import wavfile

# Read audio file
sample_rate, data = wavfile.read("scream.wav")

# Convert stereo to mono
if len(data.shape) > 1:
    data = data.mean(axis=1)

# Amplify/Normalize to full 16-bit signed scale (-32767 to 32767)
max_val = np.max(np.abs(data))
if max_val > 0:
    data = (data / max_val) * 32767
data = data.astype(np.int16)

# Write to scream.h
with open("scream.h", "w") as f:
    f.write("#ifndef SCREAM_H\n#define SCREAM_H\n\n")
    f.write("#include <Arduino.h>\n#include <pgmspace.h>\n\n")
    f.write(f"// Original Sample Rate: {sample_rate} Hz\n")
    f.write(f"const int16_t scream[] PROGMEM = {{\n  ")
    
    for i, sample in enumerate(data):
        f.write(f"{sample}, ")
        if (i + 1) % 12 == 0:
            f.write("\n  ")
            
    f.write("\n};\n\n#endif\n")

print(f"Done! Re-generated scream.h (Detected Sample Rate: {sample_rate} Hz)")