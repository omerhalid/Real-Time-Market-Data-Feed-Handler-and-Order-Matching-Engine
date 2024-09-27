cmake_minimum_required(VERSION 3.10)
project(MarketDataEngine)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories("C:/Users/katka/source/market_data_handler/curl/include") # Include curl headers
include_directories("${CMAKE_SOURCE_DIR}/../json") # Include json.hpp (one level above the main repo)
include_directories("${CMAKE_SOURCE_DIR}/../boost_1_86_0") # Include Boost headers

# Link curl library
link_directories("C:/Users/katka/source/market_data_handler/curl/lib") # don't forget to change this to your own path

# Add source files
add_subdirectory(src)
# add_subdirectory(tests) # Commented out as the directory does not contain a CMakeLists.txt file
# add_subdirectory(benchmarks) # Commented out as the directory does not contain a CMakeLists.txt file

# Manually specify cURL library and include directories
set(CURL_LIBRARY "C:/Users/katka/source/market_data_handler/curl/lib/libcurl.lib")
set(CURL_INCLUDE_DIR "C:/Users/katka/source/market_data_handler/curl/include")

# Find and link external libraries
find_package(CURL REQUIRED)

# Executable
add_executable(MarketDataEngine src/main.cpp)

# Link curl library
target_link_libraries(MarketDataEngine PRIVATE ${CURL_LIBRARY})

# Add tests
# enable_testing()
# add_subdirectory(tests)

# Add benchmarks
# add_subdirectory(benchmarks)