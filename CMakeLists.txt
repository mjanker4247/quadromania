cmake_minimum_required(VERSION 3.10)

Project ( Quadromania )

# Set SDL2 paths explicitly
set(SDL2_INCLUDE_DIRS "/opt/homebrew/include/SDL2")
set(SDL2_LIBRARIES "/opt/homebrew/lib/libSDL2.dylib")
set(SDL2_IMAGE_INCLUDE_DIRS "/opt/homebrew/include/SDL2")
set(SDL2_IMAGE_LIBRARIES "/opt/homebrew/lib/libSDL2_image.dylib")
set(SDL2_MIXER_INCLUDE_DIRS "/opt/homebrew/include/SDL2")
set(SDL2_MIXER_LIBRARIES "/opt/homebrew/lib/libSDL2_mixer.dylib")

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
   ${SDL2_INCLUDE_DIRS}
   ${SDL2_IMAGE_INCLUDE_DIRS}
   ${SDL2_MIXER_INCLUDE_DIRS}
)

add_executable(quadromania
   ${SOURCES}
   ChangeLog
   README
)

target_link_libraries(quadromania PRIVATE
   ${SDL2_LIBRARIES}
   ${SDL2_IMAGE_LIBRARIES}
   ${SDL2_MIXER_LIBRARIES}
)
