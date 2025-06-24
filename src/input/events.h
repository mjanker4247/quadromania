/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: event.h - symbolic constants, data types and function prototypes for event handling
 * last Modified: 18.11.2010 : 19:05
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

#ifndef __EVENT_H
#define __EVENT_H
#include "common/datatypes.h"
#include "common/sysconfig.h"
#include <stdbool.h>


	/*************
	 * CONSTANTS *
     *************/
#if(HAVE_JOYSTICK != _NO_JOYSTICK)
	#if(HAVE_JOYSTICK ==_GP2X_JOYSTICK)

		#define GP2X_BUTTON_UP              (0)
		#define GP2X_BUTTON_DOWN            (4)
		#define GP2X_BUTTON_LEFT            (2)
		#define GP2X_BUTTON_RIGHT           (6)
		#define GP2X_BUTTON_UPLEFT          (1)
		#define GP2X_BUTTON_UPRIGHT         (7)
		#define GP2X_BUTTON_DOWNLEFT        (3)
		#define GP2X_BUTTON_DOWNRIGHT       (5)
		#define GP2X_BUTTON_CLICK           (18)
		#define GP2X_BUTTON_A               (12)
		#define GP2X_BUTTON_B               (13)
		#define GP2X_BUTTON_X               (14)
		#define GP2X_BUTTON_Y               (15)
		#define GP2X_BUTTON_L               (10)
		#define GP2X_BUTTON_R               (11)
		#define GP2X_BUTTON_MENU            (8)
		#define GP2X_BUTTON_SELECT          (9)
		#define GP2X_BUTTON_VOLUP           (16)
		#define GP2X_BUTTON_VOLDOWN         (17)

		#define JOYSTICK_BUTTON_ESC         GP2X_BUTTON_MENU
	#else
		#define JOYSTICK_BUTTON_ESC         3
	#endif
#endif

	/**************************
	 * DATA TYPE DECLARATIONS *
     **************************/

    /**
     * This data structure represents the collect input and state of the mouse/stylus input device.
     */
	typedef struct
	{
		Uint16 x, y; /** mouse coordinates in pixels */
		Uint8 button; /** mouse button number */
		bool clicked; /** mouse clicked? */
	} MOUSE;

	/**
	 * This data structure represents the state of the virtual directional pad.
	 * Each member describes whether the direction or firebutton is pressed.
	 */
	typedef struct
	{
		bool up;
		bool down;
		bool left;
		bool right;
		bool button;
	} DPAD;

	/**************
	 * PROTOTYPES *
	 **************/
	void Event_Init(void);
	void Event_ProcessInput(void);
	bool Event_QuitRequested(void);
	bool Event_IsESCPressed(void);
	Uint16 Event_GetMouseX(void);
	Uint16 Event_GetMouseY(void);
	Uint8 Event_GetMouseButton(void);
	bool Event_MouseClicked(void);
	bool Event_GetDpadUp(void);
	bool Event_GetDpadDown(void);
	bool Event_GetDpadLeft(void);
	bool Event_GetDpadRight(void);
	bool Event_GetDpadButton(void);
	bool Event_IsDpadPressed(void);
	void Event_DebounceDpad(void);
	void Event_DebounceMouse(void);
	void Event_DebounceKeys(void);

#if(HAVE_JOYSTICK != _NO_JOYSTICK)
	void Joystick_Init(void);
#endif

#endif /* __EVENT_H */
