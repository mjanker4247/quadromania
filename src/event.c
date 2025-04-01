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
#include "datatypes.h"
#include "event.h"
#include "boolean.h"
#include "sysconfig.h"
#include "debug.h"

#include "sound.h"

/* data structures... */
MOUSE mouse;
/* directional pad button data - already abstracted from keyboard and joystick input */
DPAD dpad;
/* directional data keyboard - raw values not to be used directly */
DPAD key;
/* directional joystick data - raw values not to be used directly */
DPAD joystick;
BOOLEAN ESCpressed          = FALSE;
BOOLEAN QUITrequest         = FALSE;
Uint8 debounce_tmr_mouse    = 0;
Uint8 debounce_tmr_keys     = 0;
Uint8 debounce_tmr_dpad     = 0;

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
	mouse.clicked      = FALSE;
	debounce_tmr_mouse = Event_Debounce_timeslices;
	debounce_tmr_keys  = Event_Debounce_timeslices;
	debounce_tmr_dpad  = Event_Debounce_timeslices;

	key.up             = FALSE;
	key.down           = FALSE;
	key.left           = FALSE;
	key.right          = FALSE;
	key.button         = FALSE;

#if(HAVE_JOYSTICK != _NO_JOYSTICK)
	Joystick_Init();
#endif
}

/**
 *  This function processes SDL Events and updates the event data structures accordingly.
 *  SDL joystick input is mapped to an abstract directional pad.
 */
void Event_ProcessInput()
{
	SDL_Event event; /* SDL event for keyboard, mouse and focus actions... */

	if (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		/* mouse handling... */
		case SDL_MOUSEBUTTONUP:
			mouse.clicked = FALSE;
			break;
		case SDL_MOUSEBUTTONDOWN:
			/* collect the mouse data and mouse click location */
			mouse.x = event.button.x;
			mouse.y = event.button.y;
			mouse.button = event.button.button;
			if(debounce_tmr_mouse == 0)
			{
				mouse.clicked = TRUE;
			}
			/* fprintf(stderr,"click %d at %d,%d\n",mouse.button,mouse.x,mouse.y); */
			break;
			/* keyboard handling... */
		case SDL_KEYUP:
			if(debounce_tmr_keys == 0)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_UP:
					/* up arrow */
					key.up = FALSE;
					break;
				case SDLK_DOWN:
					/* down arrow */
					key.down = FALSE;
					break;
				case SDLK_LEFT:
					/* left arrow */
					key.left = FALSE;
					break;
				case SDLK_RIGHT:
					/* right arrow */
					key.right = FALSE;
					break;
				case SDLK_LCTRL:
				case SDLK_RCTRL:
				case SDLK_LALT:
				case SDLK_RALT:
					/* any Control or ALT */
					key.button = FALSE;
					break;
				case SDLK_ESCAPE:
					/* ESC pressed? */
					ESCpressed = TRUE;
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
				key.up = TRUE;
				break;
			case SDLK_DOWN:
				/* down arrow */
				key.down = TRUE;
				break;
			case SDLK_LEFT:
				/* left arrow */
				key.left = TRUE;
				break;
			case SDLK_RIGHT:
				/* right arrow */
				key.right = TRUE;
				break;
			case SDLK_LCTRL:
			case SDLK_RCTRL:
			case SDLK_LALT:
			case SDLK_RALT:
				/* any Control or ALT */
				key.button = TRUE;
				break;
			default:
				break;
			}
	   #if(HAVE_JOYSTICK != _NO_JOYSTICK)
		case SDL_JOYAXISMOTION:
			/* x Axis */
			if(event.jaxis.axis == 0)
			{
				joystick.left  = (BOOLEAN) (event.jaxis.value < -16000);
				joystick.right = (BOOLEAN) (event.jaxis.value >  16000);
			}
			/* y Axis */
			if(event.jaxis.axis == 1)
			{
				joystick.up    = (BOOLEAN) (event.jaxis.value < -16000);
				joystick.down  = (BOOLEAN) (event.jaxis.value >  16000);
			}
			break;
		case SDL_JOYBUTTONDOWN:
			switch(event.jbutton.button)
			{
			case JOYSTICK_BUTTON_ESC:
				ESCpressed = TRUE;
				DEBUG_PRINT("Joystick ESC pressed");
				break;
#if(HAVE_JOYSTICK ==_GP2X_JOYSTICK)
			case GP2X_BUTTON_UP:
				/* up arrow */
				key.up = TRUE;
				break;
			case GP2X_BUTTON_DOWN:
				/* down arrow */
				key.down = TRUE;
				break;
			case GP2X_BUTTON_LEFT:
				/* left arrow */
				key.left = TRUE;
				break;
			case GP2X_BUTTON_RIGHT:
				/* right arrow */
				key.right = TRUE;
				break;
			case GP2X_BUTTON_A:
			case GP2X_BUTTON_B:
			case GP2X_BUTTON_X:
			case GP2X_BUTTON_Y:
				/* any fire button */
				key.button = TRUE;
				break;
#elif(HAVE_JOYSTICK == _DEFAULT_JOYSTICK)
			case 0:
			case 1:
				/* firebutton A or B?*/
				joystick.button = TRUE;
				break;
#endif
			default:
				break;
			}
			break;
		case SDL_JOYBUTTONUP:
			switch(event.jbutton.button)
			{
#if(HAVE_JOYSTICK ==_GP2X_JOYSTICK)
			case GP2X_BUTTON_UP:
				/* up arrow */
				key.up = FALSE;
				break;
			case GP2X_BUTTON_DOWN:
				/* down arrow */
				key.down = FALSE;
				break;
			case GP2X_BUTTON_LEFT:
				/* left arrow */
				key.left = FALSE;
				break;
			case GP2X_BUTTON_RIGHT:
				/* right arrow */
				key.right = FALSE;
				break;
			case GP2X_BUTTON_A:
			case GP2X_BUTTON_B:
			case GP2X_BUTTON_X:
			case GP2X_BUTTON_Y:
				/* any fire button */
				key.button = FALSE;
				break;
			case GP2X_BUTTON_VOLUP:
				Sound_IncreaseVolume();
				break;
			case GP2X_BUTTON_VOLDOWN:
				Sound_DecreaseVolume();
				break;
#elif(HAVE_JOYSTICK == _DEFAULT_JOYSTICK)
			case 0:
			case 1:
				/* firebutton A or B?*/
				joystick.button = FALSE;
				break;
#endif
			default:
				break;
			}
			break;
	   #endif
			/* SDL_QUIT event (window close) */
		case SDL_QUIT:
			QUITrequest = TRUE;
			break;
		default:
			/* default is an unhandled event... */
			break;
		}
	}

	/* combine cursorblock and joystick movements into direction pad values with debouncing */
	if(debounce_tmr_dpad > 0)
	{
		debounce_tmr_dpad--;
	}
	else
	{
		dpad.up      = ((joystick.up == TRUE)||(key.up == TRUE));
		dpad.down    = ((joystick.down == TRUE)||(key.down == TRUE));
		dpad.left    = ((joystick.left == TRUE)||(key.left == TRUE));
		dpad.right   = ((joystick.right == TRUE)||(key.right == TRUE));
		dpad.button  = ((joystick.button == TRUE)||(key.button == TRUE));
	}

	/* debounce input devices */
	if(debounce_tmr_mouse > 0)
		debounce_tmr_mouse--;

	if(debounce_tmr_keys > 0)
		debounce_tmr_keys--;
   #if(HAVE_JOYSTICK != _NO_JOYSTICK)
	if(debounce_tmr_joystick > 0)
			debounce_tmr_joystick--;
   #endif
}

/**
 *  @return TRUE if a program shutdown has been requested
 */
BOOLEAN Event_QuitRequested()
{
	return(QUITrequest);
}

/**
 *  @return TRUE if has ESC been pressed
 */
BOOLEAN Event_IsESCPressed()
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
BOOLEAN Event_MouseClicked()
{
	return(mouse.clicked);
}

/**
 * This function implements a callback to allow the event handler to debounce the mouse button.
 */
void Event_DebounceMouse()
{
	mouse.clicked=FALSE;
	mouse.button=0;
	debounce_tmr_mouse = Event_Debounce_timeslices;
}

/**
 *  @return TRUE if directional pad direction up pressed?
 */
BOOLEAN Event_GetDpadUp()
{
	return(dpad.up);
}

/**
 *  @return TRUE if directional pad direction down pressed?
 */
BOOLEAN Event_GetDpadDown()
{
	return(dpad.down);
}

/**
 *  @return TRUE if directional pad direction left pressed?
 */
BOOLEAN Event_GetDpadLeft()
{
	return(dpad.left);
}

/**
 *  @return TRUE if directional pad direction right pressed?
 */
BOOLEAN Event_GetDpadRight()
{
	return(dpad.right);
}

/**
 *  @return TRUE if directional pad firebutton is pressed?
 */
BOOLEAN Event_GetDpadButton()
{
	return(dpad.button);
}

/**
 * This functiondetermines if any directions or firebuttons on the directional pad are pressed
 * @return TRUE if any direction or firebutton on the pad is pressed
 */
BOOLEAN Event_IsDpadPressed()
{
	return((dpad.up == TRUE)||(dpad.down == TRUE)||(dpad.left == TRUE)||(dpad.right == TRUE)||(dpad.button == TRUE));
}

/**
 * This function implements a callback to allow the event handler to debounce any directional pad input.
 */
void Event_DebounceDpad()
{
	debounce_tmr_dpad = Event_Debounce_timeslices;
	dpad.up     = FALSE;
	dpad.down   = FALSE;
	dpad.left   = FALSE;
	dpad.right  = FALSE;
	dpad.button = FALSE;
}

/**
 * This function implements a callback to allow the event handler to debounce all key presses.
 */
void Event_DebounceKeys()
{
	debounce_tmr_keys = Event_Debounce_timeslices;
	ESCpressed = FALSE;
	key.up = FALSE;
	key.down = FALSE;
	key.left = FALSE;
	key.right = FALSE;
	key.button = FALSE;
}

#if(HAVE_JOYSTICK != _NO_JOYSTICK)
/**
 * This function initializes any SDL joystick devices.
 */
void Joystick_Init()
{
#if(HAVE_JOYSTICK != _NO_JOYSTICK)
	joystick.up = FALSE;
	joystick.down = FALSE;
	joystick.left = FALSE;
	joystick.right = FALSE;
	joystick.button = FALSE;
	debounce_tmr_joystick = 0;
#endif
	return;
}
#endif
