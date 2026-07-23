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
# Third-party dependencies. Each one is looked up on the system first (via
# find_package, using whatever package manager or manual install the user
# has -- Homebrew, apt, pacman, vcpkg-as-a-toolchain, a manual prefix passed
# via CMAKE_PREFIX_PATH, etc.) and only fetched from source with FetchContent
# as a fallback. This project itself never shells out to a specific package
# manager.
#
# The CMake *package registry* is deliberately excluded from these lookups:
# it's a machine-global cache that other, unrelated projects populate when
# they FetchContent-build a dependency with CMake's export(TARGETS...), and
# picking one of those up here would mean silently depending on some other
# project's build directory (which can vanish or change flags at any time)
# instead of a real install or our own fetched copy.
# ---------------------------------------------------------------------------

# spdlog for logging
find_package(spdlog QUIET NO_CMAKE_PACKAGE_REGISTRY NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
if(spdlog_FOUND)
    message(STATUS "Using system spdlog ${spdlog_VERSION}")
else()
    message(STATUS "spdlog not found on the system, fetching from source")
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        "v1.15.0"
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(spdlog)
endif()

# nlohmann/json for JSON parsing
find_package(nlohmann_json QUIET NO_CMAKE_PACKAGE_REGISTRY NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
if(nlohmann_json_FOUND)
    message(STATUS "Using system nlohmann_json ${nlohmann_json_VERSION}")
else()
    message(STATUS "nlohmann_json not found on the system, fetching from source")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        "v3.12.0"
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# mbedTLS -- TLS backend for curl and libwebsockets on non-Windows platforms
# where no system OpenSSL can be assumed (notably Android). Mirrors sonioxpp's
# resolution order: reuse a parent-provided 'mbedtls' target (e.g. the one
# VASDK_ANDROID builds from source) before fetching our own.
if(NOT WIN32 AND NOT TARGET mbedtls)
    # Probe for a system mbedtls with dedicated variables, not
    # MBEDTLS_INCLUDE_DIR/MBEDTLS_LIBRARY/etc themselves: those get
    # CACHE FORCE-overwritten with FetchContent's own values below, so on the
    # next reconfigure (e.g. CMake Tools re-running after any CMakeLists.txt
    # edit) find_path/find_library would just see their own leftover values
    # already cached and skip searching -- making this condition true forever
    # afterwards and silently skipping FetchContent_MakeAvailable(mbedtls),
    # which drops the 'mbedtls' target from the build without dropping the
    # stale MBEDTLS_LIBRARY="mbedtls" string that libwebsockets/curl still
    # link against, producing an unresolvable bare "-lmbedtls" at link time.
    find_path(_system_mbedtls_include_dir mbedtls/ssl.h)
    find_library(_system_mbedtls_lib mbedtls)
    find_library(_system_mbedx509_lib mbedx509)
    find_library(_system_mbedcrypto_lib mbedcrypto)

    if(_system_mbedtls_include_dir AND _system_mbedtls_lib AND _system_mbedx509_lib AND _system_mbedcrypto_lib)
        set(MBEDTLS_INCLUDE_DIR "${_system_mbedtls_include_dir}" CACHE PATH "" FORCE)
        set(MBEDTLS_LIBRARY "${_system_mbedtls_lib}" CACHE STRING "" FORCE)
        set(MBEDX509_LIBRARY "${_system_mbedx509_lib}" CACHE STRING "" FORCE)
        set(MBEDCRYPTO_LIBRARY "${_system_mbedcrypto_lib}" CACHE STRING "" FORCE)
    else()
        set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
        set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(MBEDTLS_FATAL_WARNINGS OFF CACHE BOOL "" FORCE)
        set(USE_SHARED_MBEDTLS_LIBRARY OFF CACHE BOOL "" FORCE)
        set(USE_STATIC_MBEDTLS_LIBRARY ON CACHE BOOL "" FORCE)
        set(DISABLE_PACKAGE_CONFIG_AND_INSTALL ON CACHE BOOL "" FORCE)
        FetchContent_Declare(
            mbedtls
            GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
            GIT_TAG        v3.6.2
            GIT_SHALLOW    TRUE
        )
        FetchContent_MakeAvailable(mbedtls)
        set(MBEDTLS_INCLUDE_DIR "${mbedtls_SOURCE_DIR}/include" CACHE PATH "" FORCE)
        set(MBEDTLS_LIBRARY mbedtls CACHE STRING "" FORCE)
        set(MBEDX509_LIBRARY mbedx509 CACHE STRING "" FORCE)
        set(MBEDCRYPTO_LIBRARY mbedcrypto CACHE STRING "" FORCE)
    endif()
    unset(_system_mbedtls_include_dir CACHE)
    unset(_system_mbedtls_lib CACHE)
    unset(_system_mbedx509_lib CACHE)
    unset(_system_mbedcrypto_lib CACHE)
endif()
# If a parent build already provides the 'mbedtls' target (e.g. VASDK_ANDROID),
# MBEDTLS_INCLUDE_DIR/MBEDTLS_LIBRARY/MBEDX509_LIBRARY/MBEDCRYPTO_LIBRARY are
# expected to already be set as CACHE variables by that parent — same
# convention sonioxpp relies on.

# libcurl -- backs CurlHttpTransport, used by ListenRestClient/SpeakRestClient
# for Deepgram's batch REST endpoints. A parent build may already provide
# CURL::libcurl (e.g. VASDK_ANDROID, which builds curl from source since
# Android has no system curl for find_package to discover) -- reuse it
# before fetching our own.
if(NOT TARGET CURL::libcurl)
    find_package(CURL QUIET)
endif()
if(TARGET CURL::libcurl)
    message(STATUS "Using existing CURL::libcurl target")
else()
    message(STATUS "libcurl not found on the system, fetching from source")
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
        set(CURL_USE_MBEDTLS  OFF CACHE BOOL "" FORCE)
    else()
        set(CURL_USE_SCHANNEL OFF CACHE BOOL "" FORCE)
        set(CURL_USE_OPENSSL  OFF CACHE BOOL "" FORCE)
        set(CURL_USE_MBEDTLS  ON  CACHE BOOL "" FORCE)
    endif()
    FetchContent_Declare(
        CURL
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG        curl-8_11_1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(CURL)
    if(NOT TARGET CURL::libcurl AND TARGET libcurl)
        add_library(CURL::libcurl ALIAS libcurl)
    endif()
endif()

# libwebsockets -- cross-platform WebSocket transport used by Listen/Speak/Flux streaming
find_package(libwebsockets QUIET CONFIG NO_CMAKE_PACKAGE_REGISTRY NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
if(libwebsockets_FOUND)
    message(STATUS "Using system libwebsockets ${libwebsockets_VERSION}")
elseif(NOT TARGET websockets)
    message(STATUS "libwebsockets not found on the system, fetching from source")
    set(LWS_WITH_SHARED           OFF CACHE BOOL "" FORCE)
    set(LWS_WITHOUT_TESTAPPS      ON  CACHE BOOL "" FORCE)
    set(LWS_WITHOUT_TEST_SERVER   ON  CACHE BOOL "" FORCE)
    set(LWS_WITHOUT_TEST_CLIENT   ON  CACHE BOOL "" FORCE)
    set(LWS_WITHOUT_TEST_PING     ON  CACHE BOOL "" FORCE)
    set(LWS_WITHOUT_TEST_ECHO     ON  CACHE BOOL "" FORCE)
    set(LWS_WITH_MINIMAL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LWS_WITH_SSL              ON  CACHE BOOL "" FORCE)
    # Build against the mbedTLS resolved above instead of a system/Homebrew
    # OpenSSL, so this also works on Android (no system OpenSSL there).
    set(LWS_WITH_OPENSSL          OFF CACHE BOOL "" FORCE)
    set(LWS_WITH_BORINGSSL        OFF CACHE BOOL "" FORCE)
    set(LWS_WITH_MBEDTLS          ON  CACHE BOOL "" FORCE)
    set(LWS_MBEDTLS_INCLUDE_DIRS "${MBEDTLS_INCLUDE_DIR}" CACHE STRING "" FORCE)
    set(LWS_MBEDTLS_LIBRARIES "${MBEDTLS_LIBRARY}" "${MBEDX509_LIBRARY}" "${MBEDCRYPTO_LIBRARY}" CACHE STRING "" FORCE)
    set(LWS_WITH_CLIENT           ON  CACHE BOOL "" FORCE)
    set(LWS_WITH_SERVER           OFF CACHE BOOL "" FORCE)
    set(LWS_WITH_HTTP2            OFF CACHE BOOL "" FORCE)
    set(LWS_ROLE_WS               ON  CACHE BOOL "" FORCE)
    set(LWS_ROLE_H1               ON  CACHE BOOL "" FORCE)
    set(LWS_ROLE_H2               OFF CACHE BOOL "" FORCE)
    set(LWS_WITH_LIBUV            OFF CACHE BOOL "" FORCE)
    set(LWS_WITH_LIBEVENT         OFF CACHE BOOL "" FORCE)
    set(LWS_WITH_LIBEV            OFF CACHE BOOL "" FORCE)
    # We only ever link the "websockets" target in-tree via FetchContent; skip its
    # install(EXPORT) rules so they don't have to resolve every dependency (e.g.
    # mbedtls, whose own install/export is disabled) into an export set.
    set(LWS_WITH_EXPORT_LWSTARGETS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        libwebsockets
        GIT_REPOSITORY https://github.com/warmcat/libwebsockets.git
        GIT_TAG        v4.3.3
        GIT_SHALLOW    TRUE
        PATCH_COMMAND  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/patch-libwebsockets-mbedtls-extensions.cmake
    )
    # libwebsockets v4.3.3 declares cmake_minimum_required < 3.5, which CMake 4.x removed.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_MakeAvailable(libwebsockets)
    unset(CMAKE_POLICY_VERSION_MINIMUM)
endif()

# ---------------------------------------------------------------------------
# Declare deepgrampp library
# ---------------------------------------------------------------------------
add_library(deepgrampp STATIC
    ./src/listen-ws.cpp
    ./src/listen-rest.cpp
    ./src/speak-ws.cpp
    ./src/speak-rest.cpp
    ./src/listen-flux.cpp
    ./transport/lws_websocket_transport.cpp
    ./transport/curl_http_transport.cpp
)

target_link_libraries(deepgrampp
    PRIVATE
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    CURL::libcurl
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
