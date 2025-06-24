/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: event.c - implements the input event API
 * last Modified: 18.11.2010 : 18:45
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

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input/events.h"
#include "utils/logger.h"

#include "audio/sound.h"

/* data structures... */
MOUSE mouse;
/* directional pad button data - already abstracted from keyboard and joystick input */
DPAD dpad;
/* directional data keyboard - raw values not to be used directly */
DPAD key;
/* directional joystick data - raw values not to be used directly */
DPAD joystick;
bool ESCpressed          = false;
bool QUITrequest         = false;
Uint8 debounce_tmr_mouse    = 0;
Uint8 debounce_tmr_keys     = 0;
Uint8 debounce_tmr_dpad     = 0;

/* Optimization: Input buffering and reduced polling */
static struct {
    SDL_Event events[32];
    int event_count;
    Uint32 last_poll_time;
    Uint32 poll_interval;
} input_buffer = {0};

#if(HAVE_JOYSTICK != _NO_JOYSTICK)
Uint8 debounce_tmr_joystick = 0;
SDL_Joystick *CurrentJoystick;
#endif

/*************
 * CONSTANTS *
 *************/
const Uint8 Event_Debounce_timeslices = 20;

/*************
 * FUNCTIONS *
 *************/

/**
 *  This function initializes the event handling.
 *  It is called from InitGameEngine()
 */
void Event_Init()
{
	mouse.x            = 0;
	mouse.y            = 0;
	mouse.button       = 0;
	mouse.clicked      = false;
	debounce_tmr_mouse = Event_Debounce_timeslices;
	
	/* For turn-based games, we can use simpler input handling */
	input_buffer.event_count = 0;
	input_buffer.last_poll_time = 0;
	input_buffer.poll_interval = 50; /* Poll every 50ms for turn-based games */
}

/**
 *  This function processes SDL Events and updates the event data structures accordingly.
 *  SDL joystick input is mapped to an abstract directional pad.
 */
void Event_ProcessInput()
{
	/* For turn-based games, we can process all available events without timing restrictions */
	SDL_Event event;
	
	while (SDL_PollEvent(&event)) {
		switch (event.type)
		{
		/* mouse handling... */
		case SDL_MOUSEBUTTONUP:
			mouse.clicked = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			/* collect the mouse data and mouse click location */
			mouse.x = event.button.x;
			mouse.y = event.button.y;
			mouse.button = event.button.button;
			if(debounce_tmr_mouse == 0)
			{
				mouse.clicked = true;
			}
			break;
			/* keyboard handling... */
		case SDL_KEYUP:
			if(debounce_tmr_keys == 0)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_UP:
					/* up arrow */
					key.up = false;
					break;
				case SDLK_DOWN:
					/* down arrow */
					key.down = false;
					break;
				case SDLK_LEFT:
					/* left arrow */
					key.left = false;
					break;
				case SDLK_RIGHT:
					/* right arrow */
					key.right = false;
					break;
				case SDLK_LCTRL:
				case SDLK_RCTRL:
				case SDLK_LALT:
				case SDLK_RALT:
					/* any Control or ALT */
					key.button = false;
					break;
				case SDLK_ESCAPE:
					/* ESC pressed? */
					ESCpressed = true;
					break;
				case SDLK_KP_PLUS:
#if(defined __DINGUX)
				case SDLK_BACKSPACE:
#endif
					/* keypad + ? */
					Sound_IncreaseVolume();
					break;
				case SDLK_KP_MINUS:
#if(defined __DINGUX)
				case SDLK_TAB:
#endif
					/* keypad - ? */
					Sound_DecreaseVolume();
					break;
				default:
					break;
				}
			}
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:
				/* up arrow */
				key.up = true;
				break;
			case SDLK_DOWN:
				/* down arrow */
				key.down = true;
				break;
			case SDLK_LEFT:
				/* left arrow */
				key.left = true;
				break;
			case SDLK_RIGHT:
				/* right arrow */
				key.right = true;
				break;
			case SDLK_LCTRL:
			case SDLK_RCTRL:
			case SDLK_LALT:
			case SDLK_RALT:
				/* any Control or ALT */
				key.button = true;
				break;
			default:
				break;
			}
	   #if(HAVE_JOYSTICK != _NO_JOYSTICK)
		case SDL_JOYAXISMOTION:
			/* x Axis */
			if(event.jaxis.axis == 0)
			{
				joystick.left  = (bool) (event.jaxis.value < -16000);
				joystick.right = (bool) (event.jaxis.value >  16000);
			}
			/* y Axis */
			if(event.jaxis.axis == 1)
			{
				joystick.up    = (bool) (event.jaxis.value < -16000);
				joystick.down  = (bool) (event.jaxis.value >  16000);
			}
			break;
		case SDL_JOYBUTTONDOWN:
			switch(event.jbutton.button)
			{
#if(HAVE_JOYSTICK == _GP2X_JOYSTICK)
			case 0:
				joystick.button = true;
				break;
			case 1:
				joystick.button = true;
				break;
			case 2:
				joystick.button = true;
				break;
			case 3:
				joystick.button = true;
				break;
			case 4:
				joystick.button = true;
				break;
			case 5:
				joystick.button = true;
				break;
			case 6:
				joystick.button = true;
				break;
			case 7:
				joystick.button = true;
				break;
#endif
			default:
				joystick.button = true;
				break;
			}
			break;
		case SDL_JOYBUTTONUP:
			switch(event.jbutton.button)
			{
#if(HAVE_JOYSTICK ==_GP2X_JOYSTICK)
			case 0:
				joystick.button = false;
				break;
			case 1:
				joystick.button = false;
				break;
			case 2:
				joystick.button = false;
				break;
			case 3:
				joystick.button = false;
				break;
			case 4:
				joystick.button = false;
				break;
			case 5:
				joystick.button = false;
				break;
			case 6:
				joystick.button = false;
				break;
			case 7:
				joystick.button = false;
				break;
#endif
			default:
				joystick.button = false;
				break;
			}
			break;
	   #endif
		case SDL_QUIT:
			QUITrequest = true;
			break;
		default:
			break;
		}
	}
	
	/* Update debounce timers less frequently for turn-based games */
	if (debounce_tmr_mouse > 0) debounce_tmr_mouse--;
	if (debounce_tmr_keys > 0) debounce_tmr_keys--;
	if (debounce_tmr_dpad > 0) debounce_tmr_dpad--;
#if(HAVE_JOYSTICK != _NO_JOYSTICK)
	if (debounce_tmr_joystick > 0) debounce_tmr_joystick--;
#endif
}

/**
 *  @return true if a program shutdown has been requested
 */
bool Event_QuitRequested()
{
	return(QUITrequest);
}

/**
 *  @return true if has ESC been pressed
 */
bool Event_IsESCPressed()
{
	return(ESCpressed);
}

/**
 * @return X position of mouse click
 */
Uint16 Event_GetMouseX()
{
	return(mouse.x);
}

/**
 * @return Y position of mouse click
 */
Uint16 Event_GetMouseY()
{
	return(mouse.y);
}

/**
 * @return which mouse button was pressed
 */
Uint8 Event_GetMouseButton()
{
	return(mouse.button);
}

/**
 * @return if the mouse button has been clicked
 */
bool Event_MouseClicked()
{
	return(mouse.clicked);
}

/**
 * This function implements a callback to allow the event handler to debounce the mouse button.
 */
void Event_DebounceMouse()
{
	mouse.clicked=false;
	mouse.button=0;
	debounce_tmr_mouse = Event_Debounce_timeslices;
}
