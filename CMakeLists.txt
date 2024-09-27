cmake_minimum_required(VERSION 3.10)
project(MarketDataEngine)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories("${CMAKE_SOURCE_DIR}/../json")  # Include json.hpp
include_directories("${CMAKE_SOURCE_DIR}/../boost_1_86_0")  # Include Boost headers

# Use the cURL installation in C:/curl
set(CURL_INCLUDE_DIR "C:/curl/include")
set(CURL_LIBRARY "C:/curl/lib/libcurl.a")  # Use the static library

# Add source files
add_subdirectory(src)

# Executable
add_executable(MarketDataEngine src/main.cpp)

# Link cURL library
target_include_directories(MarketDataEngine PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(MarketDataEngine PRIVATE ${CURL_LIBRARY})
