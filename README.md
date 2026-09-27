# EEK!

### A talisman that protects me from talks of marriage by interrupting them

EEK! is a tiny lapel pin that listens to conversations around me and **screams** whenever it hears words like:

> “marriage”  
> “husband”  
> “shaadi”

## How it works

EEK! uses a **XIAO ESP32-S3 Sense** with its onboard microphone to listen to its surroundings.

An audio classification model running directly on the ESP32 detects the trigger sounds. When detected, the ESP32 sends a pre-recorded scream to an external speaker through a MAX98357A amplifier.

```text
        Sound
          │
          ▼
 ┌───────────────────┐
 │ XIAO ESP32-S3     │
 │                   │
 │ Microphone        │
 │       ↓           │
 │ Audio ML model    │
 └────────┬──────────┘
          │
   trigger detected
          │
          ▼
 ┌───────────────────┐
 │ MAX98357A         │
 │ amplifier         │
 └────────┬──────────┘
          │
          ▼
       Speaker
          │
          ▼
        EEK!
```

## Hardware

- Seeed Studio XIAO ESP32-S3 Sense
- MAX98357A audio amplifier
- Speaker
- Battery
- Custom enclosure

## Software

- C++ / Arduino
- Arduino ESP32 core
- Edge Impulse
- I2S audio

## Audio

The repository includes the source audio and the files used to prepare it for the ESP32:

- `scream.wav` — the original scream audio
- `scream.h` — the scream encoded as C data for use in the firmware
- `convert.py` — Python script used to convert the WAV file into C data
