#include "AudioEngine.h"
#include <cmath>
#include <array>

AudioEngine::AudioEngine(float gain) : gain_(gain) {}

void AudioEngine::setGain(float g){
    gain_ = g;
}

void AudioEngine::process(const float* input, float* output, size_t numFrames, int sampleRate) {
    // TODO: cleanup
    const int blockN = static_cast<int>(numFrames);
    const float Fc = 12000.0;
    const float filterCoeff = std::exp(-2.0 * M_PI * Fc / sampleRate);
    const std::array<float, 2> fCoefs {1-filterCoeff, filterCoeff};

    amplify(input, output, blockN);
    onePole(output, output, blockN, fCoefs);
    //saturate(output, output, blockN);
    
}

void AudioEngine::amplify(const float* input, float* output, int numFrames){
    for (int i = 0; i < numFrames; ++i){
        output[i] = input[i] * gain_;
    }
}

void AudioEngine::onePole(const float* input, float* output, int numFrames, 
                            const std::array<float,2>& coefficients){
    float a = coefficients[0];
    float b = coefficients[1];

    output[0] = a*input[0] + b*onePoleState_;
    for (int i = 1; i < numFrames; ++i){
        output[i] = a*input[i] + b*output[i-1];
    }
    onePoleState_ = output[numFrames-1];
}

// just for fun
void AudioEngine::saturate(const float* input, float* output, int numFrames){
    const float preOffset = -0.6;
    const float postOffset = preOffset * (27 + preOffset*preOffset) / (27 + 9 * preOffset*preOffset);
    const float gain = 0.25f;

    for (int i = 0; i < numFrames; ++i){
        const float x = (gain*input[i] + preOffset);
        output[i] = ((x * (27 + x*x) /  (27 + 9 * x*x)) - postOffset)/gain;
    }
}