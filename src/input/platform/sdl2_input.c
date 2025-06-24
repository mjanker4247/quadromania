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

/* Key code mappings */
#define SDL_KEY_UP         SDLK_UP
#define SDL_KEY_DOWN       SDLK_DOWN
#define SDL_KEY_LEFT       SDLK_LEFT
#define SDL_KEY_RIGHT      SDLK_RIGHT
#define SDL_KEY_CTRL       SDLK_LCTRL
#define SDL_KEY_ALT        SDLK_LALT
#define SDL_KEY_ESCAPE     SDLK_ESCAPE
#define SDL_KEY_KP_PLUS    SDLK_KP_PLUS
#define SDL_KEY_KP_MINUS   SDLK_KP_MINUS

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

    /* Initialize joystick array */
    sdl2_input_state.joysticks = NULL;
    sdl2_input_state.joystick_count = 0;

    /* Initialize SDL2 joysticks if enabled */
    if (sdl2_input_state.config.capabilities.has_joystick)
    {
        if (!SDL2Input_InitJoysticks())
        {
            DEBUG_PRINT("Failed to initialize SDL2 joysticks (continuing without joystick support)");
        }
    }

    sdl2_input_state.initialized = true;
    LOG_INFO("SDL2 input system initialized successfully");
    
    return true;
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

    SDL2Input_ShutdownJoysticks();
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
            event_buffer[events_processed].platform_event_data = &sdl_event;
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
    case SDL_KEYDOWN:
        unified_event->type = INPUT_EVENT_KEY_DOWN;
        unified_event->data.key.key_code = SDL2Input_GetUnifiedKeyCode(sdl_event->key.keysym.sym);
        unified_event->data.key.is_repeat = sdl_event->key.repeat;
        return true;

    case SDL_KEYUP:
        unified_event->type = INPUT_EVENT_KEY_UP;
        unified_event->data.key.key_code = SDL2Input_GetUnifiedKeyCode(sdl_event->key.keysym.sym);
        unified_event->data.key.is_repeat = false;
        return true;

    case SDL_MOUSEMOTION:
        unified_event->type = INPUT_EVENT_MOUSE_MOVE;
        unified_event->data.mouse.x = sdl_event->motion.x;
        unified_event->data.mouse.y = sdl_event->motion.y;
        unified_event->data.mouse.button = 0;
        return true;

    case SDL_MOUSEBUTTONDOWN:
        unified_event->type = INPUT_EVENT_MOUSE_DOWN;
        unified_event->data.mouse.x = sdl_event->button.x;
        unified_event->data.mouse.y = sdl_event->button.y;
        unified_event->data.mouse.button = sdl_event->button.button;
        return true;

    case SDL_MOUSEBUTTONUP:
        unified_event->type = INPUT_EVENT_MOUSE_UP;
        unified_event->data.mouse.x = sdl_event->button.x;
        unified_event->data.mouse.y = sdl_event->button.y;
        unified_event->data.mouse.button = sdl_event->button.button;
        return true;

    case SDL_JOYAXISMOTION:
        unified_event->type = INPUT_EVENT_JOYSTICK_AXIS;
        unified_event->data.joystick_axis.axis = sdl_event->jaxis.axis;
        unified_event->data.joystick_axis.value = sdl_event->jaxis.value;
        return true;

    case SDL_JOYBUTTONDOWN:
        unified_event->type = INPUT_EVENT_JOYSTICK_BUTTON_DOWN;
        unified_event->data.joystick_button.button = sdl_event->jbutton.button;
        return true;

    case SDL_JOYBUTTONUP:
        unified_event->type = INPUT_EVENT_JOYSTICK_BUTTON_UP;
        unified_event->data.joystick_button.button = sdl_event->jbutton.button;
        return true;

    case SDL_QUIT:
        unified_event->type = INPUT_EVENT_QUIT;
        return true;

    case SDL_JOYDEVICEADDED:
        SDL2Input_HandleJoystickAdded(sdl_event->jdevice.which);
        return false; /* Don't pass device events to game */

    case SDL_JOYDEVICEREMOVED:
        SDL2Input_HandleJoystickRemoved(sdl_event->jdevice.which);
        return false; /* Don't pass device events to game */

    default:
        return false; /* Unhandled event type */
    }
}

/**
 * Get SDL2 key code for unified key code
 */
SDL_Keycode SDL2Input_GetSDLKeyCode(uint32_t unified_key_code)
{
    /* Simple mapping - can be enhanced with a lookup table */
    switch (unified_key_code)
    {
    case 0x26: return SDL_KEY_UP;
    case 0x28: return SDL_KEY_DOWN;
    case 0x25: return SDL_KEY_LEFT;
    case 0x27: return SDL_KEY_RIGHT;
    case 0x11: return SDL_KEY_CTRL;
    case 0x12: return SDL_KEY_ALT;
    case 0x1B: return SDL_KEY_ESCAPE;
    case 0x6B: return SDL_KEY_KP_PLUS;
    case 0x6D: return SDL_KEY_KP_MINUS;
    default: return (SDL_Keycode)unified_key_code;
    }
}

/**
 * Get unified key code for SDL2 key code
 */
uint32_t SDL2Input_GetUnifiedKeyCode(SDL_Keycode sdl_key_code)
{
    /* Simple mapping - can be enhanced with a lookup table */
    switch (sdl_key_code)
    {
    case SDL_KEY_UP: return 0x26;
    case SDL_KEY_DOWN: return 0x28;
    case SDL_KEY_LEFT: return 0x25;
    case SDL_KEY_RIGHT: return 0x27;
    case SDL_KEY_CTRL: return 0x11;
    case SDL_KEY_ALT: return 0x12;
    case SDL_KEY_ESCAPE: return 0x1B;
    case SDL_KEY_KP_PLUS: return 0x6B;
    case SDL_KEY_KP_MINUS: return 0x6D;
    default: return (uint32_t)sdl_key_code;
    }
}

/**
 * Get SDL2 input capabilities
 */
PlatformInputCapabilities SDL2Input_GetCapabilities(void)
{
    PlatformInputCapabilities capabilities = {0};

    capabilities.has_keyboard = true;
    capabilities.has_mouse = true;
    capabilities.has_joystick = (SDL_NumJoysticks() > 0);
    capabilities.has_touch = false; /* SDL2 touch support can be added */
    capabilities.has_accelerometer = false;
    capabilities.has_gyroscope = false;
    capabilities.max_joysticks = SDL_NumJoysticks();
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

    if (strcmp(feature, "keyboard") == 0)
    {
        return true;
    }
    else if (strcmp(feature, "mouse") == 0)
    {
        return true;
    }
    else if (strcmp(feature, "joystick") == 0)
    {
        return SDL_NumJoysticks() > 0;
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
    case INPUT_DEVICE_JOYSTICK:
        if (device_index >= 0 && device_index < sdl2_input_state.joystick_count)
        {
            return sdl2_input_state.joysticks[device_index].name;
        }
        break;

    case INPUT_DEVICE_KEYBOARD:
        return "SDL2 Keyboard";

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
    case INPUT_DEVICE_JOYSTICK:
        return sdl2_input_state.joystick_count;

    case INPUT_DEVICE_KEYBOARD:
        return 1;

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
    case INPUT_DEVICE_JOYSTICK:
        if (device_index >= 0 && device_index < sdl2_input_state.joystick_count)
        {
            sdl2_input_state.joysticks[device_index].enabled = enabled;
            return true;
        }
        break;

    default:
        /* Keyboard and mouse are always enabled in SDL2 */
        return true;
    }

    return false;
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
    case INPUT_DEVICE_JOYSTICK:
        if (device_index >= 0 && device_index < sdl2_input_state.joystick_count)
        {
            return sdl2_input_state.joysticks[device_index].enabled;
        }
        return false;

    case INPUT_DEVICE_KEYBOARD:
    case INPUT_DEVICE_MOUSE:
        return true; /* Always enabled in SDL2 */

    default:
        return false;
    }
}

/**
 * Get SDL2 joystick device by index
 */
SDL2JoystickDevice* SDL2Input_GetJoystickDevice(int index)
{
    if (!sdl2_input_state.initialized || index < 0 || index >= sdl2_input_state.joystick_count)
    {
        return NULL;
    }

    return &sdl2_input_state.joysticks[index];
}

/**
 * Open SDL2 joystick by index
 * @param index Joystick index
 * @return true on success, false on failure
 */
bool SDL2Input_OpenJoystick(int index)
{
    if (!sdl2_input_state.initialized || index < 0 || index >= sdl2_input_state.joystick_count)
    {
        return false;
    }

    SDL2JoystickDevice* device = &sdl2_input_state.joysticks[index];
    
    device->joystick = SDL_JoystickOpen(index);
    if (!device->joystick)
    {
        return false;
    }

    return true;
}

/**
 * Initialize SDL2 joystick devices
 */
bool SDL2Input_InitJoysticks(void)
{
    int joystick_count = SDL_NumJoysticks();
    
    if (joystick_count <= 0)
    {
        LOG_INFO("No SDL2 joysticks detected");
        return true;
    }

    /* Allocate joystick array */
    sdl2_input_state.joysticks = malloc(joystick_count * sizeof(SDL2JoystickDevice));
    if (!sdl2_input_state.joysticks)
    {
        LOG_ERROR("Failed to allocate joystick array");
        return false;
    }

    sdl2_input_state.joystick_count = joystick_count;

    /* Initialize each joystick */
    for (int i = 0; i < joystick_count; i++)
    {
        SDL2JoystickDevice* device = &sdl2_input_state.joysticks[i];
        
        if (!SDL2Input_OpenJoystick(i))
        {
            DEBUG_PRINT("Failed to open SDL2 joystick %d", i);
            continue;
        }

        /* Get device information */
        const char* name = SDL_JoystickName(device->joystick);
        if (name)
        {
            strncpy(device->name, name, sizeof(device->name) - 1);
            device->name[sizeof(device->name) - 1] = '\0';
        }
        else
        {
            snprintf(device->name, sizeof(device->name), "Joystick %d", i);
        }

        device->enabled = true;
        device->button_count = SDL_JoystickNumButtons(device->joystick);
        device->axis_count = SDL_JoystickNumAxes(device->joystick);
        device->hat_count = SDL_JoystickNumHats(device->joystick);

        LOG_INFO("SDL2 joystick %d: %s (%d buttons, %d axes, %d hats)", 
                i, device->name, device->button_count, device->axis_count, device->hat_count);
    }

    return true;
}

/**
 * Shutdown SDL2 joystick devices
 */
void SDL2Input_ShutdownJoysticks(void)
{
    if (sdl2_input_state.joysticks)
    {
        for (int i = 0; i < sdl2_input_state.joystick_count; i++)
        {
            if (sdl2_input_state.joysticks[i].joystick)
            {
                SDL_JoystickClose(sdl2_input_state.joysticks[i].joystick);
            }
        }

        free(sdl2_input_state.joysticks);
        sdl2_input_state.joysticks = NULL;
        sdl2_input_state.joystick_count = 0;
    }
}

/**
 * Handle SDL2 joystick addition event
 */
void SDL2Input_HandleJoystickAdded(int device_index)
{
    LOG_INFO("SDL2 joystick %d added", device_index);
    
    /* Reinitialize joysticks to include the new device */
    SDL2Input_ShutdownJoysticks();
    SDL2Input_InitJoysticks();
}

/**
 * Handle SDL2 joystick removal event
 */
void SDL2Input_HandleJoystickRemoved(int device_index)
{
    LOG_INFO("SDL2 joystick %d removed", device_index);
    
    /* Reinitialize joysticks to exclude the removed device */
    SDL2Input_ShutdownJoysticks();
    SDL2Input_InitJoysticks();
} 