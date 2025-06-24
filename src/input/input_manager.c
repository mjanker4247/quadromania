/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: input_manager.c - Unified input management implementation
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
#include "input/input_manager.h"
#include "input/keyboard_handler.h"
#include "input/mouse_handler.h"
#include "input/joystick_handler.h"
#include "input/platform_input.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Input manager state */
static struct
{
    bool initialized;
    InputConfig config;
    MouseState mouse_state;
    DpadState dpad_state;
    bool quit_requested;
    bool esc_pressed;
    uint8_t debounce_timer_mouse;
    uint8_t debounce_timer_dpad;
    uint8_t debounce_timer_keyboard;
    bool device_enabled[INPUT_DEVICE_COUNT];
    PlatformInputEvent event_buffer[32];
    int event_buffer_size;
    int event_buffer_head;
    int event_buffer_tail;
} input_manager_state = {0};

/*************
 * CONSTANTS *
 *************/
const uint8_t INPUT_DEBOUNCE_TIMESLICES = 20;

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize the input manager with the given configuration
 */
bool InputManager_Init(const InputConfig* config)
{
    if (input_manager_state.initialized)
    {
        DEBUG_PRINT("Input manager already initialized");
        return true;
    }

    if (!config)
    {
        LOG_ERROR("Invalid input configuration");
        return false;
    }

    /* Initialize platform input system */
    PlatformInputConfig platform_config = PlatformInput_DetectPlatform();
    if (!PlatformInput_Init(&platform_config))
    {
        LOG_ERROR("Failed to initialize platform input system");
        return false;
    }

    /* Initialize input handlers */
    KeyMapping key_mapping = KeyboardHandler_GetDefaultMapping();
    if (!KeyboardHandler_Init(&key_mapping))
    {
        LOG_ERROR("Failed to initialize keyboard handler");
        PlatformInput_Shutdown();
        return false;
    }

    MouseButtonConfig mouse_button_config = MouseHandler_GetDefaultButtonConfig();
    MouseSensitivityConfig mouse_sensitivity_config = MouseHandler_GetDefaultSensitivityConfig();
    if (!MouseHandler_Init(&mouse_button_config, &mouse_sensitivity_config))
    {
        LOG_ERROR("Failed to initialize mouse handler");
        KeyboardHandler_Shutdown();
        PlatformInput_Shutdown();
        return false;
    }

    JoystickAxisConfig joystick_axis_config = JoystickHandler_GetDefaultAxisConfig(JOYSTICK_PLATFORM_DEFAULT);
    JoystickButtonConfig joystick_button_config = JoystickHandler_GetDefaultButtonConfig(JOYSTICK_PLATFORM_DEFAULT);
    if (!JoystickHandler_Init(JOYSTICK_PLATFORM_DEFAULT, &joystick_axis_config, &joystick_button_config))
    {
        DEBUG_PRINT("Failed to initialize joystick handler (continuing without joystick support)");
    }

    /* Initialize input manager state */
    memcpy(&input_manager_state.config, config, sizeof(InputConfig));
    
    /* Initialize device states */
    input_manager_state.device_enabled[INPUT_DEVICE_KEYBOARD] = config->enable_keyboard;
    input_manager_state.device_enabled[INPUT_DEVICE_MOUSE] = config->enable_mouse;
    input_manager_state.device_enabled[INPUT_DEVICE_JOYSTICK] = config->enable_joystick;
    input_manager_state.device_enabled[INPUT_DEVICE_TOUCH] = config->enable_touch;

    /* Initialize debounce timers */
    input_manager_state.debounce_timer_mouse = INPUT_DEBOUNCE_TIMESLICES;
    input_manager_state.debounce_timer_dpad = INPUT_DEBOUNCE_TIMESLICES;
    input_manager_state.debounce_timer_keyboard = INPUT_DEBOUNCE_TIMESLICES;

    /* Initialize event buffer */
    input_manager_state.event_buffer_size = 32;
    input_manager_state.event_buffer_head = 0;
    input_manager_state.event_buffer_tail = 0;

    input_manager_state.initialized = true;
    LOG_INFO("Input manager initialized successfully");
    
    return true;
}

/**
 * Shutdown the input manager and cleanup resources
 */
void InputManager_Shutdown(void)
{
    if (!input_manager_state.initialized)
    {
        return;
    }

    JoystickHandler_Shutdown();
    MouseHandler_Shutdown();
    KeyboardHandler_Shutdown();
    PlatformInput_Shutdown();

    memset(&input_manager_state, 0, sizeof(input_manager_state));
    LOG_INFO("Input manager shutdown complete");
}

/**
 * Process all pending input events
 */
void InputManager_ProcessEvents(void)
{
    if (!input_manager_state.initialized)
    {
        return;
    }

    /* Process platform events */
    int events_processed = PlatformInput_ProcessEvents(
        input_manager_state.event_buffer,
        input_manager_state.event_buffer_size
    );

    /* Process each event */
    for (int i = 0; i < events_processed; i++)
    {
        InputEvent* event = &input_manager_state.event_buffer[i].unified_event;
        
        /* Handle quit events */
        if (event->type == INPUT_EVENT_QUIT)
        {
            input_manager_state.quit_requested = true;
            continue;
        }

        /* Process events based on device type */
        switch (event->type)
        {
        case INPUT_EVENT_KEY_DOWN:
        case INPUT_EVENT_KEY_UP:
            if (input_manager_state.device_enabled[INPUT_DEVICE_KEYBOARD])
            {
                KeyboardHandler_ProcessEvent(event, &input_manager_state.dpad_state);
            }
            break;

        case INPUT_EVENT_MOUSE_DOWN:
        case INPUT_EVENT_MOUSE_UP:
        case INPUT_EVENT_MOUSE_MOVE:
            if (input_manager_state.device_enabled[INPUT_DEVICE_MOUSE])
            {
                MouseHandler_ProcessEvent(event, &input_manager_state.mouse_state);
            }
            break;

        case INPUT_EVENT_JOYSTICK_AXIS:
        case INPUT_EVENT_JOYSTICK_BUTTON_DOWN:
        case INPUT_EVENT_JOYSTICK_BUTTON_UP:
            if (input_manager_state.device_enabled[INPUT_DEVICE_JOYSTICK])
            {
                JoystickHandler_ProcessEvent(event, &input_manager_state.dpad_state);
            }
            break;

        default:
            break;
        }
    }

    /* Update debounce timers */
    if (input_manager_state.debounce_timer_mouse > 0)
        input_manager_state.debounce_timer_mouse--;

    if (input_manager_state.debounce_timer_dpad > 0)
        input_manager_state.debounce_timer_dpad--;

    if (input_manager_state.debounce_timer_keyboard > 0)
        input_manager_state.debounce_timer_keyboard--;

    /* Update ESC state from keyboard handler */
    const KeyboardState* keyboard_state = KeyboardHandler_GetState();
    if (keyboard_state)
    {
        input_manager_state.esc_pressed = keyboard_state->escape;
    }
}

/**
 * Get the current mouse state
 */
const MouseState* InputManager_GetMouseState(void)
{
    if (!input_manager_state.initialized)
    {
        return NULL;
    }
    return &input_manager_state.mouse_state;
}

/**
 * Get the current unified directional pad state
 */
const DpadState* InputManager_GetDpadState(void)
{
    if (!input_manager_state.initialized)
    {
        return NULL;
    }
    return &input_manager_state.dpad_state;
}

/**
 * Check if quit was requested
 */
bool InputManager_IsQuitRequested(void)
{
    return input_manager_state.quit_requested;
}

/**
 * Check if ESC key was pressed
 */
bool InputManager_IsESCPressed(void)
{
    return input_manager_state.esc_pressed;
}

/**
 * Debounce mouse input
 */
void InputManager_DebounceMouse(void)
{
    if (!input_manager_state.initialized)
    {
        return;
    }

    input_manager_state.mouse_state.clicked = false;
    input_manager_state.mouse_state.button = 0;
    input_manager_state.debounce_timer_mouse = INPUT_DEBOUNCE_TIMESLICES;
    MouseHandler_ResetState();
}

/**
 * Debounce directional pad input
 */
void InputManager_DebounceDpad(void)
{
    if (!input_manager_state.initialized)
    {
        return;
    }

    input_manager_state.debounce_timer_dpad = INPUT_DEBOUNCE_TIMESLICES;
    input_manager_state.dpad_state.up = false;
    input_manager_state.dpad_state.down = false;
    input_manager_state.dpad_state.left = false;
    input_manager_state.dpad_state.right = false;
    input_manager_state.dpad_state.button = false;
}

/**
 * Debounce keyboard input
 */
void InputManager_DebounceKeyboard(void)
{
    if (!input_manager_state.initialized)
    {
        return;
    }

    input_manager_state.debounce_timer_keyboard = INPUT_DEBOUNCE_TIMESLICES;
    input_manager_state.esc_pressed = false;
    KeyboardHandler_ResetState();
}

/**
 * Check if any directional pad input is pressed
 */
bool InputManager_IsDpadPressed(void)
{
    if (!input_manager_state.initialized)
    {
        return false;
    }

    return (input_manager_state.dpad_state.up || 
            input_manager_state.dpad_state.down || 
            input_manager_state.dpad_state.left || 
            input_manager_state.dpad_state.right || 
            input_manager_state.dpad_state.button);
}

/**
 * Get the next input event from the queue
 */
bool InputManager_GetNextEvent(InputEvent* event)
{
    if (!input_manager_state.initialized || !event)
    {
        return false;
    }

    if (input_manager_state.event_buffer_head == input_manager_state.event_buffer_tail)
    {
        return false; /* Queue is empty */
    }

    *event = input_manager_state.event_buffer[input_manager_state.event_buffer_tail].unified_event;
    input_manager_state.event_buffer_tail = (input_manager_state.event_buffer_tail + 1) % input_manager_state.event_buffer_size;
    
    return true;
}

/**
 * Enable or disable a specific input device
 */
void InputManager_SetDeviceEnabled(InputDeviceType device_type, bool enabled)
{
    if (!input_manager_state.initialized || device_type >= INPUT_DEVICE_COUNT)
    {
        return;
    }

    input_manager_state.device_enabled[device_type] = enabled;
    LOG_INFO("Input device %d %s", device_type, enabled ? "enabled" : "disabled");
}

/**
 * Get whether a specific input device is enabled
 */
bool InputManager_IsDeviceEnabled(InputDeviceType device_type)
{
    if (!input_manager_state.initialized || device_type >= INPUT_DEVICE_COUNT)
    {
        return false;
    }

    return input_manager_state.device_enabled[device_type];
} 