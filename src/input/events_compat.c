/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: events_compat.c - Compatibility layer implementation for old event API
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input/events_compat.h"
#include "input/input_manager.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Compatibility state - maintains old API structures */
static struct
{
    bool initialized;
    MOUSE mouse;
    bool ESCpressed;
    bool QUITrequest;
    Uint8 debounce_tmr_mouse;
} compat_state = {0};

/*************
 * CONSTANTS *
 *************/
const Uint8 Event_Debounce_timeslices = 20;

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize the compatibility event system
 */
void Event_Init(void)
{
    if (compat_state.initialized)
    {
        DEBUG_PRINT("Compatibility event system already initialized");
        return;
    }

    /* Initialize input manager with default configuration */
    InputConfig config = {
        .enable_mouse = true,
        .enable_touch = false,
        .debounce_time = Event_Debounce_timeslices
    };

    if (!InputManager_Init(&config))
    {
        LOG_ERROR("Failed to initialize input manager for compatibility layer");
        return;
    }

    /* Initialize compatibility state */
    compat_state.mouse.x = 0;
    compat_state.mouse.y = 0;
    compat_state.mouse.button = 0;
    compat_state.mouse.clicked = false;
    compat_state.debounce_tmr_mouse = Event_Debounce_timeslices;

    compat_state.ESCpressed = false;
    compat_state.QUITrequest = false;

    compat_state.initialized = true;
    LOG_INFO("Compatibility event system initialized successfully");
}

/**
 * Process input events using the new unified input system
 */
void Event_ProcessInput(void)
{
    if (!compat_state.initialized)
    {
        return;
    }

    /* Process events using the new input manager */
    InputManager_ProcessEvents();

    /* Update compatibility state from new input manager */
    const MouseState* mouse_state = InputManager_GetMouseState();

    if (mouse_state)
    {
        /* Store previous state for change detection */
        uint16_t prev_x = compat_state.mouse.x;
        uint16_t prev_y = compat_state.mouse.y;
        uint8_t prev_button = compat_state.mouse.button;
        bool prev_clicked = compat_state.mouse.clicked;
        
        compat_state.mouse.x = mouse_state->x;
        compat_state.mouse.y = mouse_state->y;
        compat_state.mouse.button = mouse_state->button;
        compat_state.mouse.clicked = mouse_state->clicked;
        
        /* Debug logging for mouse events */
        if (compat_state.mouse.clicked != prev_clicked || 
            compat_state.mouse.button != prev_button ||
            compat_state.mouse.x != prev_x || 
            compat_state.mouse.y != prev_y)
        {
            DEBUG_PRINT("Mouse state changed: x=%d, y=%d, button=%d, clicked=%s", 
                       compat_state.mouse.x, compat_state.mouse.y, 
                       compat_state.mouse.button, 
                       compat_state.mouse.clicked ? "true" : "false");
        }
    }

    /* Update quit and ESC states */
    compat_state.QUITrequest = InputManager_IsQuitRequested();

    /* Update debounce timers */
    if (compat_state.debounce_tmr_mouse > 0)
        compat_state.debounce_tmr_mouse--;

}

/**
 * Check if quit was requested
 */
bool Event_QuitRequested(void)
{
    return compat_state.QUITrequest;
}

/**
 * Check if ESC key was pressed
 */
bool Event_IsESCPressed(void)
{
    return compat_state.ESCpressed;
}

/**
 * Get mouse X position
 */
Uint16 Event_GetMouseX(void)
{
    return compat_state.mouse.x;
}

/**
 * Get mouse Y position
 */
Uint16 Event_GetMouseY(void)
{
    return compat_state.mouse.y;
}

/**
 * Get mouse button
 */
Uint8 Event_GetMouseButton(void)
{
    return compat_state.mouse.button;
}

/**
 * Check if mouse was clicked
 */
bool Event_MouseClicked(void)
{
    return compat_state.mouse.clicked;
}

/**
 * Debounce mouse input
 */
void Event_DebounceMouse(void)
{
    if (!compat_state.initialized)
    {
        return;
    }

    InputManager_DebounceMouse();
    compat_state.mouse.clicked = false;
    compat_state.mouse.button = 0;
    compat_state.debounce_tmr_mouse = Event_Debounce_timeslices;
}
