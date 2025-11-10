#pragma once

#include "Timestamp.hpp"
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>

namespace hft {

// Latency statistics
struct LatencyStats {
    int64_t min_us = 0;
    int64_t max_us = 0;
    double mean_us = 0.0;
    double median_us = 0.0;
    double p50_us = 0.0;
    double p90_us = 0.0;
    double p95_us = 0.0;
    double p99_us = 0.0;
    double p99_9_us = 0.0;
    size_t count = 0;
    
    void print() const {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Latency Statistics (microseconds):\n";
        std::cout << "  Count:     " << count << "\n";
        std::cout << "  Min:       " << min_us << " μs\n";
        std::cout << "  Max:       " << max_us << " μs\n";
        std::cout << "  Mean:      " << mean_us << " μs\n";
        std::cout << "  Median:    " << median_us << " μs\n";
        std::cout << "  P50:       " << p50_us << " μs\n";
        std::cout << "  P90:       " << p90_us << " μs\n";
        std::cout << "  P95:       " << p95_us << " μs\n";
        std::cout << "  P99:       " << p99_us << " μs\n";
        std::cout << "  P99.9:     " << p99_9_us << " μs\n";
    }
};

// Calculate latency statistics from measurements
inline LatencyStats calculateStats(const std::vector<int64_t>& latencies_us) {
    LatencyStats stats;
    if (latencies_us.empty()) {
        return stats;
    }
    
    stats.count = latencies_us.size();
    auto sorted = latencies_us;
    std::sort(sorted.begin(), sorted.end());
    
    stats.min_us = sorted.front();
    stats.max_us = sorted.back();
    
    // Mean
    stats.mean_us = static_cast<double>(
        std::accumulate(sorted.begin(), sorted.end(), 0LL)) / sorted.size();
    
    // Median
    size_t mid = sorted.size() / 2;
    stats.median_us = sorted.size() % 2 == 0
        ? (sorted[mid - 1] + sorted[mid]) / 2.0
        : sorted[mid];
    
    // Percentiles
    auto percentile = [&sorted](double p) -> double {
        size_t idx = static_cast<size_t>(p * sorted.size());
        if (idx >= sorted.size()) idx = sorted.size() - 1;
        return sorted[idx];
    };
    
    stats.p50_us = percentile(0.50);
    stats.p90_us = percentile(0.90);
    stats.p95_us = percentile(0.95);
    stats.p99_us = percentile(0.99);
    stats.p99_9_us = percentile(0.999);
    
    return stats;
}

// Benchmark helper
class Benchmark {
public:
    Benchmark(const std::string& name) : name_(name) {}
    
    void start() {
        start_ns_ = getMonotonicNs();
    }
    
    void stop() {
        int64_t elapsed_ns = getMonotonicNs() - start_ns_;
        latencies_us_.push_back(elapsed_ns / 1000LL);
    }
    
    LatencyStats getStats() const {
        return calculateStats(latencies_us_);
    }
    
    void printResults() const {
        std::cout << "\n=== Benchmark: " << name_ << " ===\n";
        getStats().print();
        std::cout << "\n";
    }
    
    void reset() {
        latencies_us_.clear();
    }

private:
    std::string name_;
    int64_t start_ns_ = 0;
    std::vector<int64_t> latencies_us_;
};

} // namespace hft

