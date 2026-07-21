cmake_minimum_required(VERSION 3.16)

project(DEEPGRAMPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# On Apple Silicon, Homebrew installs to /opt/homebrew which is not a default
# linker search path. Add it here (top level, before add_subdirectory) so it
# applies to the deepgrampp static library AND the example executables that
# link it -- libcurl/libwebsockets pull in bare "-lidn2" etc. that only
# resolve via this search path.
# ---------------------------------------------------------------------------
if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(HOMEBREW_PREFIX)
        link_directories("${HOMEBREW_PREFIX}/lib")
    endif()
endif()

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