cmake_minimum_required(VERSION 3.16)

project(DEEPGRAMPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# optionally build example applications
option(DEEPGRAMPP_BUILD_LISTEN_EXAMPLE "Build listen example application" OFF)
message(STATUS "DEEPGRAMPP_BUILD_LISTEN_EXAMPLE: ${DEEPGRAMPP_BUILD_LISTEN_EXAMPLE}")

option(DEEPGRAMPP_BUILD_LISTEN_FLUX_EXAMPLE "Build listen flux example application" ON)
message(STATUS "DEEPGRAMPP_BUILD_LISTEN_FLUX_EXAMPLE: ${DEEPGRAMPP_BUILD_LISTEN_FLUX_EXAMPLE}")

option(DEEPGRAMPP_BUILD_SPEAK_EXAMPLE "Build speak example application" OFF)
message(STATUS "DEEPGRAMPP_BUILD_SPEAK_EXAMPLE: ${DEEPGRAMPP_BUILD_SPEAK_EXAMPLE}")

# main library
add_subdirectory(deepgrampp)

if(DEEPGRAMPP_BUILD_LISTEN_EXAMPLE)
    add_subdirectory(examples/listen)
endif()
if(DEEPGRAMPP_BUILD_SPEAK_EXAMPLE)
    add_subdirectory(examples/speak)
endif()
if(DEEPGRAMPP_BUILD_LISTEN_FLUX_EXAMPLE)
    add_subdirectory(examples/listen-flux)
endif()