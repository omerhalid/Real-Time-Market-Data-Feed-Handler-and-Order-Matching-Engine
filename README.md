# Real-Time Market Data Feed Handler & Order Matching Engine

## Description

This project implements a **real-time market data feed handler** and an **order matching engine**, optimized for **High-Frequency Trading (HFT)** with microsecond-level latency targets. It is built in **modern C++** (C++20) following HFT industry best practices.

The system is designed with the following goals in mind:
- **Ultra-low latency**: Microsecond-level order processing (< 1μs for P99 operations)
- **Linux-optimized**: Designed specifically for Linux production environments
- **Cache-efficient**: Data structures optimized for CPU cache locality
- **Zero-allocation hot paths**: Memory pools prevent dynamic allocations during trading
- **Real-time performance**: CPU affinity, memory locking, and real-time scheduling

**⚠️ Note**: This codebase is optimized for Linux and may not compile on macOS/Windows. It uses Linux-specific APIs and system calls.

---

## Features

### HFT-Optimized Core
- **Fixed-Point Prices**: 64-bit integer prices (micro-dollar precision) for deterministic arithmetic
- **Price Ladder**: Efficient O(log n) insertion, O(1) best price lookup using sorted maps
- **Cache-Aligned Orders**: 32-byte aligned Order structures (one cache line)
- **Memory Pool**: Pre-allocated order pool eliminates dynamic allocations in hot paths
- **Nanosecond Timestamps**: High-resolution timestamps for price-time priority matching

### Linux Optimizations
- **CPU Affinity & Thread Pinning**: Pin process and individual threads to specific CPU cores
- **NUMA Awareness**: Allocate memory on the same NUMA node as CPU (reduces cross-NUMA latency)
- **Memory Locking**: `mlockall()` prevents swapping to disk
- **Real-Time Scheduling**: SCHED_FIFO priority for deterministic latency
- **Huge Pages**: 2MB pages to reduce TLB misses
- **Epoll Server**: High-performance event-driven TCP networking
- **UDP Busy Polling**: SO_BUSY_POLL for ultra-low latency UDP (10-50μs improvement)

### Performance
- **Latency Measurement**: Built-in microsecond-precision latency tracking
- **Benchmarking Tools**: Statistical analysis (P50, P90, P95, P99, P99.9)
- **Compiler Optimizations**: Aggressive flags (-O3, -march=native, LTO, etc.)

### Inter-Thread Communication
- **Lock-Free SPSC Queue**: Single Producer Single Consumer queue for zero-latency inter-thread messaging
- **Async Logger**: Non-blocking logging using SPSC queue and background thread
- **Thread-Safe**: Designed for high-throughput producer-consumer patterns

For detailed optimization documentation, see [HFT_OPTIMIZATIONS.md](HFT_OPTIMIZATIONS.md).

---

## Folder Structure

```bash
.
├── CMakeLists.txt
├── include/
│   ├── order_matching/
│   ├── market_data/
│   ├── networking/
│   └── utils/
├── src/
│   ├── order_matching/
│   ├── market_data/
│   ├── networking/
│   └── utils/
├── tests/
├── third_party/
├── benchmarks/
├── cmake/
└── README.md
```

### Folder Overview

- **`include/`**: Contains header files for different modules (`order_matching`, `market_data`, `networking`, and `utils`).
- **`src/`**: Contains the source code for the system implementation, organized into functional modules.
- **`tests/`**: Unit tests for each module to ensure the system works as expected.
- **`benchmarks/`**: Contains performance benchmarks for critical components.
- **`third_party/`**: External libraries such as `curl` and `json.hpp` are integrated here.

---

## Getting Started

### Prerequisites

- **Linux** (Ubuntu 20.04+, RHEL 8+, or similar)
- **CMake** (version 3.15 or higher)
- **GCC 10+** or **Clang 12+** with C++20 support
- **Optional**: Boost (for networking features)
- **Optional**: libcurl (for HTTP client features)

**Note**: This project is optimized for Linux and uses Linux-specific APIs. It will not compile on macOS/Windows without modifications.

### Building the Project

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/market-data-order-matching-engine.git
   cd market-data-order-matching-engine
   ```

2. Configure Linux system (recommended for production):

   ```bash
   # Enable huge pages
   sudo sh -c 'echo 1024 > /proc/sys/vm/nr_hugepages'
   
   # Set CPU governor to performance
   sudo cpupower frequency-set -g performance
   ```

3. Create a build directory:

   ```bash
   mkdir build && cd build
   ```

4. Configure with CMake (Release mode for optimizations):

   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```

5. Build the project:

   ```bash
   make -j$(nproc)
   ```

6. Run the executable:

   ```bash
   # For full optimizations (requires root):
   sudo ./bin/MarketDataEngine
   
   # Or without root (some optimizations disabled):
   ./bin/MarketDataEngine
   ```

### Running Unit Tests

The project includes unit tests written using a testing framework (e.g., Google Test or Catch2). To run the tests:

```bash
make test
```

### Running Benchmarks

To benchmark the system’s performance:

```bash
./benchmarks/BenchmarkOrderMatching
./benchmarks/BenchmarkMarketData
```

---

## Usage

### Order Matching Engine

The order matching engine takes buy and sell orders, processes them in real-time, and matches them based on price and time priority. You can configure the engine with various order types (market, limit) and simulate trading activity using predefined order books.

### Market Data Handler

The market data handler fetches and processes live data streams from external APIs using `libcurl`. Data is parsed from JSON format using `json.hpp` and passed to the order matching engine to inform trading decisions.

---

## Project Goals

1. **Real-World Financial System Simulation**: The project simulates a real-world financial trading system, allowing developers to experience the challenges of high-frequency trading, real-time market data handling, and system performance optimization.
   
2. **Learning Modern C++**: This project is a great way to apply modern C++ features (C++20), including advanced data structures, concurrency, and performance optimizations.

3. **Performance & Scalability**: Designed to handle high-frequency trading scenarios, the system is built for performance, with an emphasis on low-latency data processing and order matching.

---

## Contributing

Contributions are welcome! Feel free to submit pull requests for new features, optimizations, or bug fixes. Please make sure that all new code includes unit tests and follows the existing coding style.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/YourFeature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin feature/YourFeature`)
5. Create a new Pull Request

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Contact

For any inquiries or further information, please contact:

- **Author**: Ömer Halit Cinar
- **Email**: omerhalidcinar@gmail.com

Feel free to reach out with questions, feedback, or ideas for future improvements!
