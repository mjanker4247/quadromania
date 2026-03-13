/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: events.h - Simplified input event API for mouse and touch
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

#ifndef __EVENT_H
#define __EVENT_H
#include <SDL2/SDL.h>
#include <stdbool.h>

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * This data structure represents the mouse/stylus input state.
 */
typedef struct
{
    uint16_t x, y;     /** mouse coordinates in pixels */
    uint8_t button;    /** mouse button number */
    bool clicked;      /** mouse clicked? */
} MouseState;

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
} DpadState;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the event system
 */
void Event_Init(void);

/**
 * Process all pending input events
 * This should be called once per frame
 */
void Event_ProcessInput(void);

/**
 * Check if quit was requested
 * @return true if quit was requested
 */
bool Event_QuitRequested(void);

/**
 * Get mouse X position
 * @return mouse X coordinate
 */
uint16_t Event_GetMouseX(void);

/**
 * Get mouse Y position
 * @return mouse Y coordinate
 */
uint16_t Event_GetMouseY(void);

/**
 * Get mouse button
 * @return mouse button number
 */
uint8_t Event_GetMouseButton(void);

/**
 * Check if mouse was clicked
 * @return true if mouse was clicked
 */
bool Event_MouseClicked(void);

/**
 * Reset the clicked state after processing
 */
void Event_ResetClicked(void);

/**
 * Debounce mouse input
 */
void Event_DebounceMouse(void);

/**
 * Get the current mouse state
 * @return Pointer to current mouse state
 */
const MouseState* Event_GetMouseState(void);

#endif /* __EVENT_H */
