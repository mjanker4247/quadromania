find_path(SDL2_image_INCLUDE_DIR names SDL_image.h
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed"
    DOC "SDL2_image include directory")

find_library(SDL2_image_LIBRARY names SDL2_image
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed"
    DOC "SDL2_image library")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2_image REQUIRED_VARS SDL2_image_INCLUDE_DIR SDL2_image_LIBRARY)

if(SDL2_image_FOUND)
    message(STATUS "Using SDL2_image found in ${SDL2_image_INCLUDE_DIR}")
    message(STATUS "Using SDL2_image library found in ${SDL2_image_LIBRARY}")
    add_library(SDL2_image::SDL2_image UNKNOWN IMPORTED)
    set_target_properties(SDL2_image::SDL2_image PROPERTIES
        IMPORTED_LOCATION "${SDL2_image_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_image_INCLUDE_DIR}")
endif()

mark_as_advanced(SDL2_image_LIBRARY SDL2_image_INCLUDE_DIR)