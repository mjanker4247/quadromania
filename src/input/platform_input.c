/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: platform_input.c - Platform abstraction implementation
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
#include "input/platform_input.h"
#include "input/platform/sdl2_input.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Platform input system state */
static struct
{
    bool initialized;
    PlatformInputConfig config;
    PlatformInputCapabilities capabilities;
} platform_input_state = {0};

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize platform-specific input system
 */
bool PlatformInput_Init(const PlatformInputConfig* config)
{
    if (platform_input_state.initialized)
    {
        DEBUG_PRINT("Platform input system already initialized");
        return true;
    }

    /* Store configuration */
    if (config)
    {
        memcpy(&platform_input_state.config, config, sizeof(PlatformInputConfig));
    }
    else
    {
        /* Use default configuration */
        platform_input_state.config = PlatformInput_DetectPlatform();
    }

    /* Initialize SDL2 input system */
    if (!SDL2Input_Init(&platform_input_state.config))
    {
        LOG_ERROR("Failed to initialize SDL2 input system");
        return false;
    }

    /* Get capabilities from platform implementation */
    platform_input_state.capabilities = SDL2Input_GetCapabilities();

    platform_input_state.initialized = true;
    LOG_INFO("Platform input system initialized successfully");
    
    return true;
}

/**
 * Shutdown platform-specific input system
 */
void PlatformInput_Shutdown(void)
{
    if (!platform_input_state.initialized)
    {
        return;
    }

    SDL2Input_Shutdown();
    memset(&platform_input_state, 0, sizeof(platform_input_state));
    LOG_INFO("Platform input system shutdown complete");
}

/**
 * Get platform input capabilities
 */
PlatformInputCapabilities PlatformInput_GetCapabilities(void)
{
    return platform_input_state.capabilities;
}

/**
 * Process platform-specific events and convert to unified events
 */
int PlatformInput_ProcessEvents(PlatformInputEvent* event_buffer, int buffer_size)
{
    if (!platform_input_state.initialized || !event_buffer || buffer_size <= 0)
    {
        return 0;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_ProcessEvents(event_buffer, buffer_size);
}

/**
 * Convert platform-specific event to unified event
 */
bool PlatformInput_ConvertEvent(const void* platform_event, InputEvent* unified_event)
{
    if (!platform_input_state.initialized || !platform_event || !unified_event)
    {
        return false;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_ConvertEvent((const SDL_Event*)platform_event, unified_event);
}

/**
 * Get platform-specific key code for unified key code
 */
uint32_t PlatformInput_GetPlatformKeyCode(uint32_t unified_key_code)
{
    if (!platform_input_state.initialized)
    {
        return unified_key_code;
    }

    /* Delegate to SDL2 implementation */
    return (uint32_t)SDL2Input_GetSDLKeyCode(unified_key_code);
}

/**
 * Get unified key code for platform-specific key code
 */
uint32_t PlatformInput_GetUnifiedKeyCode(uint32_t platform_key_code)
{
    if (!platform_input_state.initialized)
    {
        return platform_key_code;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_GetUnifiedKeyCode((SDL_Keycode)platform_key_code);
}

/**
 * Set platform-specific input configuration
 */
void PlatformInput_SetConfig(const PlatformInputConfig* config)
{
    if (!platform_input_state.initialized || !config)
    {
        return;
    }

    memcpy(&platform_input_state.config, config, sizeof(PlatformInputConfig));
    DEBUG_PRINT("Platform input configuration updated");
}

/**
 * Get current platform input configuration
 */
PlatformInputConfig PlatformInput_GetConfig(void)
{
    return platform_input_state.config;
}

/**
 * Detect current platform and return appropriate configuration
 */
PlatformInputConfig PlatformInput_DetectPlatform(void)
{
    /* Delegate to SDL2 implementation for platform detection */
    return SDL2Input_DetectPlatform();
}

/**
 * Check if platform supports specific input feature
 */
bool PlatformInput_SupportsFeature(const char* feature)
{
    if (!platform_input_state.initialized || !feature)
    {
        return false;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_SupportsFeature(feature);
}

/**
 * Get platform-specific input device name
 */
const char* PlatformInput_GetDeviceName(InputDeviceType device_type, int device_index)
{
    if (!platform_input_state.initialized)
    {
        return NULL;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_GetDeviceName(device_type, device_index);
}

/**
 * Get platform-specific input device count
 */
int PlatformInput_GetDeviceCount(InputDeviceType device_type)
{
    if (!platform_input_state.initialized)
    {
        return 0;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_GetDeviceCount(device_type);
}

/**
 * Enable or disable platform-specific input device
 */
bool PlatformInput_SetDeviceEnabled(InputDeviceType device_type, int device_index, bool enabled)
{
    if (!platform_input_state.initialized)
    {
        return false;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_SetDeviceEnabled(device_type, device_index, enabled);
}

/**
 * Check if platform-specific input device is enabled
 */
bool PlatformInput_IsDeviceEnabled(InputDeviceType device_type, int device_index)
{
    if (!platform_input_state.initialized)
    {
        return false;
    }

    /* Delegate to SDL2 implementation */
    return SDL2Input_IsDeviceEnabled(device_type, device_index);
} 