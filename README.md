# Real-Time Market Data Feed Handler & Order Matching Engine

## Description

This project implements a **real-time market data feed handler** and an **order matching engine**, designed to mimic the core functionality of a financial trading system. It is built in **modern C++** (C++20) and uses external libraries like **libcurl** for network communication and **JSON for Modern C++** (`json.hpp`) for handling market data in JSON format.

The system is designed with the following goals in mind:
- Efficiently process real-time market data from external APIs.
- Provide a robust order matching engine that can handle high-frequency trading (HFT) scenarios.
- Ensure modularity and scalability, allowing the system to be extended with new features, such as different order types and market instruments.
- Provide performance benchmarks and unit tests to ensure correctness and efficiency.

This project simulates a real-world trading environment and is ideal for developers looking to gain practical experience in financial systems, networking, and high-performance C++ development.

---

## Features

- **Order Matching Engine**: Supports different order types (limit, market, etc.) and matches buy/sell orders based on price and time priority.
- **Real-Time Market Data Handling**: Fetches and processes real-time market data streams using `libcurl` and `json.hpp`.
- **Networking**: Implements client-server communication for interacting with market data feeds and potentially other external systems.
- **High-Performance**: Designed for performance with an emphasis on low-latency order matching and data processing.
- **Unit Testing**: Includes comprehensive unit tests to validate core functionality.
- **Benchmarking**: Performance benchmarks for key system components such as the order matching engine and market data handler.

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

- **CMake** (version 3.10 or higher)
- **C++20** compatible compiler (e.g., GCC, Clang, MSVC)
- **libcurl** (for handling HTTP requests)
- **JSON for Modern C++** (`json.hpp`)

### Building the Project

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/market-data-order-matching-engine.git
   cd market-data-order-matching-engine
   ```

2. Create a build directory:

   ```bash
   mkdir build && cd build
   ```

3. Run CMake to configure the project:

   ```bash
   cmake ..
   ```

4. Build the project:

   ```bash
   make
   ```

5. Run the executable:

   ```bash
   ./MarketDataEngine
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
