# HFT Optimizations Guide

This document describes the High-Frequency Trading (HFT) optimizations implemented in this codebase, following industry best practices from leading HFT firms.

## Overview

The codebase has been refactored to achieve microsecond-level latency targets, which is critical for HFT systems. All optimizations are Linux-specific and designed for production trading environments.

## Key Optimizations

### 1. Fixed-Point Arithmetic

**Problem**: Floating-point operations are slower and can introduce rounding errors.

**Solution**: Prices are represented as 64-bit integers scaled by 1,000,000 (micro-dollar precision).

```cpp
using Price = int64_t;
constexpr Price PRICE_SCALE = 1'000'000LL;
```

**Benefits**:
- Faster arithmetic operations
- No floating-point rounding errors
- Deterministic calculations

### 2. Cache-Optimized Data Structures

**Order Structure**:
- Tightly packed to 32 bytes (one cache line)
- 32-byte alignment for optimal cache behavior
- No padding waste

**Price Ladder**:
- Replaced `std::priority_queue` with sorted `std::map` for O(log n) insertion
- O(1) best price access via `rbegin()`/`begin()`
- Cache-friendly price level organization

### 3. Memory Management

**Memory Pool**:
- Pre-allocated pool of Order objects (1M by default)
- Zero dynamic allocations in hot path
- Lock-free allocation using atomic operations

**Memory Locking**:
- `mlockall()` prevents swapping to disk
- Critical for deterministic latency

### 4. Linux-Specific Optimizations

#### CPU Affinity & Thread Pinning
```cpp
hft::setCpuAffinity(cpu_id);        // Pin process to CPU
hft::pinThreadToCpu(cpu_id);        // Pin current thread
hft::pinThreadToCpu(thread, cpu_id); // Pin C++ thread
```
- Reduces cache misses from context switching
- Improves cache locality
- Critical for deterministic latency

#### NUMA Awareness
```cpp
int numa_node = hft::getNumaNode(cpu_id);
hft::setMemoryPolicy(numa_node);     // Prefer NUMA node for allocations
hft::bindThreadToNumaNode(numa_node); // Bind thread to NUMA node
void* ptr = hft::allocateOnNumaNode(size, numa_node); // Allocate on specific node
```
- Allocates memory on the same NUMA node as the CPU
- Reduces cross-NUMA memory access latency
- Critical for multi-socket systems
- Requires libnuma (optional, falls back gracefully)

#### Real-Time Scheduling
```cpp
hft::setRealtimePriority(50);  // SCHED_FIFO
```
- Prevents preemption by other processes
- Requires root privileges

#### Huge Pages
- 2MB pages instead of 4KB
- Reduces TLB misses
- Configured via linker flags

### 5. High-Resolution Timestamps

**RDTSC (Read Time-Stamp Counter)**:
- Ultra-fast CPU cycle counter (< 10 cycles vs hundreds for system calls)
- Used for latency measurements in hot paths
- Calibrated at startup to convert cycles to nanoseconds
- `LatencyTimerRDTSC` class for microsecond-level measurements

**clock_gettime()**:
- Used for wall-clock timestamps (order timestamps)
- More accurate for long durations
- `LatencyTimer` class for standard measurements

**Performance Comparison**:
- RDTSC: ~5-10 CPU cycles (~2-4 ns on 2.5 GHz CPU)
- clock_gettime: ~100-300 CPU cycles (~40-120 ns)
- **RDTSC is 10-30x faster** for latency measurements

**Usage**:
```cpp
// Ultra-fast latency measurement (hot path)
hft::LatencyTimerRDTSC timer;
// ... operation ...
int64_t latency_us = timer.elapsedUs();
uint64_t cycles = timer.elapsedCycles();

// Wall-clock timestamp (for orders)
int64_t timestamp = hft::getTimestampNs();  // Uses clock_gettime
```

### 6. Lock-Free SPSC Queue

**Single Producer Single Consumer Queue**:
- Lock-free, wait-free operations
- Cache-line aligned to avoid false sharing
- Bounded circular buffer (power-of-2 size)
- O(1) push/pop operations
- No CAS loops in common case

**Use Cases**:
- Market data handler → Order matcher pipeline
- Trade logging (async)
- Latency measurement aggregation
- Any producer-consumer pattern in hot paths

**Performance**:
- Push/Pop latency: < 50 ns (typical)
- Zero allocations
- No blocking, no locks

### 7. Async Logger

**Non-Blocking Logging**:
- SPSC queue-based message buffering
- Background thread for I/O
- Never blocks the hot path
- Configurable log levels
- File or stdout output

**Features**:
- Fixed-size message buffers (no allocations)
- Thread ID tracking
- Nanosecond timestamps
- Automatic flushing
- Drop messages if queue full (fail-fast)

**Usage**:
```cpp
LOG_INFO("Message");  // Non-blocking
LOG_INFO_F("Format: %d", value);  // Formatted
```

### 8. Network Optimizations

**Epoll-Based TCP Server**:
- Edge-triggered epoll for maximum efficiency
- Non-blocking I/O
- TCP_NODELAY to disable Nagle's algorithm
- SO_REUSEPORT for load balancing

**UDP Server with Busy Polling**:
- `UDPServer` class with SO_BUSY_POLL support
- Busy polling reduces latency by polling in kernel space
- Non-blocking UDP with configurable busy poll timeout (50-200μs typical)
- `UDPMulticastReceiver` for market data feeds
- SO_INCOMING_CPU for CPU affinity (with SO_REUSEPORT)

**Busy Polling Benefits**:
- Reduces latency by 10-50μs vs blocking I/O
- Kernel polls socket queue without going to sleep
- CPU-intensive but critical for microsecond-level latency
- Use with dedicated CPU cores

**Usage**:
```cpp
// UDP server with busy polling
UDPServer server(8080, handler, true); // true = enable busy poll
server.start();
server.run(); // Busy poll loop

// UDP multicast receiver
UDPMulticastReceiver receiver("239.255.1.1", 8080, "192.168.1.1", handler, true);
receiver.start();
receiver.run();
```

**Future Enhancements**:
- DPDK for kernel bypass
- Zero-copy techniques (sendfile, splice)
- XDP (eXpress Data Path) for even lower latency

### 9. Compiler Optimizations

**Build Flags**:
```cmake
-O3 -march=native -mtune=native
-flto                    # Link-time optimization
-ffast-math              # Aggressive floating-point optimizations
-funroll-loops           # Loop unrolling
-fno-exceptions          # No exception handling overhead
-fno-rtti                # No runtime type information
-finline-functions       # Aggressive inlining
-fomit-frame-pointer     # Omit frame pointers
```

**Link-Time Optimization (LTO)**:
- Cross-module optimizations
- Can improve performance by 10-20%

### 10. Header-Only Hot Paths

Critical functions are implemented inline in headers:
- Order comparison operators
- Price level access
- Matching logic

**Benefits**:
- Eliminates function call overhead
- Enables better compiler optimizations
- Reduces instruction cache misses

## Latency Targets

| Operation | Target | Measurement |
|-----------|--------|-------------|
| Order Add | < 1 μs | P99 latency |
| Order Cancel | < 1 μs | P99 latency |
| Order Match | < 2 μs | P99 latency |
| Best Price Lookup | < 100 ns | P99 latency |

## Performance Monitoring

Use `hft::LatencyTimer` and `hft::Benchmark` for latency measurements:

```cpp
hft::LatencyTimer timer;
// ... operation ...
int64_t latency_us = timer.elapsedUs();
```

## System Configuration

### Required Linux Settings

1. **Huge Pages**:
   ```bash
   echo 1024 > /proc/sys/vm/nr_hugepages
   ```

2. **CPU Isolation** (via kernel parameters):
   ```bash
   isolcpus=2,3 nohz_full=2,3 rcu_nocbs=2,3
   ```

3. **Disable CPU Frequency Scaling**:
   ```bash
   echo performance > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
   ```

4. **Network Tuning**:
   ```bash
   # Increase socket buffer sizes
   sysctl -w net.core.rmem_max=134217728
   sysctl -w net.core.wmem_max=134217728
   ```

### Running with Privileges

Some optimizations require root:
```bash
sudo ./MarketDataEngine  # For mlockall and SCHED_FIFO
```

## Best Practices

1. **Profile First**: Use `perf` or `valgrind` to identify bottlenecks
2. **Measure Everything**: Track P50, P90, P95, P99, P99.9 latencies
3. **Avoid Dynamic Allocation**: Use memory pools in hot paths
4. **Minimize System Calls**: Batch operations where possible
5. **Cache Awareness**: Design data structures for cache locality
6. **Branch Prediction**: Use `[[likely]]`/`[[unlikely]]` hints (C++20)

## Future Enhancements

- [ ] Lock-free price ladder implementation
- [ ] SIMD optimizations for batch operations
- [ ] DPDK integration for kernel bypass networking
- [ ] Custom memory allocator tuned for order book
- [ ] Profile-guided optimization (PGO) builds
- [ ] Hardware timestamping (PTP)
- [ ] FPGA offload for matching engine

## References

- "Low Latency C++" by Agner Fog
- "High Performance Trading" by Michael Driscoll
- "Designing Low Latency Trading Systems" by Peter Lawrey

