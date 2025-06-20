cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)

# Set Ninja as the default generator if not specified
if(NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja")
endif()

# Enable Ninja's fast dependency scanning
if(CMAKE_GENERATOR MATCHES "Ninja")
    set(CMAKE_NINJA_CMCLDEPS ON)
endif()

Project ( Quadromania )

# Find SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message(STATUS "SDL2_IMAGE_INCLUDE_DIRS: ${SDL2_IMAGE_INCLUDE_DIRS}")
message(STATUS "SDL2_IMAGE_LIBRARIES: ${SDL2_IMAGE_LIBRARIES}")
message(STATUS "SDL2_MIXER_INCLUDE_DIRS: ${SDL2_MIXER_INCLUDE_DIRS}")
message(STATUS "SDL2_MIXER_LIBRARIES: ${SDL2_MIXER_LIBRARIES}")

set (SOURCES
   src/boolean.h
   src/datatypes.h
   src/event.c
   src/event.h
   src/graphics.c
   src/graphics.h
   src/gui.c
   src/gui.h
   src/highscore.c
   src/highscore.h
   src/main.c
   src/main.h
   src/quadromania.c
   src/quadromania.h
   src/random.c
   src/random.h
   src/SFont.c
   src/SFont.h
   src/sound.c
   src/sound.h
   src/sysconfig.h
   src/version.h
   src/debug.c
   src/ttf_font.c
)

# Set library search paths explicitly
link_directories(${SDL2_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS} ${SDL2_MIXER_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS})

include_directories (
   src/
   ${SDL2_INCLUDE_DIRS}
   ${SDL2_IMAGE_INCLUDE_DIRS}
   ${SDL2_MIXER_INCLUDE_DIRS}
   ${SDL2_TTF_INCLUDE_DIRS}
)

add_executable(quadromania
   ${SOURCES}
   ChangeLog
   README
)

target_link_libraries(quadromania PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::SDL2_MIXER
    PkgConfig::SDL2_TTF
)

# Copy data directory to build directory
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
