cmake_minimum_required(VERSION 3.10)
project(speakerbox VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-D_XOPEN_SOURCE=700 -D_GNU_SOURCE)
add_definitions(-DPI=3.14159265358979323846)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(speakerbox ${SOURCES})

target_link_libraries(speakerbox glog pthread)  # For threads, glog

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -Wall -Wextra -Wpedantic -pedantic-errors)
else()
    add_compile_options(-O3 -Wall -Wextra -Wpedantic -pedantic-errors)
endif()

# Tests
find_package(GTest)
if(GTEST_FOUND)
    enable_testing()
    add_executable(test_speakerbox tests/test_calculator.cpp src/calculator.cpp src/utils.cpp src/sha256.cpp)
    target_link_libraries(test_speakerbox GTest::GTest GTest::Main glog pthread)
    add_test(NAME SpeakerBoxTests COMMAND test_speakerbox)
endif()
