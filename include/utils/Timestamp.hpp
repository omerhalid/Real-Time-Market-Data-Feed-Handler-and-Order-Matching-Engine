#pragma once

#include <cstdint>
#include <ctime>
#include <time.h>  // Linux-specific for clock_gettime

#ifdef __x86_64__ || __i386__
#include <x86intrin.h>  // For __rdtsc() intrinsic
#endif

// High-resolution timestamp utilities for Linux
namespace hft {

// RDTSC (Read Time-Stamp Counter) - Ultra-fast CPU cycle counter
// Much faster than clock_gettime() - typically < 10 CPU cycles vs hundreds
// Used for latency measurements in hot paths
#ifdef __x86_64__ || __i386__
// Use compiler intrinsic when available (GCC/Clang)
#ifdef __GNUC__
[[nodiscard]] inline uint64_t rdtsc() noexcept {
    return __rdtsc();
}

// RDTSCP - Serializing version (waits for previous instructions to complete)
[[nodiscard]] inline uint64_t rdtscp() noexcept {
    unsigned int aux;
    return __rdtscp(&aux);
}
#else
// Fallback to inline assembly for other compilers
[[nodiscard]] inline uint64_t rdtsc() noexcept {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

[[nodiscard]] inline uint64_t rdtscp() noexcept {
    unsigned int lo, hi, aux;
    __asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
    return ((uint64_t)hi << 32) | lo;
}
#endif
#else
// Fallback for non-x86 architectures
[[nodiscard]] inline uint64_t rdtsc() noexcept {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1'000'000'000LL + ts.tv_nsec;
}

[[nodiscard]] inline uint64_t rdtscp() noexcept {
    return rdtsc();
}
#endif

// RDTSC calibration - converts CPU cycles to nanoseconds
// Must be called once at startup to calibrate
class RDTSCCalibrator {
public:
    static void calibrate() {
        static bool calibrated = false;
        if (calibrated) return;
        
        // Measure RDTSC frequency by comparing with clock_gettime
        constexpr int iterations = 1000;
        uint64_t rdtsc_start = rdtsc();
        struct timespec ts_start, ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        
        // Busy wait
        for (volatile int i = 0; i < iterations; ++i) {}
        
        uint64_t rdtsc_end = rdtsc();
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        
        int64_t ns_elapsed = (ts_end.tv_sec - ts_start.tv_sec) * 1'000'000'000LL +
                            (ts_end.tv_nsec - ts_start.tv_nsec);
        uint64_t cycles_elapsed = rdtsc_end - rdtsc_start;
        
        if (cycles_elapsed > 0 && ns_elapsed > 0) {
            cycles_per_ns_ = static_cast<double>(cycles_elapsed) / ns_elapsed;
            ns_per_cycle_ = 1.0 / cycles_per_ns_;
        } else {
            // Fallback: assume 2.5 GHz CPU (2.5 cycles per ns)
            cycles_per_ns_ = 2.5;
            ns_per_cycle_ = 0.4;
        }
        
        calibrated = true;
    }
    
    // Convert RDTSC cycles to nanoseconds
    [[nodiscard]] static int64_t cyclesToNs(uint64_t cycles) noexcept {
        calibrate();
        return static_cast<int64_t>(cycles * ns_per_cycle_);
    }
    
    // Convert nanoseconds to RDTSC cycles
    [[nodiscard]] static uint64_t nsToCycles(int64_t ns) noexcept {
        calibrate();
        return static_cast<uint64_t>(ns * cycles_per_ns_);
    }

private:
    static double cycles_per_ns_;
    static double ns_per_cycle_;
};

double RDTSCCalibrator::cycles_per_ns_ = 2.5;  // Default: 2.5 GHz
double RDTSCCalibrator::ns_per_cycle_ = 0.4;

// Get current timestamp in nanoseconds using RDTSC (ultra-fast)
// For latency measurements - not wall-clock time
[[nodiscard]] inline int64_t getTimestampNsRDTSC() noexcept {
    return RDTSCCalibrator::cyclesToNs(rdtsc());
}

// Get current timestamp in nanoseconds (Linux CLOCK_REALTIME)
// Use for wall-clock timestamps (e.g., order timestamps)
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
// Uses clock_gettime - slower but more accurate for long durations
[[nodiscard]] inline int64_t getMonotonicNs() noexcept {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1'000'000'000LL + ts.tv_nsec;
}

// Ultra-fast latency timer using RDTSC
// Use for microsecond-level latency measurements in hot paths
class LatencyTimerRDTSC {
public:
    LatencyTimerRDTSC() : start_cycles_(rdtsc()) {}
    
    [[nodiscard]] int64_t elapsedNs() const noexcept {
        uint64_t end_cycles = rdtsc();
        uint64_t cycles_elapsed = end_cycles - start_cycles_;
        return RDTSCCalibrator::cyclesToNs(cycles_elapsed);
    }
    
    [[nodiscard]] int64_t elapsedUs() const noexcept {
        return elapsedNs() / 1000LL;
    }
    
    [[nodiscard]] uint64_t elapsedCycles() const noexcept {
        return rdtsc() - start_cycles_;
    }
    
    void reset() noexcept {
        start_cycles_ = rdtsc();
    }

private:
    uint64_t start_cycles_;
};

// Standard latency timer using clock_gettime
// Use for longer measurements or when wall-clock accuracy is needed
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

