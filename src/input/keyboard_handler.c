/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: keyboard_handler.c - Platform-agnostic keyboard input handling implementation
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
#include "input/keyboard_handler.h"
#include "utils/logger.h"

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Keyboard handler state */
static struct
{
    bool initialized;
    KeyboardState state;
    KeyMapping mapping;
    bool key_states[256]; /* Simple key state tracking */
} keyboard_handler_state = {0};

/*************
 * CONSTANTS *
 *************/

/* Default key mappings */
#define DEFAULT_UP_KEY        0x26    /* Arrow Up */
#define DEFAULT_DOWN_KEY      0x28    /* Arrow Down */
#define DEFAULT_LEFT_KEY      0x25    /* Arrow Left */
#define DEFAULT_RIGHT_KEY     0x27    /* Arrow Right */
#define DEFAULT_BUTTON_KEY    0x11    /* Ctrl */
#define DEFAULT_ESCAPE_KEY    0x1B    /* Escape */
#define DEFAULT_QUIT_KEY      0x1B    /* Escape */
#define DEFAULT_VOLUME_UP_KEY 0x6B    /* Keypad + */
#define DEFAULT_VOLUME_DOWN_KEY 0x6D  /* Keypad - */

/*************
 * FUNCTIONS *
 *************/

/**
 * Initialize the keyboard handler
 */
bool KeyboardHandler_Init(const KeyMapping* mapping)
{
    if (keyboard_handler_state.initialized)
    {
        DEBUG_PRINT("Keyboard handler already initialized");
        return true;
    }

    /* Set key mapping */
    if (mapping)
    {
        memcpy(&keyboard_handler_state.mapping, mapping, sizeof(KeyMapping));
    }
    else
    {
        /* Use default mapping */
        keyboard_handler_state.mapping = KeyboardHandler_GetDefaultMapping();
    }

    /* Initialize state */
    memset(&keyboard_handler_state.state, 0, sizeof(KeyboardState));
    memset(keyboard_handler_state.key_states, 0, sizeof(keyboard_handler_state.key_states));

    keyboard_handler_state.initialized = true;
    LOG_INFO("Keyboard handler initialized successfully");
    
    return true;
}

/**
 * Shutdown the keyboard handler
 */
void KeyboardHandler_Shutdown(void)
{
    if (!keyboard_handler_state.initialized)
    {
        return;
    }

    memset(&keyboard_handler_state, 0, sizeof(keyboard_handler_state));
    LOG_INFO("Keyboard handler shutdown complete");
}

/**
 * Process keyboard events
 */
bool KeyboardHandler_ProcessEvent(const InputEvent* event, DpadState* dpad_state)
{
    if (!keyboard_handler_state.initialized || !event || !dpad_state)
    {
        return false;
    }

    bool handled = false;
    uint32_t key_code = event->data.key.key_code;

    switch (event->type)
    {
    case INPUT_EVENT_KEY_DOWN:
        keyboard_handler_state.key_states[key_code & 0xFF] = true;
        
        /* Update directional pad state */
        if (key_code == keyboard_handler_state.mapping.up_key)
        {
            keyboard_handler_state.state.up = true;
            dpad_state->up = true;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.down_key)
        {
            keyboard_handler_state.state.down = true;
            dpad_state->down = true;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.left_key)
        {
            keyboard_handler_state.state.left = true;
            dpad_state->left = true;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.right_key)
        {
            keyboard_handler_state.state.right = true;
            dpad_state->right = true;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.button_key)
        {
            keyboard_handler_state.state.button = true;
            dpad_state->button = true;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.escape_key)
        {
            keyboard_handler_state.state.escape = true;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.quit_key)
        {
            keyboard_handler_state.state.quit = true;
            handled = true;
        }
        break;

    case INPUT_EVENT_KEY_UP:
        keyboard_handler_state.key_states[key_code & 0xFF] = false;
        
        /* Update directional pad state */
        if (key_code == keyboard_handler_state.mapping.up_key)
        {
            keyboard_handler_state.state.up = false;
            dpad_state->up = false;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.down_key)
        {
            keyboard_handler_state.state.down = false;
            dpad_state->down = false;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.left_key)
        {
            keyboard_handler_state.state.left = false;
            dpad_state->left = false;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.right_key)
        {
            keyboard_handler_state.state.right = false;
            dpad_state->right = false;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.button_key)
        {
            keyboard_handler_state.state.button = false;
            dpad_state->button = false;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.escape_key)
        {
            keyboard_handler_state.state.escape = false;
            handled = true;
        }
        else if (key_code == keyboard_handler_state.mapping.quit_key)
        {
            keyboard_handler_state.state.quit = false;
            handled = true;
        }
        break;

    default:
        break;
    }

    return handled;
}

/**
 * Get current keyboard state
 */
const KeyboardState* KeyboardHandler_GetState(void)
{
    if (!keyboard_handler_state.initialized)
    {
        return NULL;
    }
    return &keyboard_handler_state.state;
}

/**
 * Reset keyboard state
 */
void KeyboardHandler_ResetState(void)
{
    if (!keyboard_handler_state.initialized)
    {
        return;
    }

    memset(&keyboard_handler_state.state, 0, sizeof(KeyboardState));
    memset(keyboard_handler_state.key_states, 0, sizeof(keyboard_handler_state.key_states));
}

/**
 * Set key mapping
 */
void KeyboardHandler_SetKeyMapping(const KeyMapping* mapping)
{
    if (!keyboard_handler_state.initialized || !mapping)
    {
        return;
    }

    memcpy(&keyboard_handler_state.mapping, mapping, sizeof(KeyMapping));
    LOG_INFO("Keyboard mapping updated");
}

/**
 * Get current key mapping
 */
const KeyMapping* KeyboardHandler_GetKeyMapping(void)
{
    if (!keyboard_handler_state.initialized)
    {
        return NULL;
    }
    return &keyboard_handler_state.mapping;
}

/**
 * Check if a specific key is pressed
 */
bool KeyboardHandler_IsKeyPressed(uint32_t key_code)
{
    if (!keyboard_handler_state.initialized)
    {
        return false;
    }

    return keyboard_handler_state.key_states[key_code & 0xFF];
}

/**
 * Get default key mapping for the current platform
 */
KeyMapping KeyboardHandler_GetDefaultMapping(void)
{
    KeyMapping mapping = {
        .up_key = DEFAULT_UP_KEY,
        .down_key = DEFAULT_DOWN_KEY,
        .left_key = DEFAULT_LEFT_KEY,
        .right_key = DEFAULT_RIGHT_KEY,
        .button_key = DEFAULT_BUTTON_KEY,
        .escape_key = DEFAULT_ESCAPE_KEY,
        .quit_key = DEFAULT_QUIT_KEY,
        .volume_up_key = DEFAULT_VOLUME_UP_KEY,
        .volume_down_key = DEFAULT_VOLUME_DOWN_KEY
    };

    /* Platform-specific overrides can be added here */
#ifdef __DINGUX
    /* Dingux-specific key mappings */
    mapping.volume_up_key = 0x08;    /* Backspace */
    mapping.volume_down_key = 0x09;  /* Tab */
#endif

    return mapping;
} 