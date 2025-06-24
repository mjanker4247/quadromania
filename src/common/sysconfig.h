/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: sysconfig.h - target system specific settings at compile time
 * last Modified: 03.07.2010 : 18:00
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
 *
 */

#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

	/* global settings for switches */
	#define _LOW 10
	#define _HIGH 20

	#define _NO_JOYSTICK      0x00
	#define _DEFAULT_JOYSTICK 0x01
    #define _GP2X_JOYSTICK    0x10

	/* compile target dependant switches */
	#if(defined __GP2X_WIZ)
		/* GP2x Wiz target using SDL */
		#define SCREENRES 					_LOW
		/* use SDL hardware surface for the screen? */
		#define USE_HARDWARESURFACE         1
        /* use SDL doublebuffered surface for the screen? */
        #define USE_DOUBLEBUFFEREDSURFACE   1
		/* run in window or full screen? */
		#define HAVE_WINDOWED_MODE          0
		/* touch screen or show mouse pointer? */
		#define HAVE_MOUSE_POINTER          0
		/* use SDL datatypes? */
		#define HAVE_SDL_DATATYPES          1
		/* type of joystick for special button mappings? */
		#define HAVE_JOYSTICK               _GP2X_JOYSTICK
		/* enable sound via SDL_mixer ? */
		#define HAVE_AUDIO                  1
		/* same highscores etc to $HOME ? */
        #define USE_HOMEDIR                 0
	#elif(defined __DINGUX)

		/* Dingoo Dingux target using SDL - by zLoud and
         * TODO: We need keyboard support!
         *
         */
		/* GP2x Wiz target using SDL */
		#define SCREENRES 					_LOW
		/* use SDL hardware surface for the screen? */
		#define USE_HARDWARESURFACE         0
        /* use SDL doublebuffered surface for the screen? */
        #define USE_DOUBLEBUFFEREDSURFACE   0
		/* run in window or full screen? */
		#define HAVE_WINDOWED_MODE          0
		/* touch screen or show mouse pointer? */
		#define HAVE_MOUSE_POINTER          1
		/* use SDL datatypes? */
		#define HAVE_SDL_DATATYPES          1
		/* type of joystick for special button mappings? */
		#define HAVE_JOYSTICK               _DEFAULT_JOYSTICK
		/* enable sound via SDL_mixer ? */
		#define HAVE_AUDIO                  1
		/* same highscores etc to $HOME ? */
        #define USE_HOMEDIR                 1
	#else
		/* default SDL target for use with windowing GUI, e.q. X11/Windows/Mac OS */
		#define SCREENRES 					_HIGH
		/* use SDL hardware surface for the screen? */
		#define USE_HARDWARESURFACE         1
		/* use SDL doublebuffered surface for the screen? */
        #define USE_DOUBLEBUFFEREDSURFACE   1
		/* run in window or full screen? */
		#define HAVE_WINDOWED_MODE          1
		/* touch screen or show mouse pointer? */
		#define HAVE_MOUSE_POINTER          1
		/* use SDL datatypes? */
        #define HAVE_SDL_DATATYPES          1
		/* type of joystick for special button mappings? */
        #define HAVE_JOYSTICK               _DEFAULT_JOYSTICK
		/* enable sound via SDL_mixer ? */
		#define HAVE_AUDIO                  1
		/* same highscores etc to $HOME ? */
		#define USE_HOMEDIR                 1
	#endif

#define __DEBUG

#endif /* __SYSCONFIG_H */
