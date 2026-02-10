#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <random>
#include <cassert>

#include "AudioEngine.h"
#include "SpscRingBuffer.h"
#include "WavWriter.h"

struct Stats {
    std::atomic<uint64_t> produced{0};
    std::atomic<uint64_t> consumed{0};
    std::atomic<uint64_t> overruns{0};
    std::atomic<uint64_t> underruns{0};
};

int main() {
    constexpr int sampleRate = 48000;
    constexpr size_t blockSize = 128;
    constexpr double callbackMs = 1000.0 * blockSize / sampleRate;
    constexpr double latencyMs = 15.0;
    constexpr double recTime = 10.0;  // recording time in seconds
    constexpr bool simulateOverflow = false;
    constexpr bool simulateUnderrun = false;
    constexpr float errFactor = 2.5; // must be less than callbackMs 

    struct Block {std::array<float, blockSize> x;};

    SpscRingBuffer<Block, 256> rb;
    AudioEngine engine(0.25f);
    Stats stats;
    std::atomic<bool> running{true};
    std::vector<float> recorded;
    recorded.reserve(sampleRate*recTime);

    // rng for overflow/underrun simulation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1,10);

    assert(errFactor < callbackMs && "errFactor must be less than callbackMs");

    // audio callback sim
    std::thread audioThread([&](){
        pthread_setname_np("audio-thread");
        using clock = std::chrono::steady_clock;
        auto next = clock::now();

        double phase = 0.0;
        const double freq = 55.0;
        double fmPhase = 0.0;
        const double fmFreq = 0.15;
        const double fmAmt = 4.0;
        const double phaseInc = 2.0 * M_PI * freq / sampleRate;
        const double fmPhaseInc = 2.0 * M_PI * fmFreq / sampleRate;
        double fm = 0;

        float in[blockSize];
        float out[blockSize];

        const int blockN = static_cast<int>(blockSize);
        while (running.load(std::memory_order_relaxed)) {
            // generate timing
            if (simulateOverflow && simulateUnderrun){
                next += std::chrono::duration_cast<clock::duration>(
                    std::chrono::duration<double, 
                    std::milli>(callbackMs-(errFactor/distrib(gen)) + (errFactor/distrib(gen))));
            } else if (simulateOverflow){
                next += std::chrono::duration_cast<clock::duration>(
                    std::chrono::duration<double, std::milli>(callbackMs-errFactor/distrib(gen)));
            } else if (simulateUnderrun){
                next += std::chrono::duration_cast<clock::duration>(
                    std::chrono::duration<double, std::milli>(callbackMs+errFactor/distrib(gen)));
            }
            else {
            next += std::chrono::duration_cast<clock::duration>(
                std::chrono::duration<double, std::milli>(callbackMs));
            }

            //create some test audio
            for (int i = 0; i < blockN; ++i){
                fm = (1 - std::cos(fmPhase)) * pow(2,fmAmt);
                in[i] = static_cast<float>(std::sin(phase));
                fmPhase += fmPhaseInc;
                phase = phase + (phaseInc * fm);
                if (phase > 2.0 * M_PI){
                    phase -= 2.0 * M_PI;
                }
                if (fmPhase > 2.0 * M_PI){
                    fmPhase -= 2.0 * M_PI;
                }
            }

            engine.process(in, out, blockSize, sampleRate);

            Block b{};
            for (int i = 0; i < blockN; ++i){
                b.x[i] = out[i];
            }

            if (!rb.push(b)) {
                stats.overruns.fetch_add(1, std::memory_order_relaxed);
            }
            else {
                stats.produced.fetch_add(blockSize, std::memory_order_relaxed);
            }

            std::this_thread::sleep_until(next);
        }
    });

    // worker thread
    std::thread workerThread([&](){
        pthread_setname_np("worker-thread");
        using clock = std::chrono::steady_clock;
        using namespace std::chrono_literals;

        auto next = clock::now();
        Block b{};
        const int blockN = static_cast<int>(blockSize);

        // initial latency
        next += std::chrono::duration_cast<clock::duration>(
            std::chrono::duration<double, std::milli>(latencyMs));
        std::this_thread::sleep_until(next);

        while (running.load(std::memory_order_relaxed)) {
            next += std::chrono::duration_cast<clock::duration>(
                std::chrono::duration<double, std::milli>(callbackMs));
            
            if (!rb.pop(b)){
                stats.underruns.fetch_add(1, std::memory_order_relaxed);
                // output silence in a real system
            }
            else {
                stats.consumed.fetch_add(blockSize, std::memory_order_relaxed);

                //append to recording
                for (int i = 0; i< blockN; ++i){
                    recorded.push_back(b.x[i]);
                }
            }
            std::this_thread::sleep_until(next);
            
            // update consume block to calculate something like peak and RMS values
        }
    });

    // Main thread: print stats for a few seconds
    for (int i=0; i < 6; ++i){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto prod = stats.produced.load();
        auto cons = stats.consumed.load();
        auto over = stats.overruns.load();
        auto under = stats.underruns.load();
        auto fill = rb.size_approx();

        std::cout
        << "produced_frames=" << prod
        << " consumed_frames=" << cons
            << " overruns=" << over
            << " underruns=" << under
            << " blocks_in_buffer≈" << fill
            << "\n";
    }

    running.store(false);
    audioThread.join();
    workerThread.join();

    if (WavWriter16Mono::write("audio/out.wav", recorded, static_cast<uint32_t>(sampleRate))) {
        std::cout << "Wrote out.wav (" << recorded.size() << " samples)\n";
    } else {
        std::cout << "Failed to write out.wav\n";
    }
    

    std::cout << "Done.\n";
    return 0;
}