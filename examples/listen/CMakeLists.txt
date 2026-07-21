cmake_minimum_required(VERSION 3.16)

project(listen VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

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

# fetch SPDLOG for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG "v1.15.0"
)

FetchContent_MakeAvailable(spdlog)

add_executable(listen main.cpp)

target_link_libraries(listen
    PRIVATE deepgrampp
    PRIVATE nlohmann_json
    PRIVATE spdlog::spdlog
)

# so `./listen` finds sample_speech.raw via its relative path when run
# straight out of the build tree, without needing `cmake --install` first
add_custom_command(TARGET listen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/sample_speech.raw
        $<TARGET_FILE_DIR:listen>/sample_speech.raw
)

install(TARGETS listen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# copy the sample audio file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/sample_speech.raw
    DESTINATION ${CMAKE_INSTALL_DATADIR}/deepgrampp
)
