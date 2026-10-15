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
if(EMSCRIPTEN)
    set(STONE_OUTPUT "${CMAKE_SOURCE_DIR}/build/web") # the web page and its files
endif()
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
if(EMSCRIPTEN)
    # The core throws and catches std::runtime_error.
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
endif()

option(STONE_BUILD_PLAYER "Build the SDL3 player" ON)
# The tests live in research/ with the fixtures they need, outside the repository.
set(STONE_TESTS_DEFAULT OFF)
if(EXISTS "${CMAKE_SOURCE_DIR}/research/tests/CMakeLists.txt" AND EXISTS "${CMAKE_SOURCE_DIR}/original_data/SA_V1_00.EXE")
    set(STONE_TESTS_DEFAULT ON)
endif()
option(STONE_BUILD_TESTS "Build and register the tests in research/tests (needs original_data/)" ${STONE_TESTS_DEFAULT})

set(STONE_CORE_SOURCES src/stone.cpp src/machine.cpp src/game.cpp src/screens.cpp src/selector.cpp src/stats.cpp src/lines.cpp src/video.cpp
    src/music.cpp src/assets.cpp src/content.cpp src/package.cpp src/settings.cpp src/zip.cpp)
add_library(stone_core ${STONE_CORE_SOURCES})
target_include_directories(stone_core PUBLIC src)
if(NOT EMSCRIPTEN) # in the browser readable code waits on a fiber, not a thread
    find_package(Threads REQUIRED) # readable code waits for ticks on a thread (machine.cpp)
    target_link_libraries(stone_core PUBLIC Threads::Threads)
endif()
# Editable assets live in assets/: the free pictures, songs, levels, texts
# and colours, yours to change. Every build
# packs them into build/stoneage.dat. The original's files are never copied
# there: the player packs them itself when it finds them in original_data/
# (play.cpp).
if(EMSCRIPTEN)
    # stone_assets cannot run inside a wasm build: a native one (the core
    # preset) packs the package, and the page preloads it.
    set(STONE_PACKAGE "${CMAKE_SOURCE_DIR}/build/stoneage.dat" CACHE FILEPATH
        "stoneage.dat packed by a native stone_assets")
    if(NOT EXISTS "${STONE_PACKAGE}")
        message(FATAL_ERROR "${STONE_PACKAGE} is missing. Build the core preset first; it packs assets/.")
    endif()
else()
    add_executable(stone_assets src/assets_main.cpp)
    target_link_libraries(stone_assets PRIVATE stone_core)
    set(STONE_ASSETS "${CMAKE_SOURCE_DIR}/assets" CACHE PATH "Directory of editable asset files")
    add_custom_target(stone_asset_package ALL
        COMMAND stone_assets pack "${STONE_ASSETS}" "${STONE_OUTPUT}/stoneage.dat"
        COMMENT "Packing assets/ into build/stoneage.dat")
endif()

# Tests that need nothing of the original game (ENHANCEMENTS.md, Tests).
include(CTest)
if(BUILD_TESTING AND NOT EMSCRIPTEN)
    add_executable(stone_tests tests/settings_tests.cpp tests/zip_tests.cpp)
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
        # An installed SDL3 when there is one, otherwise SDL built from source;
        # in the browser a static library linked into the page.
        if(EMSCRIPTEN)
            set(SDL_SHARED OFF CACHE BOOL "" FORCE)
            set(SDL_STATIC ON CACHE BOOL "" FORCE)
        else()
            set(SDL_SHARED ON CACHE BOOL "" FORCE)
            set(SDL_STATIC OFF CACHE BOOL "" FORCE)
        endif()
        set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(SDL3
            URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-3.4.16.tar.gz
            URL_HASH SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68
            DOWNLOAD_EXTRACT_TIMESTAMP ON
            FIND_PACKAGE_ARGS 3.4 CONFIG)
        FetchContent_MakeAvailable(SDL3)
    endif()

    # ymfm's YM3812, with one change (its README), from third_party/ymfm (BSD 3-Clause).
    add_library(ymfm third_party/ymfm/ymfm_opl.cpp third_party/ymfm/ymfm_adpcm.cpp
        third_party/ymfm/ymfm_pcm.cpp)
    target_include_directories(ymfm SYSTEM PUBLIC third_party/ymfm)
    if(MSVC)
        target_compile_options(ymfm PRIVATE /w)
    else()
        target_compile_options(ymfm PRIVATE -w)
    endif()
    add_library(stone_audio_core src/audio.cpp)
    target_link_libraries(stone_audio_core PUBLIC stone_core ymfm)
    add_executable(stone_play src/play.cpp src/gamepad.cpp)
    target_link_libraries(stone_play PRIVATE stone_audio_core SDL3::SDL3)
    if(MSVC)
        target_sources(stone_play PRIVATE src/play.manifest)
    endif()
    if(EMSCRIPTEN)
        # build/web/index.html with index.js, index.wasm and index.data (the
        # package, preloaded where the desktop finds it beside the player).
        set_target_properties(stone_play PROPERTIES OUTPUT_NAME index SUFFIX .html)
        # The page's footer links to the source it was built from; the
        # workflow gives the exact commit.
        set(STONE_SOURCE_URL "https://github.com/crpcrp/stone-age-enhanced" CACHE STRING
            "Where the web page's footer finds the source it was built from")
        configure_file("${CMAKE_SOURCE_DIR}/web/shell.html" "${CMAKE_BINARY_DIR}/shell.html" @ONLY)
        # web/saves.js keeps /save in IndexedDB and exports and imports it;
        # web/original.js adds the original game from a zip.
        set_property(TARGET stone_play APPEND PROPERTY LINK_DEPENDS
            "${CMAKE_BINARY_DIR}/shell.html" "${CMAKE_SOURCE_DIR}/web/saves.js"
            "${CMAKE_SOURCE_DIR}/web/original.js" "${STONE_PACKAGE}")
        target_link_options(stone_play PRIVATE
            -sASYNCIFY -sALLOW_MEMORY_GROWTH -sSTACK_SIZE=1048576
            -lidbfs.js -sEXPORTED_RUNTIME_METHODS=ccall
            "--pre-js=${CMAKE_SOURCE_DIR}/web/saves.js"
            "--pre-js=${CMAKE_SOURCE_DIR}/web/original.js"
            "--shell-file=${CMAKE_BINARY_DIR}/shell.html"
            "--preload-file=${STONE_PACKAGE}@/stoneage.dat")
        # What a phone needs to add the page to its home screen (E6).
        foreach(web_file manifest.webmanifest icon-192.png icon-512.png)
            add_custom_command(TARGET stone_play POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/web/${web_file}" "${STONE_OUTPUT}/${web_file}")
        endforeach()
    endif()
    # Scripted runs of the free set that tap the screen (ENHANCEMENTS.md, E6):
    # each screenshot is compared by its SHA-256. options.keys taps F5 in
    # the menu and moves the cursor down; selector.keys taps F2, then map 13.
    if(BUILD_TESTING AND NOT EMSCRIPTEN)
        set(tap_runs options selector)
        set(tap_ticks 760 900)
        set(tap_hashes dc22f9e3a3fad1d7b7f53f1c855bc0386e75ca616975825e904581d236738eb9
                       87557027b2516e928d4bd50917fc6266787c7265a4fe177b573f7878151f5830)
        foreach(name ticks hash IN ZIP_LISTS tap_runs tap_ticks tap_hashes)
            add_test(NAME taps_${name} COMMAND ${CMAKE_COMMAND} -DPLAYER=$<TARGET_FILE:stone_play>
                "-DPACKAGE=${STONE_OUTPUT}/stoneage.dat" "-DKEYS=${CMAKE_SOURCE_DIR}/tests/taps/${name}.keys"
                -DTICKS=${ticks} "-DOUT=${CMAKE_BINARY_DIR}/taps_${name}.bmp" -DSHA256=${hash}
                -P "${CMAKE_SOURCE_DIR}/tests/screenshot_test.cmake")
        endforeach()
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
