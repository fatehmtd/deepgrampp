cmake_minimum_required(VERSION 3.16)

project(DEEPGRAMPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEEPGRAMPP_BUILD_EXAMPLES "Build example applications" ON)

add_subdirectory(deepgrampp)
if(DEEPGRAMPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
