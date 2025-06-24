/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: sdl2_input.c - SDL2 platform-specific input implementation
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
#include <SDL2/SDL.h>
#include "input/platform/sdl2_input.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* SDL2 input system state */
static SDL2InputState sdl2_input_state = {0};

/*************
 * CONSTANTS *
 *************/

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize SDL2 input system
 */
bool SDL2Input_Init(const PlatformInputConfig* config)
{
    if (sdl2_input_state.initialized)
    {
        DEBUG_PRINT("SDL2 input system already initialized");
        return true;
    }

    /* Store configuration */
    if (config)
    {
        memcpy(&sdl2_input_state.config, config, sizeof(PlatformInputConfig));
    }
    else
    {
        /* Use default configuration */
        sdl2_input_state.config = SDL2Input_DetectPlatform();
    }

    sdl2_input_state.renderer = NULL;  /* Will be set later */

    sdl2_input_state.initialized = true;
    LOG_INFO("SDL2 input system initialized successfully");
    
    return true;
}

/**
 * Set the renderer for coordinate conversion
 */
void SDL2Input_SetRenderer(SDL_Renderer* renderer)
{
    sdl2_input_state.renderer = renderer;
    DEBUG_PRINT("SDL2 input renderer set for coordinate conversion");
}

/**
 * Shutdown SDL2 input system
 */
void SDL2Input_Shutdown(void)
{
    if (!sdl2_input_state.initialized)
    {
        return;
    }

    memset(&sdl2_input_state, 0, sizeof(SDL2InputState));
    LOG_INFO("SDL2 input system shutdown complete");
}

/**
 * Process SDL2 events and convert to unified events
 */
int SDL2Input_ProcessEvents(PlatformInputEvent* event_buffer, int buffer_size)
{
    if (!sdl2_input_state.initialized || !event_buffer || buffer_size <= 0)
    {
        return 0;
    }

    int events_processed = 0;
    SDL_Event sdl_event;

    /* Process all pending SDL2 events */
    while (SDL_PollEvent(&sdl_event) && events_processed < buffer_size)
    {
        if (SDL2Input_ConvertEvent(&sdl_event, &event_buffer[events_processed].unified_event))
        {
            event_buffer[events_processed].platform_event_type = sdl_event.type;
            /* Don't store pointer to local variable - just store the event type */
            event_buffer[events_processed].platform_event_data = NULL;
            events_processed++;
        }
    }

    return events_processed;
}

/**
 * Convert SDL2 event to unified event
 */
bool SDL2Input_ConvertEvent(const SDL_Event* sdl_event, InputEvent* unified_event)
{
    if (!sdl_event || !unified_event)
    {
        return false;
    }

    memset(unified_event, 0, sizeof(InputEvent));

    switch (sdl_event->type)
    {
    case SDL_MOUSEMOTION:
        unified_event->type = INPUT_EVENT_MOUSE_MOVE;
        if (sdl2_input_state.renderer)
        {
            SDL_RenderWindowToLogical(sdl2_input_state.renderer, sdl_event->motion.x, sdl_event->motion.y, 
                                     &unified_event->data.mouse.x, &unified_event->data.mouse.y);
        }
        else
        {
            unified_event->data.mouse.x = sdl_event->motion.x;
            unified_event->data.mouse.y = sdl_event->motion.y;
        }
        unified_event->data.mouse.button = 0;
        return true;

    case SDL_MOUSEBUTTONDOWN:
        unified_event->type = INPUT_EVENT_MOUSE_DOWN;
        if (sdl2_input_state.renderer)
        {
            SDL_RenderWindowToLogical(sdl2_input_state.renderer, sdl_event->button.x, sdl_event->button.y, 
                                     &unified_event->data.mouse.x, &unified_event->data.mouse.y);
        }
        else
        {
            unified_event->data.mouse.x = sdl_event->button.x;
            unified_event->data.mouse.y = sdl_event->button.y;
        }
        unified_event->data.mouse.button = sdl_event->button.button;
        return true;

    case SDL_MOUSEBUTTONUP:
        unified_event->type = INPUT_EVENT_MOUSE_UP;
        if (sdl2_input_state.renderer)
        {
            SDL_RenderWindowToLogical(sdl2_input_state.renderer, sdl_event->button.x, sdl_event->button.y, 
                                     &unified_event->data.mouse.x, &unified_event->data.mouse.y);
        }
        else
        {
            unified_event->data.mouse.x = sdl_event->button.x;
            unified_event->data.mouse.y = sdl_event->button.y;
        }
        unified_event->data.mouse.button = sdl_event->button.button;
        return true;

    case SDL_QUIT:
        unified_event->type = INPUT_EVENT_QUIT;
        return true;

    default:
        return false; /* Unhandled event type */
    }
}

/**
 * Get SDL2 input capabilities
 */
PlatformInputCapabilities SDL2Input_GetCapabilities(void)
{
    PlatformInputCapabilities capabilities = {0};

    capabilities.has_mouse = true;
    capabilities.has_touch = false; /* SDL2 touch support can be added */
    capabilities.has_accelerometer = false;
    capabilities.has_gyroscope = false;
    capabilities.max_touch_points = 0;

    return capabilities;
}

/**
 * Detect SDL2 platform and return appropriate configuration
 */
PlatformInputConfig SDL2Input_DetectPlatform(void)
{
    PlatformInputConfig config = {0};

    config.capabilities = SDL2Input_GetCapabilities();
    config.enable_raw_input = false;
    config.enable_relative_mouse = false;
    config.enable_touch_emulation = false;

    return config;
}

/**
 * Check if SDL2 supports specific input feature
 */
bool SDL2Input_SupportsFeature(const char* feature)
{
    if (!feature)
    {
        return false;
    }

    if (strcmp(feature, "mouse") == 0)
    {
        return true;
    }
    else if (strcmp(feature, "touch") == 0)
    {
        return false; /* SDL2 touch support can be added */
    }

    return false;
}

/**
 * Get SDL2 input device name
 */
const char* SDL2Input_GetDeviceName(InputDeviceType device_type, int device_index)
{
    if (!sdl2_input_state.initialized)
    {
        return NULL;
    }

    switch (device_type)
    {
    case INPUT_DEVICE_MOUSE:
        return "SDL2 Mouse";
    default:
        break;
    }
    return NULL;
}

/**
 * Get SDL2 input device count
 */
int SDL2Input_GetDeviceCount(InputDeviceType device_type)
{
    if (!sdl2_input_state.initialized)
    {
        return 0;
    }

    switch (device_type)
    {
    case INPUT_DEVICE_MOUSE:
        return 1;
    default:
        return 0;
    }
}

/**
 * Enable or disable SDL2 input device
 */
bool SDL2Input_SetDeviceEnabled(InputDeviceType device_type, int device_index, bool enabled)
{
    if (!sdl2_input_state.initialized)
    {
        return false;
    }

    switch (device_type)
    {
    case INPUT_DEVICE_MOUSE:
        return true; /* Always enabled in SDL2 */
    default:
        return false;
    }
}

/**
 * Check if SDL2 input device is enabled
 */
bool SDL2Input_IsDeviceEnabled(InputDeviceType device_type, int device_index)
{
    if (!sdl2_input_state.initialized)
    {
        return false;
    }

    switch (device_type)
    {
    case INPUT_DEVICE_MOUSE:
        return true; /* Always enabled in SDL2 */
    default:
        return false;
    }
} 