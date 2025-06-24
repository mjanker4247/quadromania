/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: mouse_handler.c - Platform-agnostic mouse input handling implementation
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
#include "input/mouse_handler.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Mouse handler state */
static struct
{
    bool initialized;
    MouseState state;
    MouseButtonConfig button_config;
    MouseSensitivityConfig sensitivity_config;
    bool button_states[8]; /* Track button states */
} mouse_handler_state = {0};

/*************
 * CONSTANTS *
 *************/

/* Default mouse button mappings */
#define DEFAULT_LEFT_BUTTON    1
#define DEFAULT_RIGHT_BUTTON   3
#define DEFAULT_MIDDLE_BUTTON  2

/* Default sensitivity settings */
#define DEFAULT_X_SENSITIVITY  1.0f
#define DEFAULT_Y_SENSITIVITY  1.0f
#define DEFAULT_INVERT_Y       false

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize the mouse handler
 */
bool MouseHandler_Init(const MouseButtonConfig* button_config, 
                      const MouseSensitivityConfig* sensitivity_config)
{
    if (mouse_handler_state.initialized)
    {
        DEBUG_PRINT("Mouse handler already initialized");
        return true;
    }

    /* Set button configuration */
    if (button_config)
    {
        memcpy(&mouse_handler_state.button_config, button_config, sizeof(MouseButtonConfig));
    }
    else
    {
        /* Use default button configuration */
        mouse_handler_state.button_config = MouseHandler_GetDefaultButtonConfig();
    }

    /* Set sensitivity configuration */
    if (sensitivity_config)
    {
        memcpy(&mouse_handler_state.sensitivity_config, sensitivity_config, sizeof(MouseSensitivityConfig));
    }
    else
    {
        /* Use default sensitivity configuration */
        mouse_handler_state.sensitivity_config = MouseHandler_GetDefaultSensitivityConfig();
    }

    /* Initialize state */
    memset(&mouse_handler_state.state, 0, sizeof(MouseState));
    memset(mouse_handler_state.button_states, 0, sizeof(mouse_handler_state.button_states));

    mouse_handler_state.initialized = true;
    LOG_INFO("Mouse handler initialized successfully");
    
    return true;
}

/**
 * Shutdown the mouse handler
 */
void MouseHandler_Shutdown(void)
{
    if (!mouse_handler_state.initialized)
    {
        return;
    }

    memset(&mouse_handler_state, 0, sizeof(mouse_handler_state));
    LOG_INFO("Mouse handler shutdown complete");
}

/**
 * Process mouse events
 */
bool MouseHandler_ProcessEvent(const InputEvent* event, MouseState* mouse_state)
{
    if (!mouse_handler_state.initialized || !event || !mouse_state)
    {
        return false;
    }

    bool handled = false;
    uint8_t button = event->data.mouse.button;
    uint16_t x = event->data.mouse.x;
    uint16_t y = event->data.mouse.y;
    float adjusted_x, adjusted_y;

    switch (event->type)
    {
    case INPUT_EVENT_MOUSE_MOVE:
        /* Apply sensitivity and inversion */
        adjusted_x = x * mouse_handler_state.sensitivity_config.x_sensitivity;
        adjusted_y = y * mouse_handler_state.sensitivity_config.y_sensitivity;
        
        if (mouse_handler_state.sensitivity_config.invert_y)
        {
            adjusted_y = -adjusted_y;
        }

        mouse_handler_state.state.x = (uint16_t)adjusted_x;
        mouse_handler_state.state.y = (uint16_t)adjusted_y;
        
        /* Update the provided mouse state */
        mouse_state->x = mouse_handler_state.state.x;
        mouse_state->y = mouse_handler_state.state.y;
        
        handled = true;
        break;

    case INPUT_EVENT_MOUSE_DOWN:
        if (button < 8)
        {
            mouse_handler_state.button_states[button] = true;
            mouse_handler_state.state.button = button;
            mouse_handler_state.state.clicked = true;
            mouse_handler_state.state.x = x;
            mouse_handler_state.state.y = y;
            
            /* Update the provided mouse state */
            mouse_state->button = button;
            mouse_state->clicked = true;
            mouse_state->x = x;
            mouse_state->y = y;
            
            handled = true;
        }
        break;

    case INPUT_EVENT_MOUSE_UP:
        if (button < 8)
        {
            mouse_handler_state.button_states[button] = false;
            mouse_handler_state.state.clicked = false;
            
            /* Update the provided mouse state */
            mouse_state->clicked = false;
            
            handled = true;
        }
        break;

    default:
        break;
    }

    return handled;
}

/**
 * Get current mouse state
 */
const MouseState* MouseHandler_GetState(void)
{
    if (!mouse_handler_state.initialized)
    {
        return NULL;
    }
    return &mouse_handler_state.state;
}

/**
 * Reset mouse state
 */
void MouseHandler_ResetState(void)
{
    if (!mouse_handler_state.initialized)
    {
        return;
    }

    memset(&mouse_handler_state.state, 0, sizeof(MouseState));
    memset(mouse_handler_state.button_states, 0, sizeof(mouse_handler_state.button_states));
}

/**
 * Set mouse button configuration
 */
void MouseHandler_SetButtonConfig(const MouseButtonConfig* button_config)
{
    if (!mouse_handler_state.initialized || !button_config)
    {
        return;
    }

    memcpy(&mouse_handler_state.button_config, button_config, sizeof(MouseButtonConfig));
    LOG_INFO("Mouse button configuration updated");
}

/**
 * Get current mouse button configuration
 */
const MouseButtonConfig* MouseHandler_GetButtonConfig(void)
{
    if (!mouse_handler_state.initialized)
    {
        return NULL;
    }
    return &mouse_handler_state.button_config;
}

/**
 * Set mouse sensitivity configuration
 */
void MouseHandler_SetSensitivityConfig(const MouseSensitivityConfig* sensitivity_config)
{
    if (!mouse_handler_state.initialized || !sensitivity_config)
    {
        return;
    }

    memcpy(&mouse_handler_state.sensitivity_config, sensitivity_config, sizeof(MouseSensitivityConfig));
    LOG_INFO("Mouse sensitivity configuration updated");
}

/**
 * Get current mouse sensitivity configuration
 */
const MouseSensitivityConfig* MouseHandler_GetSensitivityConfig(void)
{
    if (!mouse_handler_state.initialized)
    {
        return NULL;
    }
    return &mouse_handler_state.sensitivity_config;
}

/**
 * Get default mouse button configuration for the current platform
 */
MouseButtonConfig MouseHandler_GetDefaultButtonConfig(void)
{
    MouseButtonConfig config = {
        .left_button = DEFAULT_LEFT_BUTTON,
        .right_button = DEFAULT_RIGHT_BUTTON,
        .middle_button = DEFAULT_MIDDLE_BUTTON
    };

    /* Platform-specific overrides can be added here */
    return config;
}

/**
 * Get default mouse sensitivity configuration for the current platform
 */
MouseSensitivityConfig MouseHandler_GetDefaultSensitivityConfig(void)
{
    MouseSensitivityConfig config = {
        .x_sensitivity = DEFAULT_X_SENSITIVITY,
        .y_sensitivity = DEFAULT_Y_SENSITIVITY,
        .invert_y = DEFAULT_INVERT_Y
    };

    /* Platform-specific overrides can be added here */
    return config;
}

/**
 * Check if a specific mouse button is pressed
 */
bool MouseHandler_IsButtonPressed(uint8_t button)
{
    if (!mouse_handler_state.initialized || button >= 8)
    {
        return false;
    }

    return mouse_handler_state.button_states[button];
}

/**
 * Get mouse position
 */
void MouseHandler_GetPosition(uint16_t* x, uint16_t* y)
{
    if (!mouse_handler_state.initialized)
    {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }

    if (x) *x = mouse_handler_state.state.x;
    if (y) *y = mouse_handler_state.state.y;
}

/**
 * Set mouse position (for relative positioning)
 */
void MouseHandler_SetPosition(uint16_t x, uint16_t y)
{
    if (!mouse_handler_state.initialized)
    {
        return;
    }

    mouse_handler_state.state.x = x;
    mouse_handler_state.state.y = y;
} 