# ryans-realtime-audio-router

A minimal C++ real-time audio pipeline demonstrating lock-free audio data transfer using a multithreaded approach.

This project is meant to model a realistic real-time audio system architecture similar to telephony, Bluetooth audio, or media playback pipelines.

![Router Output](./plots/Constant%20Frequency%20Sine%20Wave%20Integrity%20Over%2010s.png)
![Router Output](./plots/Low%20Frequency%20Sine%20Sweep%20Integrity%20Over%2010s.png)
[Listen (careful, it's loud)](./audio/test.wav)
---

## Lock-Free Audio Pipleline

### Overview

- Simulated real-time audio producer thread
- Lock-free single producer, single consumer (SPSC) ring buffer
- Worker thread consumes audio blocks and outputs them to a .wav file for analysis
- Simple DSP chain (gain + simple low-pass filter, optional saturation)
- Real-time safe logging of buffer overflow, underrun and more.
- Testable architecture with step API returning diagnostics

No external audio frameworks were used in this deliverable.

## Realtime Architecture
> **Producer Thread**
> * Generates audio blocks using FM synthesis
> * Runs simple DSP (gain + low-pass)
> * Pushes blocks into lock-free ring buffer

↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓

> **SPSC Ring Buffer**
> * Fixed-size, configurable
> * Atomic read/write indices with explicit memory alignment
> * Lock-free, allocation free

↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓

> **Worker Thread**
> * Pops audio blocks from ring buffer
> * Simulates downstream audio consumer, storing audio blocks
> * Tracks overflows/underruns

## OO Architecture

![Class Model](./plots/Class%20Model.png)

> **SimDriver**
> * Orchestrates integration testing of RealtimeSim
> * Launches audio thread, worker thread, and main thread
> * Simulates timing and jitter
> * Outputs diagnostics
> * Writes audio to a .wav file for analysis

> **RealtimeSim**
> * Step API: reads/writes one block of audio, returns diagnostics per-step for debug
> * Realtime safe, lock-free, atomic reading/writing of statistics
> * Synthesizes audio via FM
> * Calls AudioEngine for DSP
> * Pops consumed audio for downstream processing

> **AudioEngine**
> * Minimal DSP engine
> * Gain, filtering, saturation
> * Nothing crazy going on here

> **WavWriter**
> * This is all boilerplate.  It writes the .wav file

> **SpscRingBuffer**
> * Fixed-size, configurable
> * Atomic read/write indices with explicit memory alignment
> * Lock-free, allocation free

---

## Why SPSC + Lock Free

- Worst-case timing is key in real-time audio
- Real-time audio threads cannot block or wait on locks
- Locks can cause priority inversion
- One read thread and one write thread simplifies correctness
- SPSC can minimize contention and false sharing (if done correctly)
- Deterministic, glitch-free behavior is essential regarless of load conditions

## Failure Modes

- **Overflow**: Producer writes faster than consumer reads data, leading to audio data dropped.
![Overflow Simulation](./plots/Overflow%20Simulation%20(Sine%20Wave).png)
- **Underrun**: Consumer reads faster than producer writes data, leading to silence.
![Underrun Simulation](./plots/Underrun%20Simulation%20(Sine%20Wave).png)

Both are instumented and observable.

## DSP

Current DSP chain:
- Gain
- One-pole low-pass filter
- Analog-modeling saturation (commented out)

Filter state is maintained across callbacks to avoid discontinuities due to IIR design.

## Debugging

- Step API designed specifically for easy debugging
- Multiple threads named for easy inspection
- Atomic tracking of diagnostics
- Every block of audio, overrun, and underrun easily tracked

## Build

```bash
cmake -S . -B build
cmake --build build -j
./build/simulation
```

## Python Plotting

- optional
- plotter.py included for quick plot generation of audio output

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

python plotter.py
```
