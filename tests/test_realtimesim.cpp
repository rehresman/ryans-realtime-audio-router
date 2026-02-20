#include <catch2/catch_test_macros.hpp>
#include <array>

#include "RealtimeSim.h"

TEST_CASE("RealtimeSim: basic push then pop yields one block of frames", "[realtimesim]") {
    constexpr size_t BS = 128;
    constexpr size_t CAP = 8;

    RealtimeSim<BS, CAP> sim;
    typename RealtimeSim<BS, CAP>::AudioBlock b{};

    auto a = sim.audioStep();
    REQUIRE(a.pushed);
    REQUIRE_FALSE(a.overrun);

    auto w = sim.workerPop(b);
    REQUIRE(w.popped);
    REQUIRE_FALSE(w.underrun);

    REQUIRE(a.producedFrames == BS);
    REQUIRE(b.size() == BS);
    REQUIRE(sim.bufferFill() == 0);
}

TEST_CASE("RealtimeSim: overrun occurs when worker never pops", "[realtimesim]") {
    constexpr size_t BS = 128;
    constexpr size_t CAP = 4;

    RealtimeSim<BS, CAP> sim;

    bool sawOverrun = false;
    // push until failure/overrun
    for (int i = 0; i < 1000; ++i) {
        auto a = sim.audioStep();
        if (!a.pushed || a.overrun) {
            sawOverrun = true;
            break;
        }
    }
    REQUIRE(sawOverrun);
}

TEST_CASE("RealtimeSim: underrun occurs when audio never pushes", "[realtimesim]") {
    constexpr size_t BS = 128;
    constexpr size_t CAP = 4;

    RealtimeSim<BS, CAP> sim;
    typename RealtimeSim<BS, CAP>::AudioBlock b{};

    // pop immediately from empty
    auto w = sim.workerPop(b);
    REQUIRE_FALSE(w.popped);
    REQUIRE(w.underrun);
}

TEST_CASE("RealtimeSim: produced - consumed equals fill*blockSize (invariant)", "[realtimesim]") {
    constexpr size_t BS = 128;
    constexpr size_t CAP = 16;
    constexpr u_int64_t kPush = 5;
    constexpr u_int64_t kPop = 2;

    RealtimeSim<BS, CAP> sim;
    typename RealtimeSim<BS, CAP>::AudioBlock b{};
    RealtimeSim<BS, CAP>::SimStepResult audioResult;
    RealtimeSim<BS, CAP>::SimStepResult workerResult;
    u_int64_t totalProducedFrames = 0;
    u_int64_t totalConsumedFrames = 0;

    // create some fill
    for (int i = 0; i < kPush; ++i) {
        audioResult = sim.audioStep();
        totalProducedFrames += audioResult.producedFrames;
    }
    // pop a couple
    for (int i = 0; i < kPop; ++i){
        workerResult = sim.workerPop(b);
        totalConsumedFrames += workerResult.consumedFrames;
    }


    const auto fill = sim.bufferFill();
    REQUIRE(totalProducedFrames == kPush * BS);
    REQUIRE(totalConsumedFrames == kPop * BS);
    REQUIRE((totalProducedFrames - totalConsumedFrames) == fill * BS);
}
