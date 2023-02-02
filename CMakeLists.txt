cmake_minimum_required(VERSION 3.24)

include(cmake/app_utils.cmake)
setup_vcpkg_before_project()

project ( Quadromania CXX )

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
   "cmake/app_utils.cmake")
set(BUILD_SHARED_LIBS OFF)
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
)

include_directories (
	src/
)

add_executable (
   ${PROJECT_NAME}
   ${SOURCES} # We could've listed the source files here directly instead of using a variable to store them
   ${VerifyCXX}
)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   SDL2::SDL2main
   SDL2::SDL2-static
   SDL2_image::SDL2_image-static
   SDL2_mixer::SDL2_mixer-static
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)