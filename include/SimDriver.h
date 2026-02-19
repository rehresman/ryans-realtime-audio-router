#include "RealtimeSim.h"
#include <random>
#include <vector>
#include <chrono>
#include <atomic>

using Sim = RealtimeSim<128,256>;
using StepResult = Sim::SimStepResult;
using AudioBlock = Sim::AudioBlock;
using myClock = std::chrono::steady_clock;

class SimDriver {
    public:
        SimDriver(int sampleRate, int runDuration, 
            bool simulateOverflow, bool simulateUnderrun);
        void run();
        StepResult timedAudioStep();
        StepResult timedWorkerStep();
        void initAudioTiming();
        void simulateInitialWorkerLatency();
        uint64_t totalProducedFrames() const;
        uint64_t totalConsumedFrames() const;
        uint64_t totalOverruns() const;
        uint64_t totalUnderruns() const;
        uint64_t bufferFill() const;

    private:
        RealtimeSim<128,256> sim_;
        const double callbackMs_;
        const double latencyMs_ = 15.0;
        const float errFactor_ = 2.5f; // must be less than callbackMs_
        std::mt19937 rng_{1991};
        std::uniform_int_distribution<int> distrib_{1, 10};
        std::bernoulli_distribution signDistrib_;
        std::atomic<bool> running_{false};
        const bool simulateOverflow_;
        const bool simulateUnderrun_;
        const int sampleRate_;
        const int runDuration_;
        myClock::time_point nextAudioStep_;
        myClock::time_point nextWorkerStep_;

        alignas(64) std::atomic<uint64_t> totalProducedFrames_{0};
        alignas(64) std::atomic<uint64_t> totalConsumedFrames_{0};
        alignas(64) std::atomic<uint64_t> totalOverruns_{0};
        alignas(64) std::atomic<uint64_t> totalUnderruns_{0};


        // sound output
        std::vector<float> recorded_;
};