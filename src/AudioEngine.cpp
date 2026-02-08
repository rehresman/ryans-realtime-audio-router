#include "AudioEngine.h"
#include <cmath>
#include <array>

AudioEngine::AudioEngine(float gain) : gain_(gain) {}

void AudioEngine::setGain(float g){
    gain_ = g;
}

void AudioEngine::process(const float* input, float* output, size_t numFrames, size_t sampleRate) {

    size_t Fc = 12000;
    float filterCoeff = std::exp(-2.0 * M_PI * Fc / sampleRate);
    std::array<float, 2> fCoefs {1-filterCoeff, filterCoeff};

    amplify(input, output, numFrames);
    onePole(output, output, numFrames, fCoefs);
    
}

void AudioEngine::amplify(const float* input, float* output, size_t numFrames){
    for (size_t i = 0; i < numFrames; ++i){
        output[i] = input[i] * gain_;
    }
}

void AudioEngine::onePole(const float* input, float* output, size_t numFrames, const std::array<float,2>& coefficients){
    float a = coefficients[0];
    float b = coefficients[1];

    output[0] = onePoleState_;
    for (size_t i = 1; i < numFrames; ++i){
        output[i] = a*input[i] + b*output[i-1];
    }
    onePoleState_ = output[numFrames-1];
}

