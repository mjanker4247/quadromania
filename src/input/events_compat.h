/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: events_compat.h - Compatibility layer for old event API
 * last Modified: 2024-12-19
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

#ifndef __EVENTS_COMPAT_H
#define __EVENTS_COMPAT_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "input/events.h" /* Include old event types for compatibility */

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the compatibility event system
 * This replaces the old Event_Init() function
 */
void Event_Init(void);

/**
 * Process input events using the new unified input system
 * This replaces the old Event_ProcessInput() function
 */
void Event_ProcessInput(void);

/**
 * Check if quit was requested
 * This replaces the old Event_QuitRequested() function
 */
bool Event_QuitRequested(void);

/**
 * Check if ESC key was pressed
 * This replaces the old Event_IsESCPressed() function
 */
bool Event_IsESCPressed(void);

/**
 * Get mouse X position
 * This replaces the old Event_GetMouseX() function
 */
Uint16 Event_GetMouseX(void);

/**
 * Get mouse Y position
 * This replaces the old Event_GetMouseY() function
 */
Uint16 Event_GetMouseY(void);

/**
 * Get mouse button
 * This replaces the old Event_GetMouseButton() function
 */
Uint8 Event_GetMouseButton(void);

/**
 * Check if mouse was clicked
 * This replaces the old Event_MouseClicked() function
 */
bool Event_MouseClicked(void);

/**
 * Get directional pad up state
 * This replaces the old Event_GetDpadUp() function
 */
bool Event_GetDpadUp(void);

/**
 * Get directional pad down state
 * This replaces the old Event_GetDpadDown() function
 */
bool Event_GetDpadDown(void);

/**
 * Get directional pad left state
 * This replaces the old Event_GetDpadLeft() function
 */
bool Event_GetDpadLeft(void);

/**
 * Get directional pad right state
 * This replaces the old Event_GetDpadRight() function
 */
bool Event_GetDpadRight(void);

/**
 * Get directional pad button state
 * This replaces the old Event_GetDpadButton() function
 */
bool Event_GetDpadButton(void);

/**
 * Check if any directional pad input is pressed
 * This replaces the old Event_IsDpadPressed() function
 */
bool Event_IsDpadPressed(void);

/**
 * Debounce directional pad input
 * This replaces the old Event_DebounceDpad() function
 */
void Event_DebounceDpad(void);

/**
 * Debounce mouse input
 * This replaces the old Event_DebounceMouse() function
 */
void Event_DebounceMouse(void);

/**
 * Debounce keyboard input
 * This replaces the old Event_DebounceKeys() function
 */
void Event_DebounceKeys(void);

#if(HAVE_JOYSTICK != _NO_JOYSTICK)
/**
 * Initialize joystick support
 * This replaces the old Joystick_Init() function
 */
void Joystick_Init(void);
#endif

#endif /* __EVENTS_COMPAT_H */ 