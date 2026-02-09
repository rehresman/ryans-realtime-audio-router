#pragma once
#include <cstddef>
#include <array>

class AudioEngine {
    public:
        AudioEngine(float gain=0.25f);

        void setGain(float g);

        void process(const float* input, float* output, size_t numFrames, int sampleRate);

    private:
        
        void amplify(const float* input, float* output, int numFrames);

        void onePole(const float* input, float* output, int numFrames, const std::array<float,2>& coefficients);

        void saturate(const float* input, float* output, int numFrames);
        
        float gain_;

        float onePoleState_ = 0.0f;
};
