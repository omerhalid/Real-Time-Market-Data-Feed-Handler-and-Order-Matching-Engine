#pragma once

#include <cstdint>
#include <ctime>
#include <time.h>  // Linux-specific for clock_gettime

// High-resolution timestamp utilities for Linux
namespace hft {

// Get current timestamp in nanoseconds (Linux CLOCK_REALTIME)
[[nodiscard]] inline int64_t getTimestampNs() noexcept {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1'000'000'000LL + ts.tv_nsec;
}

// Get current timestamp in microseconds
[[nodiscard]] inline int64_t getTimestampUs() noexcept {
    return getTimestampNs() / 1000LL;
}

// Get monotonic timestamp (for latency measurements)
[[nodiscard]] inline int64_t getMonotonicNs() noexcept {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1'000'000'000LL + ts.tv_nsec;
}

// Latency measurement utility
class LatencyTimer {
public:
    LatencyTimer() : start_ns_(getMonotonicNs()) {}
    
    [[nodiscard]] int64_t elapsedNs() const noexcept {
        return getMonotonicNs() - start_ns_;
    }
    
    [[nodiscard]] int64_t elapsedUs() const noexcept {
        return elapsedNs() / 1000LL;
    }
    
    void reset() noexcept {
        start_ns_ = getMonotonicNs();
    }

private:
    int64_t start_ns_;
};

} // namespace hft

