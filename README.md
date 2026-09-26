# EEK!

**A talisman that protects you from talks of marriage by interrupting them.**

EEK! is a tiny lapel pin that listens to conversations around you and **screams** whenever it hears words like:

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

The detection happens **entirely on-device**, without sending conversations to a cloud service.

## Hardware

- **Seeed Studio XIAO ESP32-S3 Sense** — processing + microphone
- **MAX98357A** — I2S audio amplifier
- **1.5" 4Ω / 3W speaker**
- Battery / portable power
- Custom enclosure

## Machine Learning

The audio classifier was built using **Edge Impulse** and deployed to the ESP32.

The model is trained to recognise the sounds associated with EEK!'s trigger class rather than transcribing conversations into text.

When the model detects a trigger with sufficient confidence, the talisman plays its scream.

## Software

EEK! is built with:

- **C++ / Arduino**
- **ESP32-S3**
- **Edge Impulse**
- **I2S audio**

The scream is stored locally on the device, so the complete interaction happens offline.

## Audio

The repository includes the source audio and the files used to prepare it for the ESP32:

- `scream.wav` — the original scream audio
- `scream.h` — the scream encoded as C data for use in the firmware
- `convert.py` — Python script used to convert the WAV file into C data

## The result

A small, standalone object that combines **embedded machine learning, real-time audio classification and physical interaction** into one very specific purpose:

_protecting its wearer from marriage talk._
