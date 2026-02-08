#pragma once
#include <atomic>
#include <cstddef>


template <typename T, size_t N>
class SpscRingBuffer {
    static_assert(N >= 2, "Ring buffer size must be >= 2");

public:
    SpscRingBuffer() : write_(0), read_(0) {}

    bool push(const T& v){
        // get write index 
        size_t w = write_.load(std::memory_order_relaxed);
        // get read index from consumer
        size_t r = read_.load(std::memory_order_acquire);

        // check for overflow
        size_t next = inc(w);
        if (next == r) {
            return false; 
        }

        data_[w] = v;
        // publish changes for consumer
        write_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& out){
        // get read index
        size_t r = read_.load(std::memory_order_relaxed);
        // get write index from producer
        size_t w = write_.load(std::memory_order_acquire);

        // check for underrun
        if (w == r){
            return false;
        } 

        out = data_[r];
        read_.store(inc(r), std::memory_order_release);
        return true;
    }

    size_t size_approx() const {
        size_t w = write_.load(std::memory_order_acquire);
        size_t r = read_.load(std::memory_order_acquire);
        if (w >= r) {
            return w - r;
        }
        return (N - r) + w;
    }

private:
    static size_t inc(size_t i) {return (i+1) % N;};

    T data_[N];

    alignas(64) std::atomic<size_t> write_;
    alignas(64) std::atomic<size_t> read_;
};