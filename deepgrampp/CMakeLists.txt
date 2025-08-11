cmake_minimum_required(VERSION 3.16)

project(DEEPGRAMPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(GNUInstallDirs)

find_package(Boost REQUIRED COMPONENTS system json thread REQUIRED)
find_package(OpenSSL REQUIRED)

# Debug: Print Boost information
message(STATUS "Boost_FOUND: ${Boost_FOUND}")
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL or set OPENSSL_ROOT_DIR.")
endif()

# fetch SPDLOG for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG "v1.15.0"
)

FetchContent_MakeAvailable(spdlog)

# fetch nlohmann for json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG "v3.12.0"
)

FetchContent_MakeAvailable(nlohmann_json)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

# Add the executable for the deepgrampp library
add_executable(deepgrampp deepgram.cpp)

target_link_libraries(deepgrampp 
    PRIVATE 
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    Boost::json
    ${OPENSSL_LIBRARIES}
)

target_compile_features(deepgrampp PRIVATE cxx_std_17)

if(WIN32)
    target_link_libraries(deepgrampp PRIVATE ws2_32 wsock32)
elseif(UNIX AND NOT APPLE)
    # On Linux, you might need pthread
    find_package(Threads REQUIRED)
    target_link_libraries(deepgrampp PRIVATE Threads::Threads)
endif()