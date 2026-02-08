# ryans-realtime-audio-router

A minimal C++ real-time audio pipeline demonstrating lock-free audio data transfer using a multithreaded approach.

This project is meant to model a realistic real-time audio system architecture similar to telephony, Bluetooth audio, or media playback pipelines.

![Router Output](./plots/Constant%20Frequency%20Sine%20Wave%20Integrity%20Over%2010s.png)
![Router Output](./plots/Low%20Frequency%20Sine%20Sweep%20Integrity%20Over%2010s.png)
[Listen (careful, it's loud)](./audio/test.wav)
---

## Deliverable 1: Lock-Free Audio Pipleline

### Overview

- Simulated real-time audio producer thread
- Lock-free single producer, single consumer (SPSC) ring buffer
- Worker thread consumes audio blocks and outputs them to a .wav file for analysis
- Simple DSP chain (gain + simple low-pass filter)
- Real-time safe logging of buffer overflow, underrun and more.

No external audio frameworks were used in this deliverable.

## Architecture
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

---

## Why SPSC + Lock Free

- Performance is key in real-time audio
- Real-time audio threads cannot block or wait on locks
- One read thread and one write thread simplifies design
- Multiple producers or consumers cause locking, increasing worst-case timing scenarios
- Lock-free patterns eliminate cache-invalidation issues
- Deterministic behavior is essential for audio callbacks

## Failure Modes

- **Overflow**: Producer writes faster than consumer reads data, leading to audio data dropped.
- **Underrun**: Consumer reads faster than producer writes data, leading to silence.

Both are instumented and observable.

## DSP

Current DSP chain:
- Gain
- One-pole low-pass filter (commented out)

Filter state is maintained across callbacks to avoid discontinuities due to IIR design.

## Debugging

- Built and debugged with LLDB
- Multiple threads named for easy inspection
- Verified atomic synchronization
- Confirmed real-time-safe behavior (no allocation, no locks)

## Build

```bash
cmake -S . -B build
cmake --build build -j
./build/ryans-realtime-audio-router
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
