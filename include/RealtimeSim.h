#pragma once
#include <cstddef>
#include <cstdint>
#include <array>
#include <atomic>
#include <cmath>

#include "AudioEngine.h"
#include "SpscRingBuffer.h"

template<size_t BlockSize = 128, size_t RingCapacityBlocks = 256>
class RealtimeSim {
    public:
        using AudioBlock = std::array<float, BlockSize>;
        
        static constexpr size_t kBlockSize = BlockSize;

        struct SimStepResult {
            bool pushed = false;
            bool popped = false;
            bool overrun = false;
            bool underrun = false;

            uint64_t producedFrames = 0;
            uint64_t consumedFrames = 0;
            uint64_t overruns = 0;
            uint64_t underruns = 0;
            size_t bufferFill = 0;
        };


        RealtimeSim(int sampleRate = 48000, float gain = 0.25f)
            : sampleRate_(sampleRate), 
            gain_(gain), 
            engine_(gain),
            phaseInc_(2.0 * M_PI * freq_ / double(sampleRate_)),
            fmPhaseInc_(2.0 * M_PI * fmFreq_ / double(sampleRate_))   
        {};

        void synthesizeAudio(int blockN){
            double fm;
            double fmAmtPow = pow(2,fmAmt_);
            for (int i = 0; i < blockN; ++i){
                fm = (1 - std::cos(fmPhase_)) * fmAmtPow;
                in_[i] = static_cast<float>(std::sin(phase_));
                fmPhase_ += fmPhaseInc_;
                phase_ = phase_ + (phaseInc_ * fm);
                if (phase_ > 2.0 * M_PI){
                    phase_ -= 2.0 * M_PI;
                }
                if (fmPhase_ > 2.0 * M_PI){
                    fmPhase_ -= 2.0 * M_PI;
                }
            }
        };
        

        // Realtime Stepping API
        SimStepResult audioStep(){           
            const int blockN = static_cast<int>(BlockSize);
            AudioBlock b{};
            auto r = SimStepResult{};

            //create audio
            synthesizeAudio(blockN);
            //process DSP
            engine_.process(in_, out_, sampleRate_);
            //package into an AudioBlock
            for (int i = 0; i < blockN; ++i){
                b[i] = out_[i];
            }

            if (!rb_.push(b)) {
                r.overrun = true;
                r.overruns = 1;
            }
            else {
                r.pushed = true;
                r.producedFrames = BlockSize;
            }

            r.bufferFill = rb_.size_approx();
            return r;
        };

        // Realtime Stepping API
        SimStepResult workerPop(AudioBlock& b){
            const int blockN = static_cast<int>(BlockSize);
            auto r = SimStepResult{};

            if (!rb_.pop(b)){
                r.underrun = true;
                r.underruns = 1;
                b.fill(0);
            }
            else {
                r.popped = true;
                r.consumedFrames = BlockSize;
            }
            return r;
        };
        
        uint64_t bufferFill() const {
            return rb_.size_approx();
        }
    
    private:
        // config
        const int sampleRate_;
        const float gain_;

        // core machinery
        SpscRingBuffer<AudioBlock, RingCapacityBlocks> rb_;
        AudioEngine engine_;

        // sound generation
        const double freq_ = 55.0;
        const double fmFreq_ = 0.15;
        const double fmAmt_  = 4.0;
        double phase_ = 0.0;
        double fmPhase_ = 0.0;
        double phaseInc_ = 0.0;
        double fmPhaseInc_ = 0.0;

        // audio containers
        std::array<float, BlockSize> in_{};
        std::array<float, BlockSize> out_{};

};