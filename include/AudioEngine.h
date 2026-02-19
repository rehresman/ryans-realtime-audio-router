#pragma once
#include <cstddef>
#include <array>
#include <span>

class AudioEngine {
    public:
        AudioEngine(float gain=0.25f);

        void setGain(float g);

        void process(std::span<const float> input, std::span<float> output, int sampleRate);

    private:
        
        void amplify(std::span<const float> input,  std::span<float> output);

        void onePole(std::span<const float> input,  std::span<float> output, const std::array<float,2>& coefficients);

        void saturate(std::span<const float> input,  std::span<float> output);
        
        float gain_;

        float onePoleState_ = 0.0f;
};
