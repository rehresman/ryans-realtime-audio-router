#include "SimDriver.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <pthread.h>
#include "WavWriter.h"


SimDriver::SimDriver(int sampleRate, int runDuration, 
    bool simulateOverflow, bool simulateUnderrun)
    : sampleRate_(sampleRate), 
    runDuration_(runDuration),
    callbackMs_(1000.0 * double(Sim::kBlockSize) / double(sampleRate)),
    simulateOverflow_(simulateOverflow), 
    simulateUnderrun_(simulateUnderrun),
    sim_(sampleRate)
    {
        recorded_.reserve(size_t(sampleRate_)*size_t(runDuration_*2));
    }


StepResult SimDriver::timedAudioStep(){
    // perform audio step, then sleep as long as necessary for simulated delay
    auto r = StepResult{};
    myClock::time_point now;

    int randSign = signDistrib_(rng_) ? 1 : -1;

    if (simulateOverflow_ && simulateUnderrun_){
        nextAudioStep_ += std::chrono::duration_cast<myClock::duration>(
            std::chrono::duration<double, 
            std::milli>(callbackMs_+randSign*(errFactor_/distrib_(rng_))));
    } else if (simulateOverflow_){
        nextAudioStep_ += std::chrono::duration_cast<myClock::duration>(
            std::chrono::duration<double, std::milli>(callbackMs_-errFactor_/distrib_(rng_)));
    } else if (simulateUnderrun_){
        nextAudioStep_ += std::chrono::duration_cast<myClock::duration>(
            std::chrono::duration<double, std::milli>(callbackMs_+errFactor_/distrib_(rng_)));
    }
    else {
    nextAudioStep_ += std::chrono::duration_cast<myClock::duration>(
        std::chrono::duration<double, std::milli>(callbackMs_));
    }

    r = sim_.audioStep();
    // log results
    if (r.pushed){
        totalProducedFrames_.fetch_add(r.producedFrames, std::memory_order_relaxed);
    }
    if (r.overrun){
        totalOverruns_.fetch_add(r.overruns, std::memory_order_relaxed);
    }

    now = myClock::now();
    if (nextAudioStep_ < now) nextAudioStep_ = now;
    std::this_thread::sleep_until(nextAudioStep_);

    return r;
}

void SimDriver::initAudioTiming(){
    nextAudioStep_ = myClock::now();
    nextWorkerStep_ = nextAudioStep_ + std::chrono::duration_cast<myClock::duration>(
        std::chrono::duration<double, std::milli>(latencyMs_));
}

void SimDriver::simulateInitialWorkerLatency(){
    // initial latency
    std::this_thread::sleep_until(nextWorkerStep_);
}

StepResult SimDriver::timedWorkerStep(){
    using namespace std::chrono_literals;
    auto r = StepResult{};
    AudioBlock b{};
    myClock::time_point now;
    const int blockN = static_cast<int>(Sim::kBlockSize);

    nextWorkerStep_ += std::chrono::duration_cast<myClock::duration>(
        std::chrono::duration<double, std::milli>(callbackMs_));
    
    r = sim_.workerPop(b);
    //append to recording
    for (int i = 0; i< blockN; ++i){
        recorded_.push_back(b[i]);
    }

    // log results
    if (r.popped){
        totalConsumedFrames_.fetch_add(r.consumedFrames, std::memory_order_relaxed);
    }
    if (r.underrun){
        totalUnderruns_.fetch_add(r.underruns, std::memory_order_relaxed);
    }

    now = myClock::now();
    if (nextWorkerStep_ < now) nextWorkerStep_ = now;
    std::this_thread::sleep_until(nextWorkerStep_);
    return r;
}

void SimDriver::run(){
    running_.store(true, std::memory_order_relaxed);
    initAudioTiming();

    // audio callback sim
    std::thread audioThread([&](){
        pthread_setname_np("audio-thread");
        StepResult audioResult;
        while (running_.load(std::memory_order_relaxed)) {
            // TODO: do something with the results
            audioResult = timedAudioStep();
        }
    });

    // worker thread
    std::thread workerThread([&](){
        pthread_setname_np("worker-thread");
        StepResult workerResult;
        simulateInitialWorkerLatency();
        while (running_.load(std::memory_order_relaxed)) {
            //TODO: do something with the results
            workerResult = timedWorkerStep();
        }
    });

    // Main thread: print stats for a few seconds
    for (int i=0; i < runDuration_; ++i){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto prod = totalProducedFrames();
        auto cons = totalConsumedFrames();
        auto over = totalOverruns();
        auto under = totalUnderruns();
        auto fill = bufferFill();

        std::cout
        << "produced_frames=" << prod 
        << " consumed_frames=" << cons 
            << " overruns=" << over 
            << " underruns=" << under 
            << " blocks_in_buffer≈" << fill
            << "\n";
    }
    running_.store(false, std::memory_order_relaxed);
    audioThread.join();
    workerThread.join();

    if (WavWriter16Mono::write("audio/out.wav", recorded_, static_cast<uint32_t>(sampleRate_))) {
        std::cout << "Wrote out.wav (" << recorded_.size() << " samples)\n";
    } else {
        std::cout << "Failed to write out.wav\n";
    }
    
    
    std::cout << "Simulation Complete.\n";
}

uint64_t SimDriver::totalProducedFrames() const {
    return totalProducedFrames_.load(std::memory_order_relaxed);
}

uint64_t SimDriver::totalConsumedFrames() const {
    return totalConsumedFrames_.load(std::memory_order_relaxed);
}

uint64_t SimDriver::totalOverruns() const {
    return totalOverruns_.load(std::memory_order_relaxed);
}

uint64_t SimDriver::totalUnderruns() const {
    return totalUnderruns_.load(std::memory_order_relaxed);
}

uint64_t SimDriver::bufferFill() const {
    return sim_.bufferFill();
}