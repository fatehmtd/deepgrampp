cmake_minimum_required(VERSION 3.16)

project(DEEPGRAMPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# optionally build example applications
option(DEEPGRAMPP_BUILD_LISTEN_EXAMPLE "Build listen example application" OFF)
option(DEEPGRAMPP_BUILD_SPEAK_EXAMPLE "Build speak example application" OFF)

# main library
add_subdirectory(deepgrampp)

if(DEEPGRAMPP_BUILD_LISTEN_EXAMPLE)
    add_subdirectory(examples/listen)
endif()
if(DEEPGRAMPP_BUILD_SPEAK_EXAMPLE)
    add_subdirectory(examples/speak)
endif()
