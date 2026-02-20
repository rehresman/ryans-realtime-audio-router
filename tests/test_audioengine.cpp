#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cmath>
#include <span>

#include "AudioEngine.h"

static bool isFiniteArray(std::span<const float> x) {
    for (float v : x) {
        if (!std::isfinite(v)) return false;
    }
    return true;
}

TEST_CASE("AudioEngine: process does not produce NaNs/Infs", "[audioengine]") {
    AudioEngine eng;
    std::array<float, 128> in{};
    std::array<float, 128> out{};
    const int sampleRate = 48000;

    // 1Hz sine wave
    for (size_t i = 0; i < in.size(); ++i) {
        in[i] = sin(i*2*M_PI/sampleRate);
    }

    eng.process(in, out, sampleRate);

    REQUIRE(isFiniteArray(out));
}

TEST_CASE("AudioEngine: silence in -> silence out", "[audioengine]") {
    AudioEngine eng;

    std::array<float, 128> in{};
    std::array<float, 128> out{};

    eng.process(in, out, 48000);

    float maxAbs = 0.0f;
    for (float v : out) maxAbs = std::max(maxAbs, std::fabs(v));

    REQUIRE(maxAbs < 1e-6f);
}

TEST_CASE("AudioEngine: impulse response is bounded", "[audioengine]") {
    AudioEngine eng;

    std::array<float, 128> in{};
    std::array<float, 128> out{};

    in[0] = 1.0f;

    eng.process(in, out, 48000);

    REQUIRE(isFiniteArray(out));

    float maxAbs = 0.0f;
    for (float v : out) maxAbs = std::max(maxAbs, std::fabs(v));

    REQUIRE(maxAbs < 1.0f);
}
