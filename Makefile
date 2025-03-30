#
# Quadromania
# (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
#
# File: Makefile
# last Modified: 27.06.2010 : 14:58
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
#
# THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
#
#

CFLAGS=`sdl-config --cflags` -W -Wall -ggdb -O3

# determine whether a DEBUG build is desired
ifeq ($(DEBUG),1)
CFLAGS += -D_DEBUG
endif

SDL_LIB=`sdl-config --libs`

# the compiler to use - please use the GNU C Compiler if possible
CC=gcc

# object files needed to link the executable
OBJS = main.o graphics.o event.o random.o SFont.o quadromania.o gui.o highscore.o sound.o
# sources are located here
VPATH = src
# name of executable
EXECUTABLE = quadromania
	
all: quadromania

quadromania: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJS) $(SDL_LIB) -lSDL_image -lSDL_mixer

main.o: main.c main.h version.h event.h gui.h

graphics.o: graphics.c graphics.h version.h

sound.o: sound.c sound.h version.h

gui.o: gui.c gui.h graphics.o

random.o: random.c random.h

quadromania.o: quadromania.c quadromania.h version.h boolean.h

event.o: event.c event.h boolean.h

highscore.o: highscore.c highscore.h datatypes.h

SFont.o: SFont.c SFont.h

# default make rule for .c files - compile to .o
.c.o: datatypes.h
	$(CC) $(CFLAGS) -c $<

# tasks without a generated result
.PHONY: clean

# cleanup
clean:
	rm -f *.o quadromania

