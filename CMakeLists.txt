cmake_minimum_required(VERSION 3.21)

Project ( Quadromania )

set (
   SOURCES
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

# REQUIRED does not work in CMake <=2.4.6 for SDL
Find_Package ( SDL REQUIRED )
# if using SDL_image
Find_Package ( SDL_image REQUIRED )
# if using SDL_mixer
Find_Package ( SDL_mixer REQUIRED )

# Workaround for the non-working REQUIRED flag
if ( NOT SDL_FOUND )
   message ( FATAL_ERROR "SDL not found!" )
endif ( NOT SDL_FOUND )

link_libraries (
   ${SDL_LIBRARY}
   ${SDLIMAGE_LIBRARY} # if using SDL_image, obviously
   ${SDLMIXER_LIBRARY} # if using SDL_mixer
   SDLmain # Sadly not included in SDL_LIBRARY variable
)

add_executable (
   quadromania
   ${SOURCES} # We could've listed the source files here directly instead of using a variable to store them
   ChangeLog
   README
)
