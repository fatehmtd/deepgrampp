cmake_minimum_required(VERSION 3.16)

project(DEEPGRAMPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# optionally build example applications
option(DEEPGRAMPP_BUILD_EXAMPLES "Build example applications" ON)
message(STATUS "DEEPGRAMPP_BUILD_EXAMPLES: ${DEEPGRAMPP_BUILD_EXAMPLES}")

# main library
add_subdirectory(deepgrampp)

if(DEEPGRAMPP_BUILD_EXAMPLES)
    add_subdirectory(examples/listen)
    add_subdirectory(examples/speak)
    add_subdirectory(examples/listen-flux)
endif()