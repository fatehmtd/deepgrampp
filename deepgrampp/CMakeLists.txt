cmake_minimum_required(VERSION 3.16)

project(DEEPGRAMPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(GNUInstallDirs)

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

# ---------------------------------------------------------------------------
# On Apple Silicon, point curl/libwebsockets' OpenSSL discovery at Homebrew's
# keg (the top-level CMakeLists.txt already adds /opt/homebrew/lib as a
# linker search path for this whole build).
# ---------------------------------------------------------------------------
if(APPLE)
    execute_process(
        COMMAND brew --prefix openssl@3
        OUTPUT_VARIABLE HOMEBREW_OPENSSL_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
    )
    if(HOMEBREW_OPENSSL_PREFIX)
        set(OPENSSL_ROOT_DIR "${HOMEBREW_OPENSSL_PREFIX}" CACHE PATH "" FORCE)
    endif()
endif()

# ---------------------------------------------------------------------------
# Third-party dependencies (all fetched via CMake FetchContent -- no vcpkg,
# no Boost)
# ---------------------------------------------------------------------------

# spdlog for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        "v1.15.0"
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(spdlog)

# nlohmann/json for JSON parsing
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        "v3.12.0"
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# libcurl -- built from source; TLS backend is platform-dependent. Only used
# by the (currently unused) CurlHttpTransport, kept for future REST support.
set(CURL_USE_LIBIDN2   OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_MISC_DOCS    OFF CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE     OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING      OFF CACHE BOOL "" FORCE)
set(ENABLE_MANUAL      OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBSSH2   OFF CACHE BOOL "" FORCE)
if(WIN32)
    set(CURL_USE_SCHANNEL ON  CACHE BOOL "" FORCE)
    set(CURL_USE_OPENSSL  OFF CACHE BOOL "" FORCE)
else()
    set(CURL_USE_SCHANNEL OFF CACHE BOOL "" FORCE)
    set(CURL_USE_OPENSSL  ON  CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(
    CURL
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG        curl-8_11_1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(CURL)

# libwebsockets -- cross-platform WebSocket transport used by Listen/Speak/Flux streaming
set(LWS_WITH_SHARED           OFF CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TESTAPPS      ON  CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_SERVER   ON  CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_CLIENT   ON  CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_PING     ON  CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_ECHO     ON  CACHE BOOL "" FORCE)
set(LWS_WITH_MINIMAL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LWS_WITH_SSL              ON  CACHE BOOL "" FORCE)
set(LWS_WITH_OPENSSL          ON  CACHE BOOL "" FORCE)
set(LWS_WITH_BORINGSSL        OFF CACHE BOOL "" FORCE)
set(LWS_WITH_MBEDTLS          OFF CACHE BOOL "" FORCE)
set(LWS_WITH_CLIENT           ON  CACHE BOOL "" FORCE)
set(LWS_WITH_SERVER           OFF CACHE BOOL "" FORCE)
set(LWS_WITH_HTTP2            OFF CACHE BOOL "" FORCE)
set(LWS_ROLE_WS               ON  CACHE BOOL "" FORCE)
set(LWS_ROLE_H1               ON  CACHE BOOL "" FORCE)
set(LWS_ROLE_H2               OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LIBUV            OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LIBEVENT         OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LIBEV            OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    libwebsockets
    GIT_REPOSITORY https://github.com/warmcat/libwebsockets.git
    GIT_TAG        v4.3.3
    GIT_SHALLOW    TRUE
)
# libwebsockets v4.3.3 declares cmake_minimum_required < 3.5, which CMake 4.x removed.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(libwebsockets)
unset(CMAKE_POLICY_VERSION_MINIMUM)

# ---------------------------------------------------------------------------
# Declare deepgrampp library
# ---------------------------------------------------------------------------
add_library(deepgrampp STATIC
    ./src/listen-ws.cpp
    ./src/speak-ws.cpp
    ./src/listen-flux.cpp
    ./transport/lws_websocket_transport.cpp
    ./transport/curl_http_transport.cpp
)

target_link_libraries(deepgrampp
    PRIVATE
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    libcurl
    websockets
)

# Generate export header
include(GenerateExportHeader)
generate_export_header(deepgrampp
    BASE_NAME deepgrampp
    EXPORT_FILE_NAME deepgrampp_lib_export.h
)

target_include_directories(deepgrampp PUBLIC
    $<INSTALL_INTERFACE:include>  # For installed package (optional but good practice)
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # For direct build
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # To export the symbols header
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/deepgrampp # to simplify the internal include files
)

target_compile_features(deepgrampp PRIVATE cxx_std_17)

if(WIN32)
    target_link_libraries(deepgrampp PRIVATE ws2_32 wsock32 crypt32)
elseif(UNIX AND NOT APPLE)
    # On Linux, you might need pthread
    find_package(Threads REQUIRED)
    target_link_libraries(deepgrampp PRIVATE Threads::Threads)
endif()

include(GNUInstallDirs)

# Install the library
install(TARGETS deepgrampp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY include/deepgrampp/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/deepgrampp
    FILES_MATCHING PATTERN "*.hpp"
)
