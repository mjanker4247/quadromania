/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: joystick_handler.c - Platform-agnostic joystick input handling implementation
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
#include "input/joystick_handler.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Joystick handler state */
static struct
{
    bool initialized;
    JoystickState state;
    JoystickAxisConfig axis_config;
    JoystickButtonConfig button_config;
    JoystickPlatformType platform_type;
    bool button_states[32]; /* Track button states */
    int16_t axis_values[8]; /* Track axis values */
    bool connected;
    int connected_count;
} joystick_handler_state = {0};

/*************
 * CONSTANTS *
 *************/

/* Default axis configurations */
#define DEFAULT_X_AXIS        0
#define DEFAULT_Y_AXIS        1
#define DEFAULT_DEADZONE      16000
#define DEFAULT_THRESHOLD     16000

/* Default button configurations for different platforms */
#define DEFAULT_UP_BUTTON         0
#define DEFAULT_DOWN_BUTTON       1
#define DEFAULT_LEFT_BUTTON       2
#define DEFAULT_RIGHT_BUTTON      3
#define DEFAULT_FIRE_BUTTON       4
#define DEFAULT_ESCAPE_BUTTON     5
#define DEFAULT_VOLUME_UP_BUTTON  6
#define DEFAULT_VOLUME_DOWN_BUTTON 7

/* GP2X-specific button mappings */
#define GP2X_BUTTON_UP             0
#define GP2X_BUTTON_DOWN           4
#define GP2X_BUTTON_LEFT           2
#define GP2X_BUTTON_RIGHT          6
#define GP2X_BUTTON_A              12
#define GP2X_BUTTON_B              13
#define GP2X_BUTTON_X              14
#define GP2X_BUTTON_Y              15
#define GP2X_BUTTON_MENU           8
#define GP2X_BUTTON_VOLUP          16
#define GP2X_BUTTON_VOLDOWN        17

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize the joystick handler
 */
bool JoystickHandler_Init(JoystickPlatformType platform_type,
                         const JoystickAxisConfig* axis_config,
                         const JoystickButtonConfig* button_config)
{
    if (joystick_handler_state.initialized)
    {
        DEBUG_PRINT("Joystick handler already initialized");
        return true;
    }

    /* Set platform type */
    joystick_handler_state.platform_type = platform_type;

    /* Set axis configuration */
    if (axis_config)
    {
        memcpy(&joystick_handler_state.axis_config, axis_config, sizeof(JoystickAxisConfig));
    }
    else
    {
        /* Use default axis configuration for platform */
        joystick_handler_state.axis_config = JoystickHandler_GetDefaultAxisConfig(platform_type);
    }

    /* Set button configuration */
    if (button_config)
    {
        memcpy(&joystick_handler_state.button_config, button_config, sizeof(JoystickButtonConfig));
    }
    else
    {
        /* Use default button configuration for platform */
        joystick_handler_state.button_config = JoystickHandler_GetDefaultButtonConfig(platform_type);
    }

    /* Initialize state */
    memset(&joystick_handler_state.state, 0, sizeof(JoystickState));
    memset(joystick_handler_state.button_states, 0, sizeof(joystick_handler_state.button_states));
    memset(joystick_handler_state.axis_values, 0, sizeof(joystick_handler_state.axis_values));

    joystick_handler_state.initialized = true;
    joystick_handler_state.connected = false;
    joystick_handler_state.connected_count = 0;

    LOG_INFO("Joystick handler initialized successfully for platform %d", platform_type);
    
    return true;
}

/**
 * Shutdown the joystick handler
 */
void JoystickHandler_Shutdown(void)
{
    if (!joystick_handler_state.initialized)
    {
        return;
    }

    memset(&joystick_handler_state, 0, sizeof(joystick_handler_state));
    LOG_INFO("Joystick handler shutdown complete");
}

/**
 * Process joystick events
 */
bool JoystickHandler_ProcessEvent(const InputEvent* event, DpadState* dpad_state)
{
    if (!joystick_handler_state.initialized || !event || !dpad_state)
    {
        return false;
    }

    bool handled = false;

    switch (event->type)
    {
    case INPUT_EVENT_JOYSTICK_AXIS:
        {
            uint8_t axis = event->data.joystick_axis.axis;
            int16_t value = event->data.joystick_axis.value;
            
            if (axis < 8)
            {
                joystick_handler_state.axis_values[axis] = value;
                
                /* Process X axis */
                if (axis == joystick_handler_state.axis_config.x_axis)
                {
                    bool left = (value < -joystick_handler_state.axis_config.threshold);
                    bool right = (value > joystick_handler_state.axis_config.threshold);
                    
                    joystick_handler_state.state.left = left;
                    joystick_handler_state.state.right = right;
                    
                    dpad_state->left = left;
                    dpad_state->right = right;
                    
                    handled = true;
                }
                /* Process Y axis */
                else if (axis == joystick_handler_state.axis_config.y_axis)
                {
                    bool up = (value < -joystick_handler_state.axis_config.threshold);
                    bool down = (value > joystick_handler_state.axis_config.threshold);
                    
                    joystick_handler_state.state.up = up;
                    joystick_handler_state.state.down = down;
                    
                    dpad_state->up = up;
                    dpad_state->down = down;
                    
                    handled = true;
                }
            }
        }
        break;

    case INPUT_EVENT_JOYSTICK_BUTTON_DOWN:
        {
            uint8_t button = event->data.joystick_button.button;
            
            if (button < 32)
            {
                joystick_handler_state.button_states[button] = true;
                
                /* Process button based on platform configuration */
                switch (joystick_handler_state.platform_type)
                {
                case JOYSTICK_PLATFORM_GP2X:
                    if (button == joystick_handler_state.button_config.up_button ||
                        button == GP2X_BUTTON_UP)
                    {
                        joystick_handler_state.state.up = true;
                        dpad_state->up = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.down_button ||
                             button == GP2X_BUTTON_DOWN)
                    {
                        joystick_handler_state.state.down = true;
                        dpad_state->down = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.left_button ||
                             button == GP2X_BUTTON_LEFT)
                    {
                        joystick_handler_state.state.left = true;
                        dpad_state->left = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.right_button ||
                             button == GP2X_BUTTON_RIGHT)
                    {
                        joystick_handler_state.state.right = true;
                        dpad_state->right = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.fire_button ||
                             button == GP2X_BUTTON_A || button == GP2X_BUTTON_B ||
                             button == GP2X_BUTTON_X || button == GP2X_BUTTON_Y)
                    {
                        joystick_handler_state.state.button = true;
                        dpad_state->button = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.escape_button ||
                             button == GP2X_BUTTON_MENU)
                    {
                        joystick_handler_state.state.escape = true;
                        handled = true;
                    }
                    break;

                case JOYSTICK_PLATFORM_DEFAULT:
                default:
                    if (button == joystick_handler_state.button_config.up_button)
                    {
                        joystick_handler_state.state.up = true;
                        dpad_state->up = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.down_button)
                    {
                        joystick_handler_state.state.down = true;
                        dpad_state->down = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.left_button)
                    {
                        joystick_handler_state.state.left = true;
                        dpad_state->left = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.right_button)
                    {
                        joystick_handler_state.state.right = true;
                        dpad_state->right = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.fire_button)
                    {
                        joystick_handler_state.state.button = true;
                        dpad_state->button = true;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.escape_button)
                    {
                        joystick_handler_state.state.escape = true;
                        handled = true;
                    }
                    break;
                }
            }
        }
        break;

    case INPUT_EVENT_JOYSTICK_BUTTON_UP:
        {
            uint8_t button = event->data.joystick_button.button;
            
            if (button < 32)
            {
                joystick_handler_state.button_states[button] = false;
                
                /* Process button release based on platform configuration */
                switch (joystick_handler_state.platform_type)
                {
                case JOYSTICK_PLATFORM_GP2X:
                    if (button == joystick_handler_state.button_config.up_button ||
                        button == GP2X_BUTTON_UP)
                    {
                        joystick_handler_state.state.up = false;
                        dpad_state->up = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.down_button ||
                             button == GP2X_BUTTON_DOWN)
                    {
                        joystick_handler_state.state.down = false;
                        dpad_state->down = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.left_button ||
                             button == GP2X_BUTTON_LEFT)
                    {
                        joystick_handler_state.state.left = false;
                        dpad_state->left = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.right_button ||
                             button == GP2X_BUTTON_RIGHT)
                    {
                        joystick_handler_state.state.right = false;
                        dpad_state->right = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.fire_button ||
                             button == GP2X_BUTTON_A || button == GP2X_BUTTON_B ||
                             button == GP2X_BUTTON_X || button == GP2X_BUTTON_Y)
                    {
                        joystick_handler_state.state.button = false;
                        dpad_state->button = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.escape_button ||
                             button == GP2X_BUTTON_MENU)
                    {
                        joystick_handler_state.state.escape = false;
                        handled = true;
                    }
                    break;

                case JOYSTICK_PLATFORM_DEFAULT:
                default:
                    if (button == joystick_handler_state.button_config.up_button)
                    {
                        joystick_handler_state.state.up = false;
                        dpad_state->up = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.down_button)
                    {
                        joystick_handler_state.state.down = false;
                        dpad_state->down = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.left_button)
                    {
                        joystick_handler_state.state.left = false;
                        dpad_state->left = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.right_button)
                    {
                        joystick_handler_state.state.right = false;
                        dpad_state->right = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.fire_button)
                    {
                        joystick_handler_state.state.button = false;
                        dpad_state->button = false;
                        handled = true;
                    }
                    else if (button == joystick_handler_state.button_config.escape_button)
                    {
                        joystick_handler_state.state.escape = false;
                        handled = true;
                    }
                    break;
                }
            }
        }
        break;

    default:
        break;
    }

    return handled;
}

/**
 * Get current joystick state
 */
const JoystickState* JoystickHandler_GetState(void)
{
    if (!joystick_handler_state.initialized)
    {
        return NULL;
    }
    return &joystick_handler_state.state;
}

/**
 * Reset joystick state
 */
void JoystickHandler_ResetState(void)
{
    if (!joystick_handler_state.initialized)
    {
        return;
    }

    memset(&joystick_handler_state.state, 0, sizeof(JoystickState));
    memset(joystick_handler_state.button_states, 0, sizeof(joystick_handler_state.button_states));
    memset(joystick_handler_state.axis_values, 0, sizeof(joystick_handler_state.axis_values));
}

/**
 * Set joystick axis configuration
 */
void JoystickHandler_SetAxisConfig(const JoystickAxisConfig* axis_config)
{
    if (!joystick_handler_state.initialized || !axis_config)
    {
        return;
    }

    memcpy(&joystick_handler_state.axis_config, axis_config, sizeof(JoystickAxisConfig));
    LOG_INFO("Joystick axis configuration updated");
}

/**
 * Get current joystick axis configuration
 */
const JoystickAxisConfig* JoystickHandler_GetAxisConfig(void)
{
    if (!joystick_handler_state.initialized)
    {
        return NULL;
    }
    return &joystick_handler_state.axis_config;
}

/**
 * Set joystick button configuration
 */
void JoystickHandler_SetButtonConfig(const JoystickButtonConfig* button_config)
{
    if (!joystick_handler_state.initialized || !button_config)
    {
        return;
    }

    memcpy(&joystick_handler_state.button_config, button_config, sizeof(JoystickButtonConfig));
    LOG_INFO("Joystick button configuration updated");
}

/**
 * Get current joystick button configuration
 */
const JoystickButtonConfig* JoystickHandler_GetButtonConfig(void)
{
    if (!joystick_handler_state.initialized)
    {
        return NULL;
    }
    return &joystick_handler_state.button_config;
}

/**
 * Get default axis configuration for the given platform
 */
JoystickAxisConfig JoystickHandler_GetDefaultAxisConfig(JoystickPlatformType platform_type)
{
    JoystickAxisConfig config = {
        .x_axis = DEFAULT_X_AXIS,
        .y_axis = DEFAULT_Y_AXIS,
        .deadzone = DEFAULT_DEADZONE,
        .threshold = DEFAULT_THRESHOLD
    };

    /* Platform-specific overrides can be added here */
    return config;
}

/**
 * Get default button configuration for the given platform
 */
JoystickButtonConfig JoystickHandler_GetDefaultButtonConfig(JoystickPlatformType platform_type)
{
    JoystickButtonConfig config = {
        .up_button = DEFAULT_UP_BUTTON,
        .down_button = DEFAULT_DOWN_BUTTON,
        .left_button = DEFAULT_LEFT_BUTTON,
        .right_button = DEFAULT_RIGHT_BUTTON,
        .fire_button = DEFAULT_FIRE_BUTTON,
        .escape_button = DEFAULT_ESCAPE_BUTTON,
        .volume_up_button = DEFAULT_VOLUME_UP_BUTTON,
        .volume_down_button = DEFAULT_VOLUME_DOWN_BUTTON
    };

    /* Platform-specific overrides */
    switch (platform_type)
    {
    case JOYSTICK_PLATFORM_GP2X:
        config.up_button = GP2X_BUTTON_UP;
        config.down_button = GP2X_BUTTON_DOWN;
        config.left_button = GP2X_BUTTON_LEFT;
        config.right_button = GP2X_BUTTON_RIGHT;
        config.fire_button = GP2X_BUTTON_A;
        config.escape_button = GP2X_BUTTON_MENU;
        config.volume_up_button = GP2X_BUTTON_VOLUP;
        config.volume_down_button = GP2X_BUTTON_VOLDOWN;
        break;

    default:
        /* Use default values */
        break;
    }

    return config;
}

/**
 * Check if a specific joystick button is pressed
 */
bool JoystickHandler_IsButtonPressed(uint8_t button)
{
    if (!joystick_handler_state.initialized || button >= 32)
    {
        return false;
    }

    return joystick_handler_state.button_states[button];
}

/**
 * Get joystick axis value
 */
int16_t JoystickHandler_GetAxisValue(uint8_t axis)
{
    if (!joystick_handler_state.initialized || axis >= 8)
    {
        return 0;
    }

    return joystick_handler_state.axis_values[axis];
}

/**
 * Set joystick platform type
 */
void JoystickHandler_SetPlatformType(JoystickPlatformType platform_type)
{
    if (!joystick_handler_state.initialized)
    {
        return;
    }

    joystick_handler_state.platform_type = platform_type;
    
    /* Update configurations for new platform */
    joystick_handler_state.axis_config = JoystickHandler_GetDefaultAxisConfig(platform_type);
    joystick_handler_state.button_config = JoystickHandler_GetDefaultButtonConfig(platform_type);
    
    LOG_INFO("Joystick platform type changed to %d", platform_type);
}

/**
 * Get current joystick platform type
 */
JoystickPlatformType JoystickHandler_GetPlatformType(void)
{
    return joystick_handler_state.platform_type;
}

/**
 * Detect and set the appropriate platform type automatically
 */
JoystickPlatformType JoystickHandler_DetectPlatform(void)
{
    /* Simple platform detection - can be enhanced */
    JoystickPlatformType detected_platform = JOYSTICK_PLATFORM_DEFAULT;

#ifdef __DINGUX
    detected_platform = JOYSTICK_PLATFORM_GP2X;
#elif defined(__ANDROID__)
    detected_platform = JOYSTICK_PLATFORM_ANDROID;
#elif defined(__APPLE__)
    detected_platform = JOYSTICK_PLATFORM_MACOS;
#elif defined(_WIN32)
    detected_platform = JOYSTICK_PLATFORM_WINDOWS;
#elif defined(__linux__)
    detected_platform = JOYSTICK_PLATFORM_LINUX;
#endif

    if (joystick_handler_state.initialized)
    {
        JoystickHandler_SetPlatformType(detected_platform);
    }

    return detected_platform;
}

/**
 * Check if joystick is connected
 */
bool JoystickHandler_IsConnected(void)
{
    return joystick_handler_state.connected;
}

/**
 * Get number of connected joysticks
 */
int JoystickHandler_GetConnectedCount(void)
{
    return joystick_handler_state.connected_count;
} 