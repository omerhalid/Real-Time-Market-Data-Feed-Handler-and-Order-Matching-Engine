cmake_minimum_required(VERSION 3.15)
project(MarketDataEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(include)

# Linux-specific HFT optimizations
if(UNIX AND NOT APPLE)
    set(CMAKE_SYSTEM_NAME Linux)
    
    # Aggressive compiler optimizations for HFT
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -ffast-math -funroll-loops")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions -fno-rtti")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-parameter")
    
    # Link-time optimization
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    
    # Use huge pages (2MB pages)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,common-page-size=2097152")
    
    # Prefer static linking for performance
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    
    # Profile-guided optimization flags (uncomment after building with -fprofile-generate)
    # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-use -fprofile-correction")
    
    message(STATUS "Building with HFT optimizations for Linux")
else()
    message(WARNING "HFT optimizations are Linux-specific. Building with standard optimizations.")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Optional: Find Boost (for networking if needed)
find_package(Boost QUIET COMPONENTS system thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "Boost found: ${Boost_VERSION}")
else()
    message(STATUS "Boost not found - some networking features may be unavailable")
endif()

# Optional: Find libcurl (for HTTP client if needed)
find_package(CURL QUIET)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    message(STATUS "libcurl found")
else()
    message(STATUS "libcurl not found - HTTP client features unavailable")
endif()

# Source files
set(ORDER_MATCHING_SOURCES
    src/order_matching/OrderBook.cpp
    src/order_matching/Matcher.cpp
)

# Create main executable
add_executable(MarketDataEngine src/main.cpp ${ORDER_MATCHING_SOURCES})

# Link libraries
target_link_libraries(MarketDataEngine PRIVATE Threads::Threads)

if(Boost_FOUND)
    target_link_libraries(MarketDataEngine PRIVATE ${Boost_LIBRARIES})
endif()

if(CURL_FOUND)
    target_link_libraries(MarketDataEngine PRIVATE ${CURL_LIBRARIES})
endif()

# Linux-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(MarketDataEngine PRIVATE rt)  # For clock_gettime
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(MarketDataEngine PRIVATE
        $<$<CONFIG:Release>:
            -O3
            -march=native
            -mtune=native
            -flto
            -ffast-math
            -funroll-loops
            -fno-exceptions
            -fno-rtti
            -finline-functions
            -fomit-frame-pointer
        >
    )
endif()

# Set output directory
set_target_properties(MarketDataEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS MarketDataEngine
    RUNTIME DESTINATION bin
)
