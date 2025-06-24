cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug-Macos" 
        CACHE STRING 
        "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel, Debug-Windows, Release-Windows, RelWithDebInfo-Windows, MinSizeRel-Windows, Debug-Macos, Release-Macos, RelWithDebInfo-Macos, MinSizeRel-Macos, Debug-Linux, Release-Linux, RelWithDebInfo-Linux, MinSizeRel-Linux, Debug-Android, Release-Android, RelWithDebInfo-Android, MinSizeRel-Android, Debug-IOS, Release-IOS, RelWithDebInfo-IOS, MinSizeRel-IOS)" FORCE)
endif()

# Set available build types
set_property(CACHE CMAKE_BUILD_TYPE 
    PROPERTY STRINGS 
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel"
    "Debug-Windows" "Release-Windows" "RelWithDebInfo-Windows" "MinSizeRel-Windows"
    "Debug-Macos" "Release-Macos" "RelWithDebInfo-Macos" "MinSizeRel-Macos"
    "Debug-Linux" "Release-Linux" "RelWithDebInfo-Linux" "MinSizeRel-Linux"
    "Debug-Android" "Release-Android" "RelWithDebInfo-Android" "MinSizeRel-Android"
    "Debug-IOS" "Release-IOS" "RelWithDebInfo-IOS" "MinSizeRel-IOS")

# Set Ninja as the default generator if not specified
if(NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja")
endif()

# Enable Ninja's fast dependency scanning
if(CMAKE_GENERATOR MATCHES "Ninja")
    set(CMAKE_NINJA_CMCLDEPS ON)
endif()

Project ( Quadromania )

# Find SDL2 and related libraries using vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

message(STATUS "SDL2 found: ${SDL2_FOUND}")
message(STATUS "SDL2_image found: ${SDL2_image_FOUND}")
message(STATUS "SDL2_mixer found: ${SDL2_mixer_FOUND}")
message(STATUS "SDL2_ttf found: ${SDL2_ttf_FOUND}")
message(STATUS "spdlog found: ${spdlog_FOUND}")

# Debug: Print available targets
get_target_property(SDL2_TARGETS SDL2::SDL2 TYPE)
message(STATUS "SDL2 target type: ${SDL2_TARGETS}")

# Core game engine sources
set(CORE_SOURCES
    src/core/game.c
    src/core/game.h
    src/core/state_manager.c
    src/core/state_manager.h
)

# Graphics subsystem sources
set(GRAPHICS_SOURCES
    src/graphics/renderer.c
    src/graphics/renderer.h
    src/graphics/fonts.c
    src/graphics/fonts.h
    src/graphics/ttf_font.c
    src/graphics/ttf_font.h
    src/graphics/ui.c
    src/graphics/ui.h
)

# Input subsystem sources
set(INPUT_SOURCES
    # src/input/events.c  # Excluded to avoid duplicate symbols with events_compat.c
    src/input/events.h
    src/input/events_compat.c
    src/input/events_compat.h
    src/input/input_manager.c
    src/input/input_manager.h
    src/input/mouse_handler.c
    src/input/mouse_handler.h
    src/input/platform_input.c
    src/input/platform_input.h
    src/input/platform/sdl2_input.c
    src/input/platform/sdl2_input.h
)

# Audio subsystem sources
set(AUDIO_SOURCES
    src/audio/sound.c
    src/audio/sound.h
)

# Data management sources
set(DATA_SOURCES
    src/data/highscore.c
    src/data/highscore.h
    src/data/config.c
    src/data/config.h
    src/data/resources.c
    src/data/resources.h
)

# Utility sources
set(UTILS_SOURCES
    src/utils/logger.c
    src/utils/logger.h
    src/utils/random.cpp
    src/utils/random.h
)

# Platform abstraction sources
set(PLATFORM_SOURCES
    # Will be added later when platform-specific code is extracted
)

# Common headers
set(COMMON_HEADERS
    src/common/version.h
)

# Main application
set(MAIN_SOURCES
    src/main.c
    src/main.h
)

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${GRAPHICS_SOURCES}
    ${INPUT_SOURCES}
    ${AUDIO_SOURCES}
    ${DATA_SOURCES}
    ${UTILS_SOURCES}
    ${PLATFORM_SOURCES}
    ${COMMON_HEADERS}
    ${MAIN_SOURCES}
)

add_executable(quadromania ${ALL_SOURCES})

# Add include directories for modular structure
target_include_directories(quadromania PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Set compiler flags based on build type
target_compile_options(quadromania PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
    $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
)

# Organize source files in IDE
source_group("Core" FILES ${CORE_SOURCES})
source_group("Graphics" FILES ${GRAPHICS_SOURCES})
source_group("Input" FILES ${INPUT_SOURCES})
source_group("Audio" FILES ${AUDIO_SOURCES})
source_group("Data" FILES ${DATA_SOURCES})
source_group("Utils" FILES ${UTILS_SOURCES})
source_group("Platform" FILES ${PLATFORM_SOURCES})
source_group("Common" FILES ${COMMON_HEADERS})
source_group("Main" FILES ${MAIN_SOURCES})

# Link libraries using vcpkg targets with fallback to static targets
target_link_libraries(quadromania PRIVATE
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    spdlog::spdlog_header_only
)

# Copy data directory to build directory
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# Print build information
message(STATUS "Build configuration:")
message(STATUS "  - Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - Generator: ${CMAKE_GENERATOR}")
message(STATUS "  - Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "  - Binary dir: ${CMAKE_BINARY_DIR}")
