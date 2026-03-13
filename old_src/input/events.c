/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: events.c - Simplified input event API for mouse and touch
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

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input/events.h"
#include "graphics/renderer.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Simplified input state structure */
static struct
{
    bool initialized;
    MouseState mouse_state;
    bool quit_requested;
    uint32_t last_event_time;
} input_state = {0};

/*************
 * CONSTANTS *
 *************/
const uint8_t EVENT_DEBOUNCE_TIMESLICES = 1;

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize the event system
 */
void Event_Init(void)
{
    memset(&input_state, 0, sizeof(input_state));
    input_state.initialized = true;
    input_state.last_event_time = SDL_GetTicks();
    LOG_INFO("Event system initialized");
}

/**
 * Process all pending input events
 * This should be called once per frame
 */
void Event_ProcessInput(void)
{
    if (!input_state.initialized)
    {
        return;
    }

    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            input_state.quit_requested = true;
            DEBUG_PRINT("Quit event received");
            break;
            
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                DEBUG_PRINT("ESC key pressed");
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            /* Do not clear clicked here — it must persist until Event_ResetClicked()
               is called by the state handler. Clearing here would race against
               state_manager_update() when click+release fall in the same frame. */
            DEBUG_PRINT("Mouse button up: button=%d", event.button.button);
            break;

        case SDL_MOUSEBUTTONDOWN:
            {
                int lx, ly;
                Graphics_WindowToLogical(event.button.x, event.button.y, &lx, &ly);
                input_state.mouse_state.x = (uint16_t)lx;
                input_state.mouse_state.y = (uint16_t)ly;
                input_state.mouse_state.button = event.button.button;
                input_state.mouse_state.clicked = true;
                DEBUG_PRINT("Mouse clicked: x=%d, y=%d, button=%d",
                           input_state.mouse_state.x, input_state.mouse_state.y, input_state.mouse_state.button);
            }
            break;

        case SDL_MOUSEMOTION:
            {
                int lx, ly;
                Graphics_WindowToLogical(event.motion.x, event.motion.y, &lx, &ly);
                input_state.mouse_state.x = (uint16_t)lx;
                input_state.mouse_state.y = (uint16_t)ly;
            }
            break;
            
        case SDL_FINGERDOWN:
            /* Touch input: finger coords are 0.0-1.0 normalised to the window.
               Multiply by logical dimensions since window == logical size (640x480). */
            input_state.mouse_state.x = (uint16_t)(event.tfinger.x * Graphics_GetScreenWidth());
            input_state.mouse_state.y = (uint16_t)(event.tfinger.y * Graphics_GetScreenHeight());
            input_state.mouse_state.button = 1;
            input_state.mouse_state.clicked = true;
            DEBUG_PRINT("Touch down: x=%d, y=%d", input_state.mouse_state.x, input_state.mouse_state.y);
            break;

        case SDL_FINGERUP:
            /* Do not clear clicked — same reasoning as SDL_MOUSEBUTTONUP */
            DEBUG_PRINT("Touch up");
            break;

        case SDL_FINGERMOTION:
            input_state.mouse_state.x = (uint16_t)(event.tfinger.x * Graphics_GetScreenWidth());
            input_state.mouse_state.y = (uint16_t)(event.tfinger.y * Graphics_GetScreenHeight());
            break;
        }
        
        input_state.last_event_time = SDL_GetTicks();
    }
}

/**
 * Check if quit was requested
 * @return true if quit was requested
 */
bool Event_QuitRequested(void)
{
    return input_state.quit_requested;
}

/**
 * Get mouse X position
 * @return mouse X coordinate
 */
uint16_t Event_GetMouseX(void)
{
    return input_state.mouse_state.x;
}

/**
 * Get mouse Y position
 * @return mouse Y coordinate
 */
uint16_t Event_GetMouseY(void)
{
    return input_state.mouse_state.y;
}

/**
 * Get mouse button
 * @return mouse button number
 */
uint8_t Event_GetMouseButton(void)
{
    return input_state.mouse_state.button;
}

/**
 * Check if mouse was clicked
 * @return true if mouse was clicked
 */
bool Event_MouseClicked(void)
{
    return input_state.mouse_state.clicked;
}

/**
 * Reset the clicked state after processing
 */
void Event_ResetClicked(void)
{
    input_state.mouse_state.clicked = false;
    input_state.mouse_state.button = 0;
}

/**
 * Debounce mouse input
 */
void Event_DebounceMouse(void)
{
    input_state.mouse_state.clicked = false;
    input_state.mouse_state.button = 0;
}

/**
 * Get the current mouse state
 * @return Pointer to current mouse state
 */
const MouseState* Event_GetMouseState(void)
{
    return &input_state.mouse_state;
}
