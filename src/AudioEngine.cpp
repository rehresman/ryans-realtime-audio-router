#include "AudioEngine.h"
#include <cmath>

AudioEngine::AudioEngine(float gain) : gain_(gain) {}

void AudioEngine::setGain(float g){
    gain_ = g;
}

void AudioEngine::process(std::span<const float> input,  std::span<float> output, int sampleRate) {
    // TODO: cleanup
    const int numFrames = input.size();
    const int blockN = static_cast<int>(numFrames);
    const float Fc = 12000.0;
    const float filterCoeff = std::exp(-2.0 * M_PI * Fc / sampleRate);
    const std::array<float, 2> fCoefs {1-filterCoeff, filterCoeff};

    amplify(input, output);
    onePole(output, output, fCoefs);
    //saturate(output, output, blockN);
    
}

void AudioEngine::amplify(std::span<const float> input,  std::span<float> output){
    const int numFrames = input.size();
    for (int i = 0; i < numFrames; ++i){
        output[i] = input[i] * gain_;
    }
}

void AudioEngine::onePole(std::span<const float> input,  std::span<float> output, 
                            const std::array<float,2>& coefficients){
    const int numFrames = input.size();
    float a = coefficients[0];
    float b = coefficients[1];

    output[0] = a*input[0] + b*onePoleState_;
    for (int i = 1; i < numFrames; ++i){
        output[i] = a*input[i] + b*output[i-1];
    }
    onePoleState_ = output[numFrames-1];
}

// just for fun
void AudioEngine::saturate(std::span<const float> input,  std::span<float> output){
    const int numFrames = input.size();
    const float preOffset = -0.6;
    const float postOffset = preOffset * (27 + preOffset*preOffset) / (27 + 9 * preOffset*preOffset);
    const float gain = 0.25f;

    for (int i = 0; i < numFrames; ++i){
        const float x = (gain*input[i] + preOffset);
        output[i] = ((x * (27 + x*x) /  (27 + 9 * x*x)) - postOffset)/gain;
    }
}