cmake_minimum_required(VERSION 3.10)
project(MarketDataEngine)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories("C:/curl/include") # Include curl headers
include_directories("${CMAKE_SOURCE_DIR}/../json") # Include json.hpp (one level above the main repo)

# Link curl library
link_directories("C:/curl/lib")

# Add source files
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmarks)

# Find and link external libraries
find_package(CURL REQUIRED)

# Executable
add_executable(MarketDataEngine src/main.cpp)

# Link curl
target_link_libraries(MarketDataEngine PRIVATE CURL::libcurl)

# Add tests
enable_testing()
add_subdirectory(tests)

# Add benchmarks
add_subdirectory(benchmarks)
