cmake_minimum_required(VERSION 3.25)
project(stone_reconstruction LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Executables land in build/ whatever the generator, so documented paths
# (build/stone_play.exe) stay valid.
set(STONE_OUTPUT "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${STONE_OUTPUT}")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} "${STONE_OUTPUT}")
endforeach()

if(MSVC)
    add_compile_options(/W3 /utf-8 /bigobj)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(MINGW)
        add_link_options(-static-libgcc -static-libstdc++)
    endif()
endif()

option(STONE_BUILD_PLAYER "Build the SDL3 player" ON)
# The tests live in research/ with the fixtures they need, outside the repository.
set(STONE_TESTS_DEFAULT OFF)
if(EXISTS "${CMAKE_SOURCE_DIR}/research/tests/CMakeLists.txt" AND EXISTS "${CMAKE_SOURCE_DIR}/original_data/SA_V1_00.EXE")
    set(STONE_TESTS_DEFAULT ON)
endif()
option(STONE_BUILD_TESTS "Build and register the tests in research/tests (needs original_data/)" ${STONE_TESTS_DEFAULT})

set(STONE_CORE_SOURCES src/stone.cpp src/machine.cpp src/game.cpp src/screens.cpp src/selector.cpp src/video.cpp
    src/music.cpp src/assets.cpp src/content.cpp src/package.cpp src/settings.cpp)
add_library(stone_core ${STONE_CORE_SOURCES})
target_include_directories(stone_core PUBLIC src)
find_package(Threads REQUIRED) # readable code waits for ticks on a thread (machine.cpp)
target_link_libraries(stone_core PUBLIC Threads::Threads)
# Editable assets live in assets/: the free pictures, songs, levels, texts
# and colours, yours to change. Every build
# packs them into build/stoneage.dat. The original's files are never copied
# there: the player packs them itself when it finds them in original_data/
# (play.cpp).
add_executable(stone_assets src/assets_main.cpp)
target_link_libraries(stone_assets PRIVATE stone_core)
set(STONE_ASSETS "${CMAKE_SOURCE_DIR}/assets" CACHE PATH "Directory of editable asset files")
add_custom_target(stone_asset_package ALL
    COMMAND stone_assets pack "${STONE_ASSETS}" "${STONE_OUTPUT}/stoneage.dat"
    COMMENT "Packing assets/ into build/stoneage.dat")

# Tests that need nothing of the original game (ENHANCEMENTS.md, Tests).
include(CTest)
if(BUILD_TESTING)
    add_executable(stone_tests tests/settings_tests.cpp)
    target_link_libraries(stone_tests PRIVATE stone_core)
    add_test(NAME settings COMMAND stone_tests)
endif()

if(STONE_BUILD_PLAYER)
    # Dependencies are downloaded once into the build tree and checked against
    # pinned hashes. FETCHCONTENT_SOURCE_DIR_<NAME> points one at a local copy.
    include(FetchContent)
    if(WIN32 AND NOT SDL3_DIR)
        # SDL's prebuilt MinGW development package; its import library also links with MSVC.
        FetchContent_Declare(sdl3_mingw
            URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-devel-3.4.16-mingw.tar.gz
            URL_HASH SHA256=c7ef65bd72eabac6e5b535411dbd8d5824d0aab24fd62ff8812666b336f18a9c
            DOWNLOAD_EXTRACT_TIMESTAMP ON)
        FetchContent_MakeAvailable(sdl3_mingw)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(STONE_SDL_ARCH "${sdl3_mingw_SOURCE_DIR}/x86_64-w64-mingw32")
        else()
            set(STONE_SDL_ARCH "${sdl3_mingw_SOURCE_DIR}/i686-w64-mingw32")
        endif()
        add_library(SDL3::SDL3 SHARED IMPORTED)
        set_target_properties(SDL3::SDL3 PROPERTIES
            IMPORTED_LOCATION "${STONE_SDL_ARCH}/bin/SDL3.dll"
            IMPORTED_IMPLIB "${STONE_SDL_ARCH}/lib/libSDL3.dll.a"
            INTERFACE_INCLUDE_DIRECTORIES "${STONE_SDL_ARCH}/include")
    else()
        # An installed SDL3 when there is one, otherwise SDL built from source.
        set(SDL_SHARED ON CACHE BOOL "" FORCE)
        set(SDL_STATIC OFF CACHE BOOL "" FORCE)
        set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(SDL3
            URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-3.4.16.tar.gz
            URL_HASH SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68
            DOWNLOAD_EXTRACT_TIMESTAMP ON
            FIND_PACKAGE_ARGS 3.4 CONFIG)
        FetchContent_MakeAvailable(SDL3)
    endif()

    # DOSBox 0.74-3, unmodified; only its OPL emulator DBOPL is built.
    FetchContent_Declare(dosbox
        URL https://deb.debian.org/debian/pool/main/d/dosbox/dosbox_0.74-3.orig.tar.gz
            https://downloads.sourceforge.net/project/dosbox/dosbox/0.74-3/dosbox-0.74-3.tar.gz
        URL_HASH SHA256=c0d13dd7ed2ed363b68de615475781e891cd582e8162b5c3669137502222260a
        DOWNLOAD_EXTRACT_TIMESTAMP ON)
    FetchContent_MakeAvailable(dosbox)
    # The two files are copied apart from DOSBox's own headers, so that
    # dbopl.h finds the small stand-ins in src/dosbox_compat.
    set(STONE_DBOPL "${CMAKE_BINARY_DIR}/dbopl")
    foreach(name dbopl.cpp dbopl.h)
        file(COPY "${dosbox_SOURCE_DIR}/src/hardware/${name}" DESTINATION "${STONE_DBOPL}")
    endforeach()
    file(SHA256 "${STONE_DBOPL}/dbopl.cpp" dbopl_cpp_hash)
    file(SHA256 "${STONE_DBOPL}/dbopl.h" dbopl_h_hash)
    if(NOT dbopl_cpp_hash STREQUAL "e538f5021c960aa180e43ec7910646b17b4948284723d8aa6c7dd873777e5141" OR
       NOT dbopl_h_hash STREQUAL "d0b98e9df2b35b1fe83af96fd4fdc45b392a53827f13259110384193c7c61e45")
        message(FATAL_ERROR "DOSBox DBOPL source checksum mismatch")
    endif()
    add_library(dbopl "${STONE_DBOPL}/dbopl.cpp")
    target_include_directories(dbopl PRIVATE "${CMAKE_SOURCE_DIR}/src/dosbox_compat")
    target_include_directories(dbopl SYSTEM PUBLIC "${STONE_DBOPL}")
    if(MSVC)
        target_compile_options(dbopl PRIVATE /w)
    else()
        target_compile_options(dbopl PRIVATE -w)
    endif()
    add_library(stone_audio_core src/audio.cpp)
    target_include_directories(stone_audio_core PRIVATE src/dosbox_compat)
    target_link_libraries(stone_audio_core PUBLIC stone_core dbopl)
    add_executable(stone_play src/play.cpp)
    target_link_libraries(stone_play PRIVATE stone_audio_core SDL3::SDL3)
    if(MSVC)
        target_sources(stone_play PRIVATE src/play.manifest)
    endif()
    if(WIN32)
        add_custom_command(TARGET stone_play POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:stone_play>)
    endif()
    # The notices go beside the player; a partial copy of the sources still builds.
    foreach(notice third_party/licenses LICENSE LEGAL.md)
        get_filename_component(notice_name "${notice}" NAME)
        if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/${notice}")
            add_custom_command(TARGET stone_play POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/${notice}" "${STONE_OUTPUT}/${notice_name}")
        elseif(EXISTS "${CMAKE_SOURCE_DIR}/${notice}")
            add_custom_command(TARGET stone_play POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/${notice}" "${STONE_OUTPUT}/${notice_name}")
        else()
            message(WARNING "${notice} is missing; it is not copied beside the player.")
        endif()
    endforeach()
endif()

if(STONE_BUILD_TESTS)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/research/tests/CMakeLists.txt")
        message(FATAL_ERROR "The tests are not part of this repository; they need research/tests and original_data/.")
    endif()
    enable_testing()
    include(research/tests/CMakeLists.txt)
endif()
