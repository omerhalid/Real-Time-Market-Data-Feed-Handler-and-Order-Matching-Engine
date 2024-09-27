cmake_minimum_required(VERSION 3.10)
project(MarketDataEngine)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories("${CMAKE_SOURCE_DIR}/../json")  # Include json.hpp
include_directories("C:/Users/katka/source/market_data_handler/boost_1_86_0")  # Include Boost headers

# Use the cURL installation in C:/curl
set(CURL_INCLUDE_DIR "C:/curl/include")
set(CURL_LIBRARY "C:/curl/lib/libcurl.a")  # Use the static library

# Find Boost libraries
set(BOOST_ROOT "C:/Users/katka/source/market_data_handler/boost_1_86_0")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/stage/lib")

find_package(Boost 1.86 REQUIRED COMPONENTS system thread)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Could not find Boost libraries")
endif()

# Manually specify the Boost libraries (for MinGW)
set(BOOST_LIBS
    "${BOOST_LIBRARYDIR}/libboost_system-mgw14-mt.a"
    "${BOOST_LIBRARYDIR}/libboost_thread-mgw14-mt.a"
)

# Create the server executable
add_executable(MarketDataEngineServer src/networking/Server_main.cpp src/networking/Server.cpp src/order_matching/OrderBook.cpp src/order_matching/Matcher.cpp)
target_include_directories(MarketDataEngineServer PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(MarketDataEngineServer PRIVATE ${CURL_LIBRARY} ${BOOST_LIBS} ws2_32)

# Create the client executable
add_executable(MarketDataEngineClient src/networking/Client_main.cpp src/networking/Client.cpp)
target_include_directories(MarketDataEngineClient PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(MarketDataEngineClient PRIVATE ${CURL_LIBRARY} ${BOOST_LIBS} ws2_32)
