#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdint>

#include "SpscRingBuffer.h"

TEST_CASE("SpscRingBuffer: single-thread push/pop preserves order", "[ringbuffer]") {
    constexpr size_t kCap = 8;
    SpscRingBuffer<int, kCap> rb;

    // push a few
    REQUIRE(rb.push(1));
    REQUIRE(rb.push(2));
    REQUIRE(rb.push(3));

    int x = 0;
    REQUIRE(rb.pop(x)); REQUIRE(x == 1);
    REQUIRE(rb.pop(x)); REQUIRE(x == 2);
    REQUIRE(rb.pop(x)); REQUIRE(x == 3);

    // now empty
    REQUIRE_FALSE(rb.pop(x));
}

TEST_CASE("SpscRingBuffer: overflow rejects push when full", "[ringbuffer]") {
    constexpr size_t kCap = 4;
    SpscRingBuffer<int, kCap> rb;

    // push until failure, then assert failure is consistent.
    int pushed = 0;
    while (rb.push(pushed)) {
        ++pushed;
    }
    REQUIRE(pushed == kCap-1);

    // Once full, further pushes should fail
    REQUIRE_FALSE(rb.push(999));
}

TEST_CASE("SpscRingBuffer: spsc transfers all items", "[ringbuffer][thread]") {
    constexpr size_t kCap = 1024;
    constexpr int kN = 100000;

    SpscRingBuffer<int, kCap> rb;

    std::atomic<bool> done{false};
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    std::thread producer([&](){
        for (int i = 0; i < kN; ) {
            if (rb.push(i)) {
                ++i;
                produced.fetch_add(1, std::memory_order_relaxed);
            }
            // else: buffer full; retry
        }
        done.store(true, std::memory_order_release);
    });

    std::thread consumer([&](){
        int expected = 0;
        int x = -1;
        while (!done.load(std::memory_order_acquire) || consumed.load(std::memory_order_relaxed) < produced.load(std::memory_order_relaxed)) {
            if (rb.pop(x)) {
                REQUIRE(x == expected);
                ++expected;
                consumed.fetch_add(1, std::memory_order_relaxed);
            }
            // else: empty; retry
        }
        REQUIRE(expected == kN);
    });

    producer.join();
    consumer.join();

    REQUIRE(produced.load() == kN);
    REQUIRE(consumed.load() == kN);
}
