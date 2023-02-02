find_path(SDL2_mixer_INCLUDE_DIR names SDL_mixer.h
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed"
    DOC "SDL2_mixer include directory")

find_library(SDL2_mixer_LIBRARY names SDL_mixer
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed"
    DOC "SDL2_mixer library")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2_mixer REQUIRED_VARS SDL2_mixer_INCLUDE_DIR SDL2_mixer_LIBRARY)

if(SDL2_mixer_FOUND)
    message(STATUS "Using SDL2_mixer found in ${SDL2_mixer_INCLUDE_DIR}")
    message(STATUS "Using SDL2_mixer library found in ${SDL2_mixer_LIBRARY}")
    add_library(SDL2_mixer::SDL2_mixer UNKNOWN IMPORTED)
    set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
        IMPORTED_LOCATION "${SDL2_mixer_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_mixer_INCLUDE_DIR}")
endif()

mark_as_advanced(SDL2_mixer_LIBRARY SDL2_mixer_INCLUDE_DIR)