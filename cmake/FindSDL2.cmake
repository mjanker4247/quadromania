find_path(SDL2_INCLUDE_DIR names SDL.h
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed"
    DOC "SDL2 include directory")

find_library(SDL2_LIBRARY names SDL2 SDL2main
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed"
    DOC "SDL2 library")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2 REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARY)

if(SDL2_FOUND)
    message(STATUS "Using SDL2 found in ${SDL2_INCLUDE_DIR}")
    message(STATUS "Using SDL2 library found in ${SDL2_LIBRARY}")
    add_library(SDL2::SDL2main UNKNOWN IMPORTED)
    set_target_properties(SDL2::SDL2main PROPERTIES
        IMPORTED_LOCATION "${SDL2_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}")
endif()

mark_as_advanced(SDL2_LIBRARY SDL2_INCLUDE_DIR)